#ifndef NEXUS_BOARDLOTCHECK_HPP
#define NEXUS_BOARDLOTCHECK_HPP
#include <memory>
#include <type_traits>
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include "Nexus/Definitions/BboQuote.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheck.hpp"
#include "Nexus/OrderExecutionService/OrderSubmissionCheckException.hpp"
#include "Nexus/TechnicalAnalysis/StandardSecurityQueries.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class BoardLotCheck
      \brief Validates an Order's board lot size.
      \tparam MarketDataClientType The type of MarketDataClient used to
              determine the price of a Security.
      \tparam TimeClientType The type of TimeClient used to get the current
              time.
   */
  template<typename MarketDataClientType, typename TimeClientType>
  class BoardLotCheck : public OrderSubmissionCheck {
    public:

      //! The type of MarketDataClient used to price Orders for buying power
      //! checks.
      using MarketDataClient =
        Beam::GetTryDereferenceType<MarketDataClientType>;

      //! The type of TimeClient used to get the current time.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs a BoardLotCheck.
      /*!
        \param marketDataClient Initializes the MarketDataClient.
        \param timeClient Initializes the TimeClient.
      */
      template<typename MarketDataClientForward, typename TimeClientForward>
      BoardLotCheck(MarketDataClientForward&& marketDataClient,
        TimeClientForward&& timeClient, const MarketDatabase& marketDatabase,
        const boost::local_time::tz_database& timeZoneDatabase);

      virtual void Submit(const OrderInfo& orderInfo);

    private:
      struct ClosingEntry {
        boost::posix_time::ptime m_lastUpdate;
        Money m_closingPrice;

        ClosingEntry();
      };
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      MarketDatabase m_marketDatabase;
      boost::local_time::tz_database m_timeZoneDatabase;
      Beam::SynchronizedUnorderedMap<Security,
        Beam::Threading::Sync<ClosingEntry, Beam::Threading::Mutex>>
        m_closingEntries;
      Beam::SynchronizedUnorderedMap<Security,
        std::shared_ptr<Beam::StateQueue<BboQuote>>> m_bboQuotes;

      Money LoadPrice(const Security& security);
  };

  template<typename MarketDataClientType, typename TimeClientType>
  BoardLotCheck<MarketDataClientType, TimeClientType>::ClosingEntry::
      ClosingEntry()
      : m_lastUpdate{boost::posix_time::neg_infin},
        m_closingPrice{Money::ZERO} {}

  template<typename MarketDataClient, typename TimeClient>
  std::unique_ptr<BoardLotCheck<typename std::decay<MarketDataClient>::type,
      typename std::decay<TimeClient>::type>> MakeBoardLotCheck(
      MarketDataClient&& marketDataClient, TimeClient&& timeClient,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    return std::make_unique<BoardLotCheck<
      typename std::decay<MarketDataClient>::type,
      typename std::decay<TimeClient>::type>>(
      std::forward<MarketDataClient>(marketDataClient),
      std::forward<TimeClient>(timeClient), marketDatabase, timeZoneDatabase);
  }

  template<typename MarketDataClientType, typename TimeClientType>
  template<typename MarketDataClientForward, typename TimeClientForward>
  BoardLotCheck<MarketDataClientType, TimeClientType>::BoardLotCheck(
      MarketDataClientForward&& marketDataClient,
      TimeClientForward&& timeClient, const MarketDatabase& marketDatabase,
        const boost::local_time::tz_database& timeZoneDatabase)
      : m_marketDataClient{std::forward<MarketDataClientForward>(
          marketDataClient)},
        m_timeClient{std::forward<TimeClientForward>(timeClient)},
        m_marketDatabase{marketDatabase},
        m_timeZoneDatabase{timeZoneDatabase} {}

  template<typename MarketDataClientType, typename TimeClientType>
  void BoardLotCheck<MarketDataClientType, TimeClientType>::Submit(
      const OrderInfo& orderInfo) {
    if(orderInfo.m_fields.m_security.GetMarket() != DefaultMarkets::TSX() &&
        orderInfo.m_fields.m_security.GetMarket() != DefaultMarkets::TSXV()) {
      return;
    }
    auto currentPrice = LoadPrice(orderInfo.m_fields.m_security);
    if(currentPrice <= 10 * Money::CENT) {
      if(orderInfo.m_fields.m_quantity % 1000 != 0) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
          "Quantity must be a multiple of 1000."});
      }
    } else if(currentPrice < Money::ONE) {
      if(orderInfo.m_fields.m_quantity % 500 != 0) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
          "Quantity must be a multiple of 500."});
      }
    } else {
      if(orderInfo.m_fields.m_quantity % 100 != 0) {
        BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
          "Quantity must be a multiple of 100."});
      }
    }
  }

  template<typename MarketDataClientType, typename TimeClientType>
  Money BoardLotCheck<MarketDataClientType, TimeClientType>::LoadPrice(
      const Security& security) {
    auto& closingEntry = m_closingEntries.Get(security);
    auto closingPrice = Beam::Threading::With(closingEntry,
      [&] (ClosingEntry& entry) {
        auto currentTime = m_timeClient->GetTime();
        if(currentTime - entry.m_lastUpdate > boost::posix_time::hours(1)) {
          auto previousClose = TechnicalAnalysis::LoadPreviousClose(
            *m_marketDataClient, security, currentTime, m_marketDatabase,
            m_timeZoneDatabase, "");
          if(previousClose.is_initialized()) {
            entry.m_closingPrice = previousClose->m_price;
          } else {
            entry.m_closingPrice = Money::ZERO;
          }
          entry.m_lastUpdate = currentTime;
        }
        return entry.m_closingPrice;
      });
    if(closingPrice != Money::ZERO) {
      return closingPrice;
    }
    auto publisher = m_bboQuotes.GetOrInsert(security,
      [&] {
        auto publisher = std::make_shared<Beam::StateQueue<BboQuote>>();
        MarketDataService::QueryRealTimeWithSnapshot(*m_marketDataClient,
          publisher);
        return publisher;
      });
    try {
      return publisher->Top().m_bid.m_price;
    } catch(const Beam::PipeBrokenException&) {
      m_bboQuotes.Erase(security);
      BOOST_THROW_EXCEPTION(OrderSubmissionCheckException{
        "No BBO quote available."});
    }
  }
}
}

#endif
