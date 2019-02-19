#include "spire/charting/cached_chart_model.hpp"
#include <stack>
#include <boost/range/adaptor/reversed.hpp>

using namespace boost::signals2;
using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel& model)
    : m_chart_model(&model),
      m_data_promise_counter(0) {}

ChartValue::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

ChartValue::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(ChartValue first,
    ChartValue last) {

  // check if range already exists, if so, return it instead of loading


  if(m_ranges.empty()) {
    return load_data({ChartRange({first, last})});
  }
  auto value_loaded = [=] (const auto& value) {
    return std::find_if(m_loaded_data.begin(), m_loaded_data.end(),
      [=] (const auto& v) { return v.GetEnd() == value; }) ==
      m_loaded_data.end();
  };
  auto first_loaded = value_loaded(first);
  auto last_loaded = value_loaded(last);
  auto gaps = std::vector<ChartRange>();
  if(first_loaded) {
    gaps.push_back({first, ChartValue()});
  }
  // I don't think this will work, assumes that if both values are found,
  // then the loaded values are contiguous
  if(last_loaded) {
    if(m_loaded_data.begin()->GetStart() > last ||
        (first_loaded && last_loaded)) {
      gaps.front().m_end = last;
      return load_data(gaps);
    }
  }
  if(gaps.begin() == gaps.end()) {
    gaps.push_back({m_ranges.front().m_end, ChartValue()});
  }
  for(auto i = 0; i < m_ranges.size(); ++i) {
    if(last < m_ranges[i].m_start) {
      gaps.back().m_end = last;
      break;
    }
    gaps.back().m_end = m_ranges[i].m_start;
    if(m_ranges[i].m_end > last) {
      break;
    }
    gaps.push_back({m_ranges[i].m_end, ChartValue()});
  }
  if(gaps.back().m_end == ChartValue()) {
    gaps.back().m_end = last;
  }
  return load_data(gaps);
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const std::vector<ChartRange>& data) {
  if(m_ranges.empty()) {
    m_ranges.push_back(data.front());
  } else {
    auto ranges = m_ranges;
    ranges.insert(ranges.end(), data.begin(), data.end());
    std::sort(ranges.begin(), ranges.end(),
      [] (const auto& lhs, const auto& rhs) {
        return lhs.m_start < rhs.m_start;
      });
    m_ranges.clear();
    for(auto& range : ranges) {
      if(m_ranges.empty()) {
        m_ranges.push_back(range);
      } else {
        auto top = m_ranges.back();
        if(range.m_start <= top.m_end) {
          auto upper_bound = std::max(top.m_end, range.m_end);
          m_ranges.back() = {top.m_start, upper_bound};
        } else {
          m_ranges.push_back(range);
        }
      }
    }
  }
  m_load_data_promises.clear();
  return make_qt_promise([=] {
    for(auto& range : data) {
      m_load_data_promises.push_back(m_chart_model->load(
        range.m_start, range.m_end));
      ++m_data_promise_counter;
      m_load_data_promises.back().then([=] (auto result) {
        // sort this data
        m_loaded_data.insert(m_loaded_data.end(), result.Get().begin(),
          result.Get().end());
        --m_data_promise_counter;
        if(m_data_promise_counter == 0) {
          m_load_data_promises.clear();
        }
      });
    }
    while(m_data_promise_counter != 0) {
      
    }
    return m_loaded_data;
  });
}
