#ifndef NEXUS_RISK_TRANSITION_TRACKER_HPP
#define NEXUS_RISK_TRANSITION_TRACKER_HPP
#include <iostream>
#include <unordered_set>
#include <Beam/Utilities/ReportException.hpp>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/RiskService/RiskService.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus::RiskService {

  /**
   * Keeps track of an account's RiskState and performs the actions required to
   * transition from one RiskState to another.
   * @param <O> The type of OrderExecutionClient used to cancel Orders and
   *        flatten Positions.
   */
  template<typename O>
  class RiskTransitionTracker {
    public:

      /**
       * The type of OrderExecutionClient used to cancel Orders and flatten
       * Positions.
       */
      using OrderExecutionClient = Beam::GetTryDereferenceType<O>;

      /**
       * Constructs a RiskTransitionTracker.
       * @param account The account being tracked.
       * @param orderExecutionClient The OrderExecutionClient used to cancel
       *        Orders and flatten Positions.
       * @param destinations The database of destinations used to flatten
       *        Orders.
       * @param markets The database of markets used to flatten Orders.
       */
      template<typename OF>
      RiskTransitionTracker(Beam::ServiceLocator::DirectoryEntry account,
        OF&& orderExecutionClient, const DestinationDatabase& destinations,
        const MarketDatabase& markets);

      /**
       * Adds an Order.
       * @param order The Order to add.
       */
      void Add(const OrderExecutionService::Order& order);

      /**
       * Updates the RiskState.
       * @param state The new RiskState.
       */
      void Update(const RiskState& state);

      /**
       * Updates an Order with an ExecutionReport.
       * @param report The ExecutionReport containing the details of the update.
       */
      void Update(const OrderExecutionService::ExecutionReport& report);

    private:
      Beam::ServiceLocator::DirectoryEntry m_account;
      Beam::GetOptionalLocalPtr<O> m_orderExecutionClient;
      DestinationDatabase m_destinations;
      MarketDatabase m_markets;
      RiskState m_riskState;
      Accounting::PositionOrderBook m_book;
      std::unordered_set<OrderExecutionService::OrderId> m_liveOrders;
      int m_state;

      bool C0();
      bool C1();
      bool C2();
      bool C3();
      void S0();
      void S1();
      void S2();
      void S3();
      void S4();
      void S5();
      void S6();
  };

  template<typename O>
  template<typename OF>
  RiskTransitionTracker<O>::RiskTransitionTracker(
    Beam::ServiceLocator::DirectoryEntry account, OF&& orderExecutionClient,
    const DestinationDatabase& destinations, const MarketDatabase& markets)
    : m_account(std::move(account)),
      m_orderExecutionClient(std::forward<OF>(orderExecutionClient)),
      m_destinations(destinations),
      m_markets(markets),
      m_state(0) {}

  template<typename O>
  void RiskTransitionTracker<O>::Add(
      const OrderExecutionService::Order& order) {
    m_book.Add(order);
  }

  template<typename O>
  void RiskTransitionTracker<O>::Update(const RiskState& state) {
    m_riskState = state;
    if(m_state == 0) {
      return S0();
    } else if(m_state == 2) {
      return S2();
    } else if(m_state == 4) {
      return S4();
    } else if(m_state == 6) {
      return S6();
    }
  }

  template<typename O>
  void RiskTransitionTracker<O>::Update(
      const OrderExecutionService::ExecutionReport& report) {
    m_book.Update(report);
    if(IsTerminal(report.m_status)) {
      m_liveOrders.erase(report.m_id);
    }
    if(m_state == 4) {
      return S4();
    }
  }

  template<typename O>
  bool RiskTransitionTracker<O>::C0() {
    return m_riskState.m_type == RiskState::Type::CLOSE_ORDERS;
  }

  template<typename O>
  bool RiskTransitionTracker<O>::C1() {
    return m_riskState.m_type == RiskState::Type::ACTIVE;
  }

  template<typename O>
  bool RiskTransitionTracker<O>::C2() {
    return m_riskState.m_type == RiskState::Type::DISABLED;
  }

  template<typename O>
  bool RiskTransitionTracker<O>::C3() {
    return m_liveOrders.empty();
  }

  template<typename O>
  void RiskTransitionTracker<O>::S0() {
    m_state = 0;
    if(C0()) {
      return S1();
    }
  }

  template<typename O>
  void RiskTransitionTracker<O>::S1() {
    m_state = 1;
    for(auto& openingOrder : m_book.GetOpeningOrders()) {
      m_orderExecutionClient->Cancel(*openingOrder);
    }
    return S2();
  }

  template<typename O>
  void RiskTransitionTracker<O>::S2() {
    m_state = 2;
    if(C1()) {
      return S0();
    } else if(C2()) {
      return S3();
    }
  }

  template<typename O>
  void RiskTransitionTracker<O>::S3() {
    m_state = 3;
    auto& liveOrders = m_book.GetLiveOrders();
    m_liveOrders.clear();
    for(auto& liveOrder : liveOrders) {
      m_liveOrders.insert(liveOrder->GetInfo().m_orderId);
      m_orderExecutionClient->Cancel(*liveOrder);
    }
    return S4();
  }

  template<typename O>
  void RiskTransitionTracker<O>::S4() {
    m_state = 4;
    if(C1()) {
      return S0();
    } else if(C3()) {
      return S5();
    }
  }

  template<typename O>
  void RiskTransitionTracker<O>::S5() {
    m_state = 5;
    for(auto& position : m_book.GetPositions()) {
      auto currency = m_markets.FromCode(
        position.m_security.GetMarket()).m_currency;
      auto destination = m_destinations.GetPreferredDestination(
        position.m_security.GetMarket()).m_id;
      auto orderFields = OrderExecutionService::OrderFields::BuildMarketOrder(
        m_account, position.m_security, currency,
        GetOpposite(GetSide(position.m_quantity)), destination,
        Abs(position.m_quantity));
      try {
        m_orderExecutionClient->Submit(orderFields);
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
    return S6();
  }

  template<typename O>
  void RiskTransitionTracker<O>::S6() {
    m_state = 6;
    if(C1()) {
      return S0();
    }
  }
}

#endif