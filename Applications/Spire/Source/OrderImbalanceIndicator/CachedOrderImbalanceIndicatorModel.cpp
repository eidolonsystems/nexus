#include "Spire/OrderImbalanceIndicator/CachedOrderImbalanceIndicatorModel.hpp"

using namespace boost;
using namespace boost::icl;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

CachedOrderImbalanceIndicatorModel::CachedOrderImbalanceIndicatorModel(
    std::shared_ptr<OrderImbalanceIndicatorModel> source)
    : m_source_model(std::move(source)) {
  auto [connection, promise] = m_source_model->subscribe(
    [=] (const auto& imbalance) {
      on_imbalance_published(imbalance);
    });
  m_subscription_connection = std::move(connection);
  m_subscription_promise = std::move(promise);
}

QtPromise<std::vector<OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load(const TimeInterval& interval) {
  if(boost::icl::contains(m_intervals, interval)) {
    return m_cache.load(interval);
  }
  return load_from_model(interval).then([=] (const auto& loaded_imbalances) {
    //on_imbalances_loaded(interval, loaded_imbalances.Get());
    return m_cache.load(interval);
  });
}

QtPromise<std::vector<OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::load(const Nexus::Security& security,
    const TimeInterval& interval) {
  //if(is_security_data_loaded(security, interval)) {
  //  return m_cache.load(security, interval);
  //}
  return load_from_model(security, interval).then(
    [=] (const auto& loaded_imbalances) {
      //on_imbalances_loaded(security, interval, loaded_imbalances.Get());
      return m_cache.load(security, interval);
  });
}

SubscriptionResult<optional<Nexus::OrderImbalance>>
    CachedOrderImbalanceIndicatorModel::subscribe(
    const OrderImbalanceSignal::slot_type& slot) {
  return m_source_model->subscribe(slot);
}

QtPromise<void> CachedOrderImbalanceIndicatorModel::load_from_model(
    const TimeInterval& interval) {
  //auto promises = std::vector<QtPromise<std::vector<OrderImbalance>>>();
  //auto unloaded_intervals = interval_set<ptime>(interval) - m_intervals;
  //for(auto& interval : unloaded_intervals) {
  //  promises.push_back(m_source_model->load(TimeInterval::closed(
  //    interval.lower(), interval.upper())));
  //}
  //return all(std::move(promises));
  auto promises = std::vector<QtPromise<void>>();
  for(auto& security : m_security_intervals) {
    auto unloaded_intervals = interval_set<ptime>(interval) -
      security.m_intervals;
    auto promise = m_source_model->load(security.m_security,
      TimeInterval::closed(interval.lower(), interval.upper())).then(
      [=] (const auto& imbalances) {
        //on_imbalances_loaded(security.m_security, interval, imbalances.Get());
        //update_security_intervals(security.m_security, interval);
        return QtPromise([] {});
      });
    promises.push_back(std::move(promise));
  }
  return all(std::move(promises));
}

void CachedOrderImbalanceIndicatorModel::on_imbalance_published(
    const OrderImbalance& imbalance) {
  // TODO: add it to master list as well?
  //m_intervals.add(TimeInterval::closed(imbalance.m_timestamp,
  //  imbalance.m_timestamp));
  //update_security_intervals(imbalance.m_security, TimeInterval::closed(
  //  imbalance.m_timestamp, imbalance.m_timestamp));
  m_cache.insert(imbalance);
}
//
//void CachedOrderImbalanceIndicatorModel::on_imbalances_loaded(
//    const TimeInterval& interval,
//    const std::vector<std::vector<OrderImbalance>>& imbalances) {
//  for(auto& list : imbalances) {
//    if(!list.empty()) {
//      auto unique_intervals = interval_set<ptime>({list.front().m_timestamp,
//        list.back().m_timestamp}) - m_all_intervals;
//      for(auto& unique_interval : unique_intervals) {
//        auto first = [&] {
//          auto lower = unique_interval.lower();
//          if(boost::icl::contains(m_all_intervals, TimeInterval::closed(lower,
//              lower))) {
//            return std::upper_bound(list.begin(), list.end(), lower,
//              [] (const auto& timestamp, const auto& imbalance) {
//                return imbalance.m_timestamp > timestamp;
//              });
//          }
//          return list.begin();
//        }();
//        auto last = [&] {
//          auto upper =  unique_interval.upper();
//          if(boost::icl::contains(m_all_intervals, TimeInterval::closed(upper,
//              upper))) {
//            return std::lower_bound(first, list.end(), upper,
//              [] (const auto& imbalance, const auto& timestamp) {
//                return timestamp < imbalance.m_timestamp;
//              });
//          }
//          return list.end();
//        }();
//        if(first == last && !boost::icl::contains(m_all_intervals,
//            TimeInterval::closed(first->m_timestamp, first->m_timestamp))) {
//          m_cache.insert(std::move(*first));
//        } else {
//          for(auto i = first; i < last; ++i) {
//            m_cache.insert(std::move(*i));
//          }
//        }
//      }
//    }
//  }
//  m_all_intervals.add(interval);
//}
