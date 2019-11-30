#ifndef SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#define SPIRE_LOCAL_ORDER_IMBALANCE_INDICATOR_MODEL_HPP
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"

namespace Spire {

  //! Implements the OrderImbalanceIndicatorModel in memory.
  class LocalOrderImbalanceIndicatorModel :
      public OrderImbalanceIndicatorModel {
    public:

      //! Publishes a new order imbalance.
      /*
        \param imbalance The order imbalance to publish.
      */
      void publish(const Nexus::OrderImbalance& imbalance);

      //! Inserts an order imbalance into the model, does not publish it.
      /*
        \param imbalance The order imbalance to insert into the model.
      */
      void insert(const Nexus::OrderImbalance& imbalance);

      QtPromise<std::vector<Nexus::OrderImbalance>> load(
        const TimeInterval& interval) override;

      SubscriptionResult<boost::optional<Nexus::OrderImbalance>> subscribe(
        const OrderImbalanceSignal::slot_type& slot) override;
  };
}

#endif
