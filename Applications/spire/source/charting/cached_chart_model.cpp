#include "spire/charting/cached_chart_model.hpp"

using namespace boost::signals2;
using namespace Spire;

CachedChartModel::CachedChartModel(ChartModel& model)
    : m_chart_model(&model) {}

ChartValue::Type CachedChartModel::get_x_axis_type() const {
  return m_chart_model->get_x_axis_type();
}

ChartValue::Type CachedChartModel::get_y_axis_type() const {
  return m_chart_model->get_y_axis_type();
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load(ChartValue first,
    ChartValue last) {
  for(auto& range : m_ranges) {
    if(range.m_start <= first && range.m_end >= last) {
      auto first_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), first,
        [=] (const auto& value, const auto& search_value) {
          return value.GetStart() < search_value;
      });
      auto last_iterator = std::lower_bound(m_loaded_data.begin(),
        m_loaded_data.end(), last,
        [=] (const auto& value, const auto& search_value) {
          return value.GetEnd() <= search_value;
      });
      return QtPromise([=] {
        return std::vector<Candlestick>(first_iterator, last_iterator);
      });
    }
  }
  return load_data({first, last});
}

connection CachedChartModel::connect_candlestick_slot(
    const CandlestickSignal::slot_type& slot) const {
  return m_chart_model->connect_candlestick_slot(slot);
}

QtPromise<std::vector<Candlestick>> CachedChartModel::load_data(
    const ChartRange& data) {
  return m_chart_model->load(data.m_start, data.m_end).then(
    [=] (auto result) {
      if(m_loaded_data.empty()) {
        m_loaded_data.insert(m_loaded_data.end(), result.Get().begin(),
          result.Get().end());
      } else {
        auto first = std::find_if(m_loaded_data.begin(), m_loaded_data.end(),
          [=] (const auto& value) { return value.GetStart() < data.m_start; });
        auto last = std::find_if(m_loaded_data.begin(), m_loaded_data.end(),
          [=] (const auto& value) { return value.GetStart() < data.m_end; });
        
      }
      if(m_ranges.empty()) {
        m_ranges.push_back(data);
      } else {
        auto ranges = m_ranges;
        ranges.push_back(data);
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
      return result.Get();
  });
}
