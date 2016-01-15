#ifndef NEXUS_MARKETDATAREGISTRYSERVLET_HPP
#define NEXUS_MARKETDATAREGISTRYSERVLET_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queries/IndexedSubscriptions.hpp>
#include <Beam/Services/ServiceProtocolServlet.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/EntitlementDatabase.hpp"
#include "Nexus/MarketDataService/MarketDataRegistry.hpp"
#include "Nexus/MarketDataService/MarketDataRegistryServices.hpp"
#include "Nexus/MarketDataService/MarketDataRegistrySession.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Nexus/Queries/EvaluatorTranslator.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class MarketDataRegistryServlet
      \brief Maintains a registry of all Securities and data subscriptions.
      \tparam ContainerType The container instantiating this servlet.
      \tparam MarketDataRegistryType The registry storing all market data
              originating from this servlet.
      \tparam HistoricalDataStoreType The type of data store storing historical
              market data.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient to use.
   */
  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  class MarketDataRegistryServlet : private boost::noncopyable {
    public:

      //! The registry storing all market data sent to this servlet.
      using MarketDataRegistry = Beam::GetTryDereferenceType<
        MarketDataRegistryType>;

      //! The type of data store storing historical market data.
      using HistoricalDataStore = Beam::GetTryDereferenceType<
        HistoricalDataStoreType>;
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of ServiceLocatorClient to use.
      using ServiceLocatorClient = ServiceLocatorClientType;

      //! Constructs a MarketDataRegistryServlet.
      /*!
        \param entitlementDatabase The database of all market data entitlements.
        \param serviceLocatorClient Used to check for entitlements.
        \param marketDataRegistry The registry storing all market data
               originating from this servlet.
        \param dataStore Initializes the historical market data store.
      */
      template<typename MarketDataRegistryForward,
        typename HistoricalDataStoreForward>
      MarketDataRegistryServlet(const EntitlementDatabase& entitlementDatabase,
        Beam::RefType<ServiceLocatorClient> serviceLocatorClient,
        MarketDataRegistryForward&& marketDataRegistry,
        HistoricalDataStoreForward&& dataStore);

      void Add(const SecurityInfo& securityInfo);

      void PublishOrderImbalance(const MarketOrderImbalance& orderImbalance,
        int sourceId);

      void PublishBboQuote(const SecurityBboQuote& bboQuote, int sourceId);

      void PublishMarketQuote(const SecurityMarketQuote& marketQuote,
        int sourceId);

      void UpdateBookQuote(const SecurityBookQuote& delta, int sourceId);

      void PublishTimeAndSale(const SecurityTimeAndSale& timeAndSale,
        int sourceId);

      void Clear(int sourceId);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      template<typename T>
      using MarketSubscriptions = Beam::Queries::IndexedSubscriptions<
        T, MarketCode, ServiceProtocolClient>;
      template<typename T>
      using SecuritySubscriptions = Beam::Queries::IndexedSubscriptions<
        T, Security, ServiceProtocolClient>;
      EntitlementDatabase m_entitlementDatabase;
      ServiceLocatorClient* m_serviceLocatorClient;
      Beam::GetOptionalLocalPtr<MarketDataRegistryType> m_registry;
      Beam::GetOptionalLocalPtr<HistoricalDataStoreType> m_dataStore;
      MarketSubscriptions<OrderImbalance> m_orderImbalanceSubscriptions;
      SecuritySubscriptions<BboQuote> m_bboQuoteSubscriptions;
      SecuritySubscriptions<BookQuote> m_bookQuoteSubscriptions;
      SecuritySubscriptions<MarketQuote> m_marketQuoteSubscriptions;
      SecuritySubscriptions<TimeAndSale> m_timeAndSaleSubscriptions;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void OnQueryOrderImbalances(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryOrderImbalancesService>& request,
        const MarketWideDataQuery& query);
      void OnEndOrderImbalanceQuery(ServiceProtocolClient& client,
        MarketCode market, int id);
      void OnQueryBboQuotes(Beam::Services::RequestToken<ServiceProtocolClient,
        QueryBboQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndBboQuoteQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      void OnQueryBookQuotes(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryBookQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndBookQuoteQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      void OnQueryMarketQuotes(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryMarketQuotesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndMarketQuoteQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      void OnQueryTimeAndSales(Beam::Services::RequestToken<
        ServiceProtocolClient, QueryTimeAndSalesService>& request,
        const SecurityMarketDataQuery& query);
      void OnEndTimeAndSaleQuery(ServiceProtocolClient& client,
        const Security& security, int id);
      SecuritySnapshot OnLoadSecuritySnapshot(ServiceProtocolClient& client,
        const Security& security);
      SecurityTechnicals OnLoadSecurityTechnicals(ServiceProtocolClient& client,
        const Security& security);
      std::vector<SecurityInfo> OnLoadSecurityInfoFromPrefix(
        ServiceProtocolClient& client, const std::string& prefix);
  };

  template<typename MarketDataRegistryType, typename HistoricalDataStoreType,
    typename ServiceLocatorClientType>
  struct MetaMarketDataRegistryServlet {
    using Session = MarketDataRegistrySession;
    template<typename ContainerType>
    struct apply {
      using type = MarketDataRegistryServlet<ContainerType,
        MarketDataRegistryType, HistoricalDataStoreType,
        ServiceLocatorClientType>;
    };
  };

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  template<typename MarketDataRegistryForward,
    typename HistoricalDataStoreForward>
  MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::
      MarketDataRegistryServlet(const EntitlementDatabase& entitlementDatabase,
      Beam::RefType<ServiceLocatorClient> serviceLocatorClient,
      MarketDataRegistryForward&& registry,
      HistoricalDataStoreForward&& dataStore)
      : m_entitlementDatabase(entitlementDatabase),
        m_serviceLocatorClient(serviceLocatorClient.Get()),
        m_registry(std::forward<MarketDataRegistryForward>(registry)),
        m_dataStore(std::forward<HistoricalDataStoreForward>(dataStore)) {}

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::Add(
      const SecurityInfo& securityInfo) {
    m_registry->Add(securityInfo);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::PublishOrderImbalance(
      const MarketOrderImbalance& orderImbalance, int sourceId) {
    m_registry->PublishOrderImbalance(orderImbalance, sourceId,
      [&] {
        return m_dataStore->LoadInitialSequences(orderImbalance->m_security);
      },
      [&] {
        return m_dataStore->LoadInitialSequences(orderImbalance.GetIndex());
      },
      [&] (const SequencedMarketOrderImbalance& orderImbalance) {
        m_dataStore->Store(orderImbalance);
        m_orderImbalanceSubscriptions.Publish(orderImbalance,
          [&] (const std::vector<ServiceProtocolClient*>& clients) {
            Beam::Services::BroadcastRecordMessage<OrderImbalanceMessage>(
              clients, orderImbalance);
          });
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::PublishBboQuote(
      const SecurityBboQuote& bboQuote, int sourceId) {
    m_registry->PublishBboQuote(bboQuote, sourceId,
      [&] {
        return m_dataStore->LoadInitialSequences(bboQuote.GetIndex());
      },
      [&] (const SequencedSecurityBboQuote& bboQuote) {
        m_dataStore->Store(bboQuote);
        m_bboQuoteSubscriptions.Publish(bboQuote,
          [&] (const std::vector<ServiceProtocolClient*>& clients) {
            Beam::Services::BroadcastRecordMessage<BboQuoteMessage>(clients,
              bboQuote);
          });
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::PublishMarketQuote(
      const SecurityMarketQuote& marketQuote, int sourceId) {
    m_registry->PublishMarketQuote(marketQuote, sourceId,
      [&] {
        return m_dataStore->LoadInitialSequences(marketQuote.GetIndex());
      },
      [&] (const SequencedSecurityMarketQuote& marketQuote) {
        m_dataStore->Store(marketQuote);
        m_marketQuoteSubscriptions.Publish(marketQuote,
          [&] (const std::vector<ServiceProtocolClient*>& clients) {
            Beam::Services::BroadcastRecordMessage<MarketQuoteMessage>(clients,
              marketQuote);
          });
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::UpdateBookQuote(
      const SecurityBookQuote& delta, int sourceId) {
    auto security = m_registry->GetPrimaryListing(delta.GetIndex());
    auto key = EntitlementKey{security.GetMarket(), delta.GetValue().m_market};
    m_registry->UpdateBookQuote(delta, sourceId,
      [&] {
        return m_dataStore->LoadInitialSequences(delta.GetIndex());
      },
      [&] (const SequencedSecurityBookQuote& bookQuote) {
        m_dataStore->Store(bookQuote);
        if(security.GetMarket() == MarketCode{}) {
          return;
        }
        m_bookQuoteSubscriptions.Publish(bookQuote,
          [&] (const ServiceProtocolClient& client) {
            return client.GetSession().GetEntitlements().HasEntitlement(key,
              MarketDataType::BOOK_QUOTE);
          },
          [&] (const std::vector<ServiceProtocolClient*>& clients) {
            Beam::Services::BroadcastRecordMessage<BookQuoteMessage>(clients,
              bookQuote);
          });
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::PublishTimeAndSale(
      const SecurityTimeAndSale& timeAndSale, int sourceId) {
    m_registry->PublishTimeAndSale(timeAndSale, sourceId,
      [&] {
        return m_dataStore->LoadInitialSequences(timeAndSale.GetIndex());
      },
      [&] (const SequencedSecurityTimeAndSale& timeAndSale) {
        m_dataStore->Store(timeAndSale);
        m_timeAndSaleSubscriptions.Publish(timeAndSale,
          [&] (const std::vector<ServiceProtocolClient*>& clients) {
            Beam::Services::BroadcastRecordMessage<TimeAndSaleMessage>(clients,
              timeAndSale);
          });
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::Clear(int sourceId) {
    m_registry->Clear(sourceId);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>> slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterMarketDataRegistryServices(Beam::Store(slots));
    RegisterMarketDataRegistryMessages(Beam::Store(slots));
    QueryOrderImbalancesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnQueryOrderImbalances, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<EndOrderImbalanceQueryMessage>(Store(slots),
      std::bind(&MarketDataRegistryServlet::OnEndOrderImbalanceQuery, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    QueryBboQuotesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnQueryBboQuotes, this, std::placeholders::_1,
      std::placeholders::_2));
    Beam::Services::AddMessageSlot<EndBboQuoteQueryMessage>(Store(slots),
      std::bind(&MarketDataRegistryServlet::OnEndBboQuoteQuery, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    QueryBookQuotesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnQueryBookQuotes, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<EndBookQuoteQueryMessage>(Store(slots),
      std::bind(&MarketDataRegistryServlet::OnEndBookQuoteQuery, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    QueryMarketQuotesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnQueryMarketQuotes, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<EndMarketQuoteQueryMessage>(Store(slots),
      std::bind(&MarketDataRegistryServlet::OnEndMarketQuoteQuery, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    QueryTimeAndSalesService::AddRequestSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnQueryTimeAndSales, this,
      std::placeholders::_1, std::placeholders::_2));
    Beam::Services::AddMessageSlot<EndTimeAndSaleQueryMessage>(Store(slots),
      std::bind(&MarketDataRegistryServlet::OnEndTimeAndSaleQuery, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    LoadSecuritySnapshotService::AddSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnLoadSecuritySnapshot, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadSecurityTechnicalsService::AddSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnLoadSecurityTechnicals, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadSecurityInfoFromPrefixService::AddSlot(Store(slots), std::bind(
      &MarketDataRegistryServlet::OnLoadSecurityInfoFromPrefix, this,
      std::placeholders::_1, std::placeholders::_2));
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::HandleClientAccepted(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    auto parents = m_serviceLocatorClient->LoadParents(session.GetAccount());
    const auto& entitlements = m_entitlementDatabase.GetEntries();
    for(const auto& entitlement : entitlements) {
      auto entryIterator = std::find(parents.begin(), parents.end(),
        entitlement.m_groupEntry);
      if(entryIterator != parents.end()) {
        for(const auto& applicability : entitlement.m_applicability) {
          session.GetEntitlements().GrantEntitlement(applicability.first,
            applicability.second);
        }
      }
    }
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_orderImbalanceSubscriptions.RemoveAll(client);
    m_bboQuoteSubscriptions.RemoveAll(client);
    m_marketQuoteSubscriptions.RemoveAll(client);
    m_bookQuoteSubscriptions.RemoveAll(client);
    m_timeAndSaleSubscriptions.RemoveAll(client);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::Shutdown() {
    m_dataStore->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::
      OnQueryOrderImbalances(Beam::Services::RequestToken<ServiceProtocolClient,
      QueryOrderImbalancesService>& request, const MarketWideDataQuery& query) {
    auto& session = request.GetSession();
    if(!session.GetEntitlements().HasEntitlement(query.GetIndex(),
        MarketDataType::ORDER_IMBALANCE)) {
      OrderImbalanceQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    OrderImbalanceQueryResult result;
    result.m_queryId = m_orderImbalanceSubscriptions.Initialize(
      query.GetIndex(), request.GetClient(), query.GetRange(),
      std::move(filter));
    result.m_snapshot = m_dataStore->LoadOrderImbalances(query);
    m_orderImbalanceSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const OrderImbalanceQueryResult& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::
      OnEndOrderImbalanceQuery(ServiceProtocolClient& client, MarketCode market,
      int id) {
    auto& session = client.GetSession();
    m_orderImbalanceSubscriptions.End(market, id);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnQueryBboQuotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
      QueryBboQuotesService>& request, const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!session.GetEntitlements().HasEntitlement(query.GetIndex().GetMarket(),
        MarketDataType::BBO_QUOTE)) {
      BboQuoteQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    BboQuoteQueryResult result;
    result.m_queryId = m_bboQuoteSubscriptions.Initialize(query.GetIndex(),
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadBboQuotes(query);
    m_bboQuoteSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const BboQuoteQueryResult& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnEndBboQuoteQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    auto& session = client.GetSession();
    m_bboQuoteSubscriptions.End(security, id);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnQueryBookQuotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
      QueryBookQuotesService>& request, const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!session.GetEntitlements().HasEntitlement(query.GetIndex().GetMarket(),
        MarketDataType::BOOK_QUOTE)) {
      BookQuoteQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    BookQuoteQueryResult result;
    result.m_queryId = m_bookQuoteSubscriptions.Initialize(query.GetIndex(),
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadBookQuotes(query);
    m_bookQuoteSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const BookQuoteQueryResult& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnEndBookQuoteQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    auto& session = client.GetSession();
    m_bookQuoteSubscriptions.End(security, id);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnQueryMarketQuotes(
      Beam::Services::RequestToken<ServiceProtocolClient,
      QueryMarketQuotesService>& request,
      const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!session.GetEntitlements().HasEntitlement(query.GetIndex().GetMarket(),
        MarketDataType::MARKET_QUOTE)) {
      MarketQuoteQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    MarketQuoteQueryResult result;
    result.m_queryId = m_marketQuoteSubscriptions.Initialize(query.GetIndex(),
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadMarketQuotes(query);
    m_marketQuoteSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const MarketQuoteQueryResult& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnEndMarketQuoteQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    auto& session = client.GetSession();
    m_marketQuoteSubscriptions.End(security, id);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnQueryTimeAndSales(
      Beam::Services::RequestToken<ServiceProtocolClient,
      QueryTimeAndSalesService>& request,
      const SecurityMarketDataQuery& query) {
    auto& session = request.GetSession();
    if(!session.GetEntitlements().HasEntitlement(query.GetIndex().GetMarket(),
        MarketDataType::TIME_AND_SALE)) {
      TimeAndSaleQueryResult result;
      request.SetResult(result);
      return;
    }
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    TimeAndSaleQueryResult result;
    result.m_queryId = m_timeAndSaleSubscriptions.Initialize(query.GetIndex(),
      request.GetClient(), query.GetRange(), std::move(filter));
    result.m_snapshot = m_dataStore->LoadTimeAndSales(query);
    m_timeAndSaleSubscriptions.Commit(query.GetIndex(), std::move(result),
      [&] (const TimeAndSaleQueryResult& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  void MarketDataRegistryServlet<ContainerType, MarketDataRegistryType,
      HistoricalDataStoreType, ServiceLocatorClientType>::OnEndTimeAndSaleQuery(
      ServiceProtocolClient& client, const Security& security, int id) {
    auto& session = client.GetSession();
    m_timeAndSaleSubscriptions.End(security, id);
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  SecuritySnapshot MarketDataRegistryServlet<ContainerType,
      MarketDataRegistryType, HistoricalDataStoreType,
      ServiceLocatorClientType>::OnLoadSecuritySnapshot(
      ServiceProtocolClient& client, const Security& security) {
    auto& session = client.GetSession();
    auto securitySnapshot = m_registry->FindSnapshot(security);
    if(!securitySnapshot.is_initialized()) {
      return SecuritySnapshot();
    }
    if(!session.GetEntitlements().HasEntitlement(security.GetMarket(),
        MarketDataType::BBO_QUOTE)) {
      securitySnapshot->m_bboQuote = SequencedBboQuote();
    }
    if(!session.GetEntitlements().HasEntitlement(security.GetMarket(),
        MarketDataType::TIME_AND_SALE)) {
      securitySnapshot->m_timeAndSale = SequencedTimeAndSale();
    }
    if(!session.GetEntitlements().HasEntitlement(security.GetMarket(),
        MarketDataType::MARKET_QUOTE)) {
      securitySnapshot->m_marketQuotes.clear();
    }
    auto askEndRange = std::remove_if(securitySnapshot->m_askBook.begin(),
      securitySnapshot->m_askBook.end(),
      [&] (const BookQuote& bookQuote) {
        return !session.GetEntitlements().HasEntitlement(
          EntitlementKey(security.GetMarket(), bookQuote.m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot->m_askBook.erase(askEndRange,
      securitySnapshot->m_askBook.end());
    auto bidEndRange = std::remove_if(securitySnapshot->m_bidBook.begin(),
      securitySnapshot->m_bidBook.end(),
      [&] (const BookQuote& bookQuote) {
        return !session.GetEntitlements().HasEntitlement(
          EntitlementKey(security.GetMarket(), bookQuote.m_market),
          MarketDataType::BOOK_QUOTE);
      });
    securitySnapshot->m_bidBook.erase(bidEndRange,
      securitySnapshot->m_bidBook.end());
    return *securitySnapshot;
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  SecurityTechnicals MarketDataRegistryServlet<ContainerType,
      MarketDataRegistryType, HistoricalDataStoreType,
      ServiceLocatorClientType>::OnLoadSecurityTechnicals(
      ServiceProtocolClient& client, const Security& security) {
    auto& session = client.GetSession();
    auto securityTechnicals = m_registry->FindSecurityTechnicals(security);
    if(!securityTechnicals.is_initialized()) {
      return SecurityTechnicals();
    }
    return *securityTechnicals;
  }

  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename ServiceLocatorClientType>
  std::vector<SecurityInfo> MarketDataRegistryServlet<ContainerType,
      MarketDataRegistryType, HistoricalDataStoreType,
      ServiceLocatorClientType>::OnLoadSecurityInfoFromPrefix(
      ServiceProtocolClient& client, const std::string& prefix) {
    return m_registry->SearchSecurityInfo(prefix);
  }
}
}

#endif
