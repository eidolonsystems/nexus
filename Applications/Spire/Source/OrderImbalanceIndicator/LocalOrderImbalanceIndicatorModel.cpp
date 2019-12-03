#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/Utility.hpp"

using namespace boost;
using namespace boost::icl;
using namespace Nexus;
using namespace Spire;

void LocalOrderImbalanceIndicatorModel::publish(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
  m_imbalance_published_signal(imbalance);
}

void LocalOrderImbalanceIndicatorModel::insert(
    const OrderImbalance& imbalance) {
  insert_sorted(imbalance);
}

QtPromise<std::vector<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::load(const TimeInterval& interval) {
  auto first = [&] {
    if(is_left_closed(interval.bounds())) {
      return std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
        interval.lower(), [] (const auto& imbalance, const auto& timestamp) {
          return imbalance.m_timestamp < timestamp;
        });
    }
    return std::upper_bound(m_imbalances.begin(), m_imbalances.end(),
      interval.lower(), [] (const auto& timestamp, const auto& imbalance) {
        return imbalance.m_timestamp > timestamp;
      });
  }();
  auto last = [&] {
    if(is_right_closed(interval.bounds())) {
     return std::upper_bound(first, m_imbalances.end(),
        interval.upper(), [] (const auto& timestamp, const auto& imbalance) {
          return imbalance.m_timestamp > timestamp;
        });
    }
    return std::lower_bound(first, m_imbalances.end(),
      interval.upper(), [] (const auto& imbalance, const auto& timestamp) {
        return imbalance.m_timestamp < timestamp;
      });
  }();
  return QtPromise(
    [imbalances = std::vector<OrderImbalance>(first, last)] () mutable {
      return std::move(imbalances);
  });
}

SubscriptionResult<optional<Nexus::OrderImbalance>>
    LocalOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  auto last_imbalance = [&] () -> optional<Nexus::OrderImbalance> {
    if(m_imbalances.empty()) {
      return none;
    }
    return m_imbalances.back();
  }();
  return {m_imbalance_published_signal.connect(slot),
    QtPromise([imbalance = std::move(last_imbalance)] () mutable {
      return std::move(imbalance); })};
}

std::shared_ptr<OrderImbalanceChartModel>
    LocalOrderImbalanceIndicatorModel::get_chart_model(
    const Security& security) {
  throw std::runtime_error("method not implemented");
}

void LocalOrderImbalanceIndicatorModel::insert_sorted(
    const OrderImbalance& imbalance) {
  if(m_imbalances.empty() ||
      m_imbalances.back().m_timestamp < imbalance.m_timestamp) {
    m_imbalances.push_back(imbalance);
    return;
  }
  auto index = std::lower_bound(m_imbalances.begin(), m_imbalances.end(),
    imbalance.m_timestamp,
    [] (const auto& stored_imbalance, const auto& timestamp) {
      return stored_imbalance.m_timestamp < timestamp;
    });
  m_imbalances.insert(index, imbalance);
}
