#ifndef NEXUS_SECURITYORDERSIMULATOR_HPP
#define NEXUS_SECURITYORDERSIMULATOR_HPP
#include <unordered_set>
#include <boost/noncopyable.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class SecurityOrderSimulator
      \brief Handles simulating Orders submitted for a specific Security.
      \tparam TimeClientType The type of TimeClient used for Order timestamps.
   */
  template<typename TimeClientType>
  class SecurityOrderSimulator : private boost::noncopyable {
    public:

      //! The type of TimeClient to use.
      using TimeClient = TimeClientType;

      //! Constructs a SecurityOrderSimulator.
      /*!
        \param marketDataClient The MarketDataClient to query.
        \param security The Security to simulate Order executions for.
        \param timeClient The TimeClient used for Order timestamps.
      */
      template<typename MarketDataClient>
      SecurityOrderSimulator(MarketDataClient& marketDataClient,
        const Security& security, Beam::RefType<TimeClient> timeClient);

      //! Submits an Order for simulated Order entry.
      /*!
        \param order The Order to submit.
      */
      void Submit(const std::shared_ptr<PrimitiveOrder>& order);

      //! Cancels a previously submitted order.
      /*!
        \param order The Order to cancel.
      */
      void Cancel(const std::shared_ptr<PrimitiveOrder>& order);

      //! Updates an Order.
      /*!
        \param order The Order to update.
        \param executionReport The ExecutionReport containing the update.
      */
      void Update(const std::shared_ptr<PrimitiveOrder>& order,
        const ExecutionReport& executionReport);

      //! Recovers a previously submitted Order.
      /*!
        \param order The Order to recover.
      */
      void Recover(const std::shared_ptr<PrimitiveOrder>& order);

    private:
      TimeClient* m_timeClient;
      std::unordered_set<std::shared_ptr<PrimitiveOrder>> m_orders;
      std::shared_ptr<Beam::StateQueue<BboQuote>> m_bboQuoteQueue;
      Beam::RoutineTaskQueue m_tasks;

      OrderStatus FillOrder(PrimitiveOrder& order, Money price);
      OrderStatus UpdateOrder(PrimitiveOrder& order);
      void OnBbo(const BboQuote& bboQuote);
  };

  template<typename TimeClientType>
  template<typename MarketDataClient>
  SecurityOrderSimulator<TimeClientType>::SecurityOrderSimulator(
      MarketDataClient& marketDataClient, const Security& security,
      Beam::RefType<TimeClient> timeClient)
      : m_timeClient(timeClient.Get()),
        m_bboQuoteQueue(std::make_shared<Beam::StateQueue<BboQuote>>()) {
    MarketDataService::SecurityMarketDataQuery bboQuery =
      MarketDataService::BuildRealTimeWithSnapshotQuery(security);
    marketDataClient.QueryBboQuotes(bboQuery, m_bboQuoteQueue);
    marketDataClient.QueryBboQuotes(bboQuery, m_tasks.GetSlot<BboQuote>(
      std::bind(&SecurityOrderSimulator::OnBbo, this, std::placeholders::_1)));
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Submit(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push(
      [=] {
        m_orders.insert(order);
        order->With(
          [&] (OrderStatus status,
              const std::vector<ExecutionReport>& reports) {
            const ExecutionReport& lastReport = reports.back();
            ExecutionReport updatedReport = ExecutionReport::BuildUpdatedReport(
              lastReport, OrderStatus::NEW, order->GetInfo().m_timestamp);
            order->Update(updatedReport);
          });
        UpdateOrder(*order);
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Cancel(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push(
      [=] {
        order->With(
          [&] (OrderStatus status,
              const std::vector<ExecutionReport>& reports) {
            if(IsTerminal(status) || reports.empty()) {
              return;
            }
            ExecutionReport pendingCancelReport =
              ExecutionReport::BuildUpdatedReport(reports.back(),
              OrderStatus::PENDING_CANCEL, m_timeClient->GetTime());
            order->Update(pendingCancelReport);
            ExecutionReport cancelReport = ExecutionReport::BuildUpdatedReport(
              reports.back(), OrderStatus::CANCELED, m_timeClient->GetTime());
            order->Update(cancelReport);
          });
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Update(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& executionReport) {
    m_tasks.Push(
      [=] {
        order->With(
          [&] (OrderStatus status,
              const std::vector<ExecutionReport>& executionReports) {
            if(IsTerminal(status)) {
              return;
            }
            auto updatedReport = executionReport;
            updatedReport.m_sequence = executionReports.back().m_sequence + 1;
            if(updatedReport.m_timestamp.is_special()) {
              updatedReport.m_timestamp =
                boost::posix_time::microsec_clock::universal_time();
            }
            order->Update(updatedReport);
          });
      });
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::Recover(
      const std::shared_ptr<PrimitiveOrder>& order) {
    m_tasks.Push(
      [=] {
        m_orders.insert(order);
        UpdateOrder(*order);
      });
  }

  template<typename TimeClientType>
  OrderStatus SecurityOrderSimulator<TimeClientType>::FillOrder(
      PrimitiveOrder& order, Money price) {
    const auto BOARD_LOT = 100;
    for(int i = 0; i != order.GetInfo().m_fields.m_quantity / BOARD_LOT; ++i) {
      order.With(
        [&] (OrderStatus status, const std::vector<ExecutionReport>& reports) {
          const ExecutionReport& lastReport = reports.back();
          OrderStatus fillStatus;
          if(i + 1 < order.GetInfo().m_fields.m_quantity / BOARD_LOT) {
            fillStatus = OrderStatus::PARTIALLY_FILLED;
          } else {
            fillStatus = OrderStatus::FILLED;
          }
          ExecutionReport updatedReport = ExecutionReport::BuildUpdatedReport(
            lastReport, fillStatus, m_timeClient->GetTime());
          updatedReport.m_lastQuantity = BOARD_LOT;
          updatedReport.m_lastPrice = price;
          order.Update(updatedReport);
        });
    }
    return OrderStatus::FILLED;
  }

  template<typename TimeClientType>
  OrderStatus SecurityOrderSimulator<TimeClientType>::UpdateOrder(
      PrimitiveOrder& order) {
    OrderStatus finalStatus;
    BboQuote bboQuote = m_bboQuoteQueue->Top();
    order.With(
      [&] (OrderStatus status, const std::vector<ExecutionReport>& reports) {
        Side side = order.GetInfo().m_fields.m_side;
        finalStatus = status;
        if(status == OrderStatus::PENDING_NEW || IsTerminal(status)) {
          return;
        }
        if(order.GetInfo().m_fields.m_timeInForce.GetType() ==
            TimeInForce::Type::MOC) {
          return;
        }
        if(order.GetInfo().m_fields.m_type == OrderType::MARKET) {
          Money price;
          if(side == Side::BID) {
            price = bboQuote.m_ask.m_price;
          } else {
            price = bboQuote.m_bid.m_price;
          }
          finalStatus = this->FillOrder(order, price);
        } else {
          if(side == Side::BID && bboQuote.m_ask.m_price <=
              order.GetInfo().m_fields.m_price) {
            finalStatus = this->FillOrder(order, bboQuote.m_ask.m_price);
          } else if(side == Side::ASK && bboQuote.m_bid.m_price >=
              order.GetInfo().m_fields.m_price) {
            finalStatus = this->FillOrder(order, bboQuote.m_bid.m_price);
          }
        }
      });
    return finalStatus;
  }

  template<typename TimeClientType>
  void SecurityOrderSimulator<TimeClientType>::OnBbo(const BboQuote& bboQuote) {
    auto i = m_orders.begin();
    while(i != m_orders.end()) {
      OrderStatus status = UpdateOrder(**i);
      if(IsTerminal(status)) {
        i = m_orders.erase(i);
      } else {
        ++i;
      }
    }
  }
}
}

#endif
