#ifndef NEXUS_INTERNALMATCHINGORDEREXECUTIONDRIVER_HPP
#define NEXUS_INTERNALMATCHINGORDEREXECUTIONDRIVER_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Threading/TimedConditionVariable.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/InternalMatcher/InternalMatcher.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {
namespace InternalMatcher {
namespace Details {
  inline Money GetOfferPrice(
      const OrderExecutionService::OrderFields& orderFields) {
    if(orderFields.m_type == OrderType::LIMIT) {
      return orderFields.m_price;
    }
    if(orderFields.m_side == Side::ASK) {
      return Money::ZERO;
    }
    return std::numeric_limits<Money>::max();
  }
}

  /*! \class InternalMatchingOrderExecutionDriver
      \brief A OrderExecutionDriver layer that maintains a complete Order book
             and attempts to match Orders internally before passing them on.
      \tparam MatchReportBuilderType The type used to build ExecutionReports
              for internal matches.
      \tparam MarketDataClientType The type of MarketDataClient used to price
              internally matched orders.
      \tparam TimeClientType The type of TimeClient used for Order timestamps.
      \tparam UidClientType The type of UidClient used for Order ids.
      \tparam OrderExecutionDriverType The type of OrderExecutionDriver to pass
              non-internalized Order submissions to.
   */
  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  class InternalMatchingOrderExecutionDriver : private boost::noncopyable {
    public:

      //! The type used to build ExecutionReports for internal matches.
      using MatchReportBuilder =
        Beam::GetTryDereferenceType<MatchReportBuilderType>;

      //! The type of MarketDataClient used to price internally matched orders.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! The type of TimeClient used for Order timestamps.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! The type of UidClient used for Order ids.
      using UidClient = Beam::GetTryDereferenceType<UidClientType>;

      //! The type of OrderExecutionDriver to send the submission to if all
      //! checks pass.
      using OrderExecutionDriver =
        Beam::GetTryDereferenceType<OrderExecutionDriverType>;

      //! Constructs an InternalMatchingOrderExecutionDriver.
      /*!
        \param matchReportBuilder Initializes the MatchReportBuilder.
        \param marketDataClient Initializes the MarketDataClient.
        \param timeClient Initializes the TimeClient.
        \param uidClient Initializes the UidClient.
        \param orderExecutionDriver Initializes the OrderExecutionDriver.
        \param timerThreadPool The thread pool used for timeouts.
      */
      template<typename MatchReportBuilderForward,
        typename MarketDataClientForward, typename TimeClientForward,
        typename UidClientForward, typename OrderExecutionDriverForward>
      InternalMatchingOrderExecutionDriver(
        const Beam::ServiceLocator::DirectoryEntry& rootSessionAccount,
        MatchReportBuilderForward&& matchReportBuilder,
        MarketDataClientForward&& marketDataClient,
        TimeClientForward&& timeClient, UidClientForward&& uidClient,
        OrderExecutionDriverForward&& orderExecutionDriver,
        Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool);

      ~InternalMatchingOrderExecutionDriver();

      const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord);

      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& orderInfo);

      void Cancel(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId);

      void Update(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport);

      void Open();

      void Close();

    private:
      struct OrderEntry {
        OrderExecutionService::OrderInfo m_orderInfo;
        const OrderExecutionService::Order* m_driverOrder;
        std::shared_ptr<OrderExecutionService::PrimitiveOrder> m_order;
        bool m_isPendingNew;
        boost::atomic_bool m_isMatching;
        Quantity m_remainingQuantity;
        Beam::Threading::Sync<bool> m_isLive;
        Beam::Threading::Sync<bool> m_isTerminal;
        Beam::Threading::TimedConditionVariable m_isLiveCondition;
        Beam::Threading::TimedConditionVariable m_isTerminalCondition;

        OrderEntry(const OrderExecutionService::OrderInfo& orderInfo,
          Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool);
      };
      struct SecurityEntry {
        std::vector<std::shared_ptr<OrderEntry>> m_asks;
        std::vector<std::shared_ptr<OrderEntry>> m_bids;
        std::shared_ptr<Beam::StateQueue<BboQuote>> m_bboQuote;

        SecurityEntry();
      };
      Beam::GetOptionalLocalPtr<MatchReportBuilderType> m_matchReportBuilder;
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::GetOptionalLocalPtr<UidClientType> m_uidClient;
      Beam::GetOptionalLocalPtr<OrderExecutionDriverType>
        m_orderExecutionDriver;
      Beam::Threading::TimerThreadPool* m_timerThreadPool;
      OrderExecutionService::OrderExecutionSession m_rootSession;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId,
        OrderExecutionService::OrderId> m_orderIds;
      Beam::SynchronizedUnorderedSet<
        std::shared_ptr<OrderExecutionService::Order>> m_orders;
      std::unordered_map<Security, std::shared_ptr<SecurityEntry>>
        m_securityEntries;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_submissionTasks;
      Beam::RoutineTaskQueue m_executionReportTasks;

      void Shutdown();
      void Submit(const std::shared_ptr<OrderEntry>& orderEntry);
      void SubmitToDriver(
        const Beam::ServiceLocator::DirectoryEntry submissionAccount,
        const OrderExecutionService::OrderFields& fields,
        const std::shared_ptr<OrderEntry>& orderEntry);
      OrderExecutionService::ExecutionReport InternalMatch(
        const std::shared_ptr<OrderEntry>& activeOrderEntry,
        const std::shared_ptr<OrderEntry>& passiveOrderEntry,
        Quantity quantity, Beam::Out<bool> passiveOrderRemaining);
      void WaitForLiveOrder(OrderEntry& orderEntry);
      void WaitForTerminalOrder(OrderEntry& orderEntry);
      void SetOrderToLive(OrderEntry& orderEntry);
      void SetOrderToTerminal(OrderEntry& orderEntry);
      void OnExecutionReport(std::weak_ptr<OrderEntry> weakOrderEntry,
        const OrderExecutionService::ExecutionReport& executionReport);
  };

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::OrderEntry::OrderEntry(
      const OrderExecutionService::OrderInfo& orderInfo,
      Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool)
      : m_orderInfo{orderInfo},
        m_driverOrder{nullptr},
        m_order{std::make_shared<OrderExecutionService::PrimitiveOrder>(
          m_orderInfo)},
        m_isPendingNew{true},
        m_isMatching{false},
        m_remainingQuantity{orderInfo.m_fields.m_quantity},
        m_isLive{false},
        m_isTerminal{false},
        m_isLiveCondition{Beam::Ref(timerThreadPool)},
        m_isTerminalCondition{Beam::Ref(timerThreadPool)} {}

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::SecurityEntry::SecurityEntry()
      : m_bboQuote{std::make_shared<Beam::StateQueue<BboQuote>>()} {}

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  template<typename MatchReportBuilderForward,
    typename MarketDataClientForward, typename TimeClientForward,
    typename UidClientForward, typename OrderExecutionDriverForward>
  InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::InternalMatchingOrderExecutionDriver(
      const Beam::ServiceLocator::DirectoryEntry& rootSessionAccount,
      MatchReportBuilderForward&& matchReportBuilder,
      MarketDataClientForward&& marketDataClient,
      TimeClientForward&& timeClient, UidClientForward&& uidClient,
      OrderExecutionDriverForward&& orderExecutionDriver,
      Beam::Ref<Beam::Threading::TimerThreadPool> timerThreadPool)
      : m_matchReportBuilder{std::forward<MatchReportBuilderForward>(
          matchReportBuilder)},
        m_marketDataClient{std::forward<MarketDataClientForward>(
          marketDataClient)},
        m_timeClient{std::forward<TimeClientForward>(timeClient)},
        m_uidClient{std::forward<UidClientForward>(uidClient)},
        m_orderExecutionDriver{std::forward<OrderExecutionDriverForward>(
          orderExecutionDriver)},
        m_timerThreadPool{timerThreadPool.Get()} {
    m_rootSession.SetAccount(rootSessionAccount);
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::~InternalMatchingOrderExecutionDriver() {
    Close();
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  const OrderExecutionService::Order& InternalMatchingOrderExecutionDriver<
      MatchReportBuilderType, MarketDataClientType, TimeClientType,
      UidClientType, OrderExecutionDriverType>::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    auto& order = m_orderExecutionDriver->Recover(orderRecord);
    m_orderIds.Insert(order.GetInfo().m_orderId, order.GetInfo().m_orderId);
    return order;
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  const OrderExecutionService::Order& InternalMatchingOrderExecutionDriver<
      MatchReportBuilderType, MarketDataClientType, TimeClientType,
      UidClientType, OrderExecutionDriverType>::Submit(
      const OrderExecutionService::OrderInfo& orderInfo) {
    auto& fields = orderInfo.m_fields;
    if((fields.m_timeInForce.GetType() != TimeInForce::Type::DAY &&
        fields.m_timeInForce.GetType() != TimeInForce::Type::GTC &&
        fields.m_timeInForce.GetType() != TimeInForce::Type::IOC &&
        fields.m_timeInForce.GetType() != TimeInForce::Type::GTX &&
        fields.m_timeInForce.GetType() != TimeInForce::Type::GTD) ||
        (fields.m_type != OrderType::LIMIT &&
        fields.m_type != OrderType::MARKET) || fields.m_quantity <= 0) {
      m_orderIds.Insert(orderInfo.m_orderId, orderInfo.m_orderId);
      return m_orderExecutionDriver->Submit(orderInfo);
    }
    auto orderEntry = std::make_shared<OrderEntry>(orderInfo,
      Beam::Ref(*m_timerThreadPool));
    m_submissionTasks.Push(
      [=] {
        Submit(orderEntry);
      });
    m_orders.Insert(orderEntry->m_order);
    return *orderEntry->m_order;
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::Cancel(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId) {
    m_submissionTasks.Push(
      [=] {
        auto driverOrderId = m_orderIds.FindValue(orderId);
        if(driverOrderId.is_initialized()) {
          m_orderExecutionDriver->Cancel(session, *driverOrderId);
        } else {
          m_orderExecutionDriver->Cancel(session, orderId);
        }
      });
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport) {
    m_submissionTasks.Push(
      [=] {
        auto driverOrderId = m_orderIds.FindValue(orderId);
        if(driverOrderId.is_initialized()) {
          auto sanitizedExecutionReport = executionReport;
          sanitizedExecutionReport.m_id = *driverOrderId;
          m_orderExecutionDriver->Update(session, *driverOrderId,
            sanitizedExecutionReport);
        } else {
          m_orderExecutionDriver->Update(session, orderId, executionReport);
        }
      });
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataClient->Open();
      m_timeClient->Open();
      m_uidClient->Open();
      m_orderExecutionDriver->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::Shutdown() {
    m_executionReportTasks.Break();
    m_submissionTasks.Break();
    m_orderExecutionDriver->Close();
    m_openState.SetClosed();
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::Submit(
      const std::shared_ptr<OrderEntry>& orderEntry) {
    const auto& fields = orderEntry->m_order->GetInfo().m_fields;
    auto securityEntry = Beam::GetOrInsert(m_securityEntries,
      fields.m_security,
      [&] {
        auto securityEntry = std::make_shared<SecurityEntry>();
        MarketDataService::QueryRealTimeWithSnapshot(fields.m_security,
          *m_marketDataClient, securityEntry->m_bboQuote);
        return securityEntry;
      });
    auto bboQuote =
      [&] () -> boost::optional<BboQuote> {
        try {
          return securityEntry->m_bboQuote->Top();
        } catch(const Beam::PipeBrokenException&) {
          m_securityEntries.erase(fields.m_security);
          return boost::none;
        }
      }();
    if(!bboQuote.is_initialized()) {
      orderEntry->m_order->With(
        [&] (auto status, auto& executionReports) {
          auto newReport = OrderExecutionService::ExecutionReport::
            BuildUpdatedReport(executionReports.back(),
            OrderStatus::REJECTED, m_timeClient->GetTime());
          orderEntry->m_order->Update(newReport);
        });
      return;
    }
    std::vector<std::shared_ptr<OrderEntry>>* orderEntries;
    std::vector<std::shared_ptr<OrderEntry>>* passiveOrderEntries;
    if(fields.m_side == Side::ASK) {
      orderEntries = &securityEntry->m_asks;
      passiveOrderEntries = &securityEntry->m_bids;
    } else {
      orderEntries = &securityEntry->m_bids;
      passiveOrderEntries = &securityEntry->m_asks;
    }
    Money bboThresholdPrice;
    if(fields.m_side == Side::ASK) {
      bboThresholdPrice = bboQuote->m_bid.m_price;
    } else {
      bboThresholdPrice = bboQuote->m_ask.m_price;
    }
    std::vector<OrderExecutionService::ExecutionReport> internalMatchReports;
    auto matchedQuantityRemaining = fields.m_quantity;
    auto passiveOrderEntryIterator = passiveOrderEntries->begin();
    while(passiveOrderEntryIterator != passiveOrderEntries->end() &&
        matchedQuantityRemaining > 0) {
      const auto& passiveOrderEntry = *passiveOrderEntryIterator;
      const auto& orderEntryFields =
        passiveOrderEntry->m_driverOrder->GetInfo().m_fields;
      auto fieldsPrice = Details::GetOfferPrice(fields);
      auto orderEntryPrice = Details::GetOfferPrice(orderEntryFields);
      if(OfferComparator(fields.m_side, fieldsPrice, orderEntryPrice) <= 0 &&
          OfferComparator(fields.m_side, orderEntryPrice,
          bboThresholdPrice) >= 0) {
        bool passiveOrderRemaining;
        OrderExecutionService::ExecutionReport matchReport;
        try {
          matchReport = InternalMatch(orderEntry, passiveOrderEntry,
            matchedQuantityRemaining, Beam::Store(passiveOrderRemaining));
        } catch(const std::exception&) {
          ++passiveOrderEntryIterator;
          continue;
        }
        if(matchReport.m_lastQuantity != 0) {
          internalMatchReports.push_back(matchReport);
          matchedQuantityRemaining -= matchReport.m_lastQuantity;
        }
        if(passiveOrderRemaining) {
          ++passiveOrderEntryIterator;
        } else {
          passiveOrderEntryIterator = passiveOrderEntries->erase(
            passiveOrderEntryIterator);
        }
      } else {
        ++passiveOrderEntryIterator;
      }
    }
    if(!internalMatchReports.empty()) {
      orderEntry->m_order->With(
        [&] (auto status, auto& executionReports) {
          orderEntry->m_isPendingNew = false;
          auto newReport = OrderExecutionService::ExecutionReport::
            BuildUpdatedReport(executionReports.back(), OrderStatus::NEW,
            m_timeClient->GetTime());
          orderEntry->m_order->Update(newReport);
          auto sequence = newReport.m_sequence;
          for(auto& executionReport : internalMatchReports) {
            ++sequence;
            executionReport.m_timestamp = m_timeClient->GetTime();
            executionReport.m_sequence = sequence;
            orderEntry->m_order->Update(executionReport);
          }
        });
    }
    if(matchedQuantityRemaining != 0) {
      auto insertIterator = std::lower_bound(orderEntries->begin(),
        orderEntries->end(), orderEntry,
        [&] (auto& lhs, auto& rhs) {
          auto lhsPrice = Details::GetOfferPrice(
            lhs->m_order->GetInfo().m_fields);
          auto rhsPrice = Details::GetOfferPrice(
            rhs->m_order->GetInfo().m_fields);
          auto offerComparison = OfferComparator(
            lhs->m_order->GetInfo().m_fields.m_side, lhsPrice, rhsPrice);
          if(offerComparison < 0) {
            return true;
          } else if(offerComparison > 0) {
            return false;
          }
          if(lhs->m_order->GetInfo().m_timestamp <
              rhs->m_order->GetInfo().m_timestamp) {
            return true;
          }
          return lhs->m_order->GetInfo().m_orderId <
            rhs->m_order->GetInfo().m_orderId;
        });
      orderEntries->insert(insertIterator, orderEntry);
      auto matchedFields = fields;
      matchedFields.m_quantity = matchedQuantityRemaining;
      SubmitToDriver(orderEntry->m_orderInfo.m_submissionAccount,
        matchedFields, orderEntry);
    }
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::SubmitToDriver(
      const Beam::ServiceLocator::DirectoryEntry submissionAccount,
      const OrderExecutionService::OrderFields& fields,
      const std::shared_ptr<OrderEntry>& orderEntry) {
    OrderExecutionService::OrderId driverOrderId;
    if(orderEntry->m_driverOrder == nullptr) {
      driverOrderId = orderEntry->m_order->GetInfo().m_orderId;
    } else {
      driverOrderId = m_uidClient->LoadNextUid();
    }
    m_orderIds.Update(orderEntry->m_order->GetInfo().m_orderId, driverOrderId);
    OrderExecutionService::OrderInfo driverOrderInfo;
    driverOrderInfo.m_fields = fields;
    driverOrderInfo.m_orderId = driverOrderId;
    driverOrderInfo.m_submissionAccount = submissionAccount;
    driverOrderInfo.m_timestamp = m_timeClient->GetTime();
    driverOrderInfo.m_shortingFlag = orderEntry->m_orderInfo.m_shortingFlag;
    auto driverOrder = &m_orderExecutionDriver->Submit(driverOrderInfo);
    orderEntry->m_driverOrder = driverOrder;
    orderEntry->m_driverOrder->GetPublisher().Monitor(
      m_executionReportTasks.GetSlot<OrderExecutionService::ExecutionReport>(
      std::bind(&InternalMatchingOrderExecutionDriver::OnExecutionReport, this,
      std::weak_ptr<OrderEntry>{orderEntry}, std::placeholders::_1)));
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  OrderExecutionService::ExecutionReport InternalMatchingOrderExecutionDriver<
      MatchReportBuilderType, MarketDataClientType, TimeClientType,
      UidClientType, OrderExecutionDriverType>::InternalMatch(
      const std::shared_ptr<OrderEntry>& activeOrderEntry,
      const std::shared_ptr<OrderEntry>& passiveOrderEntry,
      Quantity quantity, Beam::Out<bool> passiveOrderRemaining) {
    passiveOrderEntry->m_isMatching = true;
    try {
      WaitForLiveOrder(*passiveOrderEntry);
    } catch(const Beam::Threading::TimeoutException&) {
      passiveOrderEntry->m_isMatching = false;
      BOOST_RETHROW;
    }
    m_orderExecutionDriver->Cancel(m_rootSession,
      passiveOrderEntry->m_driverOrder->GetInfo().m_orderId);
    try {
      WaitForTerminalOrder(*passiveOrderEntry);
    } catch(const Beam::Threading::TimeoutException&) {
      passiveOrderEntry->m_isMatching = false;
      BOOST_RETHROW;
    }
    passiveOrderEntry->m_isMatching = false;
    passiveOrderEntry->m_isLive = false;
    passiveOrderEntry->m_isTerminal = false;
    OrderExecutionService::ExecutionReport passiveMatchReport;
    OrderExecutionService::ExecutionReport activeMatchReport;
    passiveMatchReport.m_id = passiveOrderEntry->m_order->GetInfo().m_orderId;
    activeMatchReport.m_id = activeOrderEntry->m_order->GetInfo().m_orderId;
    auto matchedQuantity = std::min(passiveOrderEntry->m_remainingQuantity,
      quantity);
    passiveOrderEntry->m_remainingQuantity -= matchedQuantity;
    *passiveOrderRemaining = (passiveOrderEntry->m_remainingQuantity != 0);
    if(matchedQuantity == 0) {
      return activeMatchReport;
    }
    passiveMatchReport.m_lastQuantity = matchedQuantity;
    activeMatchReport.m_lastQuantity = matchedQuantity;
    passiveMatchReport.m_lastPrice =
      passiveOrderEntry->m_order->GetInfo().m_fields.m_price;
    activeMatchReport.m_lastPrice =
      passiveOrderEntry->m_order->GetInfo().m_fields.m_price;
    if(matchedQuantity == quantity) {
      activeMatchReport.m_status = OrderStatus::FILLED;
    } else {
      activeMatchReport.m_status = OrderStatus::PARTIALLY_FILLED;
    }
    if(passiveOrderEntry->m_remainingQuantity == 0) {
      passiveMatchReport.m_status = OrderStatus::FILLED;
    } else {
      passiveMatchReport.m_status = OrderStatus::PARTIALLY_FILLED;
    }
    m_matchReportBuilder->Build(passiveOrderEntry->m_order->GetInfo().m_fields,
      activeOrderEntry->m_order->GetInfo().m_fields,
      Beam::Store(passiveMatchReport), Beam::Store(activeMatchReport));
    passiveOrderEntry->m_order->With(
      [&] (auto status, auto& executionReports) {
        passiveMatchReport.m_timestamp = m_timeClient->GetTime();
        passiveMatchReport.m_sequence = executionReports.back().m_sequence + 1;
        passiveOrderEntry->m_order->Update(passiveMatchReport);
      });
    if(passiveOrderEntry->m_remainingQuantity != 0) {
      auto matchedFields = passiveOrderEntry->m_order->GetInfo().m_fields;
      matchedFields.m_quantity = passiveOrderEntry->m_remainingQuantity;
      SubmitToDriver(m_rootSession.GetAccount(), matchedFields,
        passiveOrderEntry);
    }
    return activeMatchReport;
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::WaitForLiveOrder(OrderEntry& orderEntry) {
    Beam::Threading::With(orderEntry.m_isLive,
      [&] (auto& isLive) {
        while(!isLive) {
          orderEntry.m_isLiveCondition.timed_wait(
            boost::posix_time::seconds(1), orderEntry.m_isLive.GetLock());
        }
      });
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::WaitForTerminalOrder(OrderEntry& orderEntry) {
    Beam::Threading::With(orderEntry.m_isTerminal,
      [&] (auto& isTerminal) {
        while(!isTerminal) {
          orderEntry.m_isTerminalCondition.timed_wait(
            boost::posix_time::seconds(1), orderEntry.m_isTerminal.GetLock());
        }
      });
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::SetOrderToLive(OrderEntry& orderEntry) {
    Beam::Threading::With(orderEntry.m_isLive,
      [&] (auto& isLive) {
        if(isLive) {
          return;
        }
        isLive = true;
        orderEntry.m_isLiveCondition.notify_all();
      });
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::SetOrderToTerminal(OrderEntry& orderEntry) {
    Beam::Threading::With(orderEntry.m_isTerminal,
      [&] (auto& isTerminal) {
        if(isTerminal) {
          return;
        }
        isTerminal = true;
        orderEntry.m_isTerminalCondition.notify_all();
      });
  }

  template<typename MatchReportBuilderType, typename MarketDataClientType,
    typename TimeClientType, typename UidClientType,
    typename OrderExecutionDriverType>
  void InternalMatchingOrderExecutionDriver<MatchReportBuilderType,
      MarketDataClientType, TimeClientType, UidClientType,
      OrderExecutionDriverType>::OnExecutionReport(
      std::weak_ptr<OrderEntry> weakOrderEntry,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    auto orderEntry = weakOrderEntry.lock();
    if(orderEntry == nullptr) {
      return;
    }
    SetOrderToLive(*orderEntry);
    if(orderEntry->m_isPendingNew) {
      orderEntry->m_isPendingNew = false;
    } else if(executionReport.m_status == OrderStatus::NEW) {
      return;
    }
    orderEntry->m_remainingQuantity -= executionReport.m_lastQuantity;
    if(orderEntry->m_isMatching) {
      if(IsTerminal(executionReport.m_status) &&
          executionReport.m_lastQuantity == 0) {
        SetOrderToTerminal(*orderEntry);
        return;
      } else if(executionReport.m_status == OrderStatus::PENDING_CANCEL &&
          executionReport.m_lastQuantity == 0) {
        return;
      }
    }
    auto updateReport = executionReport;
    updateReport.m_id = orderEntry->m_order->GetInfo().m_orderId;
    orderEntry->m_order->With(
      [&] (auto status, auto& executionReports) {
        updateReport.m_timestamp = executionReport.m_timestamp;
        updateReport.m_sequence = executionReports.back().m_sequence + 1;
        orderEntry->m_order->Update(updateReport);
      });
    if(IsTerminal(executionReport.m_status)) {
      orderEntry->m_remainingQuantity = 0;
      SetOrderToTerminal(*orderEntry);
    }
  }
}
}

#endif
