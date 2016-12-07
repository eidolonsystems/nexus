#ifndef NEXUS_CHARTINGSERVLET_HPP
#define NEXUS_CHARTINGSERVLET_HPP
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/UniquePtr.hpp>
#include <Beam/Queries/ConversionEvaluatorNode.hpp>
#include <Beam/Queries/IndexedExpressionSubscriptions.hpp>
#include <Beam/Queries/ExpressionSubscriptions.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ChartingService/ChartingService.hpp"
#include "Nexus/ChartingService/ChartingServices.hpp"
#include "Nexus/MarketDataService/CachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/ClientHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/HistoricalDataStoreUtilities.hpp"
#include "Nexus/MarketDataService/MarketDataClientUtilities.hpp"
#include "Nexus/Queries/ShuttleQueryTypes.hpp"
#include "Nexus/TechnicalAnalysis/CandlestickTypes.hpp"

namespace Nexus {
namespace ChartingService {
namespace Details {
  struct ExpressionConverter {
    template<typename T>
    static std::unique_ptr<Beam::Queries::BaseEvaluatorNode> Template(
        std::unique_ptr<Beam::Queries::BaseEvaluatorNode> baseExpression) {
      auto expression = Beam::UniqueStaticCast<Beam::Queries::EvaluatorNode<T>>(
        std::move(baseExpression));
      return Beam::Queries::MakeConstructEvaluatorNode<
        T, Queries::QueryVariant>(std::move(expression));
    }

    using SupportedTypes = Queries::QueryTypes::NativeTypes;
  };
}

  /*! \class ChartingServlet
      \brief Provides historical and charting related data.
      \tparam ContainerType The container instantiating this servlet.
      \tparam MarketDataClientType The type of MarketDataClient used to access
              real-time data.
   */
  template<typename ContainerType, typename MarketDataClientType>
  class ChartingServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of MarketDataClient used.
      using MarketDataClient = Beam::GetTryDereferenceType<
        MarketDataClientType>;

      //! Constructs a ChartingServlet.
      /*!
        \param marketDataClient Initializes the MarketDataClient.
      */
      template<typename MarketDataClientForward>
      ChartingServlet(MarketDataClientForward&& marketDataClient);

      void RegisterServices(Beam::Out<Beam::Services::ServiceSlots<
        ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      template<typename MarketDataType>
      struct QueryEntry {
        using Query = MarketDataService::GetMarketDataQueryType<MarketDataType>;
        Beam::Queries::IndexedExpressionSubscriptions<
          typename MarketDataType::Value, Queries::QueryVariant,
          typename Query::Index, ServiceProtocolClient> m_queries;
        Beam::SynchronizedUnorderedSet<Security, Beam::Threading::Mutex>
          m_realTimeSubscriptions;
      };
      Beam::GetOptionalLocalPtr<MarketDataClientType> m_marketDataClient;
      MarketDataService::CachedHistoricalDataStore<
        MarketDataService::ClientHistoricalDataStore<MarketDataClient*>>
        m_dataStore;
      QueryEntry<SequencedTimeAndSale> m_timeAndSaleQueries;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_taskQueue;

      void Shutdown();
      void OnQuerySecurityRequest(Beam::Services::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int clientQueryId);
      void OnEndSecurityQuery(ServiceProtocolClient& client, int id);
      TimePriceQueryResult OnLoadSecurityTimePriceSeriesRequest(
        ServiceProtocolClient& client, const Security& security,
        const boost::posix_time::ptime& startTime,
        const boost::posix_time::ptime& endTime,
        const boost::posix_time::time_duration& interval);
      template<typename MarketDataType>
      void HandleQuery(Beam::Services::RequestToken<
        ServiceProtocolClient, QuerySecurityService>& request,
        const SecurityChartingQuery& query, int clientQueryId,
        QueryEntry<MarketDataType>& queryEntry);
      template<typename Index, typename MarketDataType>
      void OnQueryUpdate(const Index& index, const MarketDataType& value,
        QueryEntry<MarketDataType>& queryEntry);
  };

  template<typename MarketDataClientType>
  struct MetaChartingServlet {
    using Session = Beam::NullType;
    template<typename ContainerType>
    struct apply {
      using type = ChartingServlet<ContainerType, MarketDataClientType>;
    };
  };

  template<typename ContainerType, typename MarketDataClientType>
  template<typename MarketDataClientForward>
  ChartingServlet<ContainerType, MarketDataClientType>::ChartingServlet(
      MarketDataClientForward&& marketDataClient)
      : m_marketDataClient{std::forward<MarketDataClientForward>(
          marketDataClient)},
        m_dataStore{Beam::Initialize(&*m_marketDataClient), 10000} {}

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::RegisterServices(
      Beam::Out<Beam::Services::ServiceSlots<ServiceProtocolClient>>
      slots) {
    Queries::RegisterQueryTypes(Beam::Store(slots->GetRegistry()));
    RegisterChartingServices(Store(slots));
    RegisterChartingMessages(Store(slots));
    QuerySecurityService::AddRequestSlot(Store(slots), std::bind(
      &ChartingServlet::OnQuerySecurityRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    Beam::Services::AddMessageSlot<EndSecurityQueryMessage>(Store(slots),
      std::bind(&ChartingServlet::OnEndSecurityQuery, this,
      std::placeholders::_1, std::placeholders::_2));
    LoadSecurityTimePriceSeriesService::AddSlot(Store(slots), std::bind(
      &ChartingServlet::OnLoadSecurityTimePriceSeriesRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4, std::placeholders::_5));
  }

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_timeAndSaleQueries.m_queries.RemoveAll(client);
  }

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataClient->Open();
      m_dataStore.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::Shutdown() {
    m_marketDataClient->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::
      OnQuerySecurityRequest(
      Beam::Services::RequestToken<ServiceProtocolClient, QuerySecurityService>&
      request, const SecurityChartingQuery& query, int clientQueryId) {
    auto& session = request.GetSession();
    if(query.GetMarketDataType() ==
        MarketDataService::MarketDataType::TIME_AND_SALE) {
      HandleQuery(request, query, clientQueryId, m_timeAndSaleQueries);
    } else {
      SecurityChartingQueryResult result;
      request.SetResult(std::move(result));
    }
  }

  template<typename ContainerType, typename MarketDataClientType>
  void ChartingServlet<ContainerType, MarketDataClientType>::OnEndSecurityQuery(
      ServiceProtocolClient& client, int id) {
    auto& session = client.GetSession();
    m_timeAndSaleQueries.m_queries.End(client, id);
  }

  template<typename ContainerType, typename MarketDataClientType>
  TimePriceQueryResult ChartingServlet<ContainerType, MarketDataClientType>::
      OnLoadSecurityTimePriceSeriesRequest(ServiceProtocolClient& client,
      const Security& security, const boost::posix_time::ptime& startTime,
      const boost::posix_time::ptime& endTime,
      const boost::posix_time::time_duration& interval) {
    if(endTime < startTime + interval ||
        startTime == boost::posix_time::neg_infin  ||
        endTime == boost::posix_time::pos_infin) {
      throw Beam::Services::ServiceRequestException{"Invalid time range."};
    }
    auto queue = std::make_shared<Beam::Queue<SequencedTimeAndSale>>();
    MarketDataService::SecurityMarketDataQuery timeAndSaleQuery;
    timeAndSaleQuery.SetIndex(security);
    timeAndSaleQuery.SetRange(startTime, endTime);
    timeAndSaleQuery.SetSnapshotLimit(
      Beam::Queries::SnapshotLimit::Unlimited());
    m_marketDataClient->QueryTimeAndSales(timeAndSaleQuery, queue);
    std::vector<SequencedTimeAndSale> timeAndSales;
    Beam::FlushQueue(queue, std::back_inserter(timeAndSales));
    TimePriceQueryResult result;
    if(!timeAndSales.empty()) {
      result.start = timeAndSales.front().GetSequence();
      result.end = timeAndSales.back().GetSequence();
    }
    auto currentStart = startTime;
    auto currentEnd = startTime + interval;
    auto timeAndSalesIterator = timeAndSales.begin();
    while(timeAndSalesIterator != timeAndSales.end() &&
        currentStart <= endTime) {
      TechnicalAnalysis::TimePriceCandlestick candlestick{currentStart,
        currentEnd};
      auto hasPoint = false;
      while(timeAndSalesIterator != timeAndSales.end() &&
          (*timeAndSalesIterator)->m_timestamp < currentEnd) {
        candlestick.Update((*timeAndSalesIterator)->m_price);
        hasPoint = true;
        ++timeAndSalesIterator;
      }
      if(hasPoint) {
        result.series.push_back(candlestick);
      }
      currentStart = currentEnd;
      currentEnd = currentStart + interval;
    }
    return result;
  }

  template<typename ContainerType, typename MarketDataClientType>
  template<typename MarketDataType>
  void ChartingServlet<ContainerType, MarketDataClientType>::HandleQuery(
      Beam::Services::RequestToken<ServiceProtocolClient, QuerySecurityService>&
      request, const SecurityChartingQuery& query, int clientQueryId,
      QueryEntry<MarketDataType>& queryEntry) {
    using Query = MarketDataService::GetMarketDataQueryType<MarketDataType>;
    if(query.GetRange().GetEnd() == Beam::Queries::Sequence::Last()) {
      queryEntry.m_realTimeSubscriptions.TestAndSet(query.GetIndex(),
        [&] {
          Query realTimeQuery;
          realTimeQuery.SetIndex(query.GetIndex());
          realTimeQuery.SetRange(Beam::Queries::Range::RealTime());
          MarketDataService::QueryMarketDataClient<MarketDataType>(
            *m_marketDataClient, realTimeQuery,
            m_taskQueue.GetSlot<MarketDataType>(std::bind(
            &ChartingServlet::OnQueryUpdate<
            typename Query::Index, MarketDataType>, this, query.GetIndex(),
            std::placeholders::_1, std::ref(queryEntry))));
        });
    }
    SecurityChartingQueryResult result;
    result.m_queryId = clientQueryId;
    auto filter = Beam::Queries::Translate<Queries::EvaluatorTranslator>(
      query.GetFilter());
    Queries::EvaluatorTranslator translator;
    translator.Translate(query.GetExpression());
    auto baseExpression = std::move(translator.GetEvaluator());
    auto expression = Beam::Instantiate<Details::ExpressionConverter>(
      baseExpression->GetResultType())(std::move(baseExpression));
    auto expressionEvaluator = std::make_unique<Beam::Queries::Evaluator>(
      std::move(expression), translator.GetParameters());
    queryEntry.m_queries.Initialize(query.GetIndex(), request.GetClient(),
      clientQueryId, query.GetRange(), std::move(filter),
      query.GetUpdatePolicy(), std::move(expressionEvaluator));
    Query snapshotQuery = query;
    snapshotQuery.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    auto snapshot = MarketDataService::HistoricalDataStoreLoad<MarketDataType>(
      m_dataStore, snapshotQuery);
    queryEntry.m_queries.Commit(query.GetIndex(), request.GetClient(),
      query.GetSnapshotLimit(), std::move(result), std::move(snapshot),
      [&] (const SecurityChartingQueryResult& result) {
        request.SetResult(result);
      });
  }

  template<typename ContainerType, typename MarketDataClientType>
  template<typename Index, typename MarketDataType>
  void ChartingServlet<ContainerType, MarketDataClientType>::OnQueryUpdate(
      const Index& index, const MarketDataType& value,
      QueryEntry<MarketDataType>& queries) {
    auto indexedValue = Beam::Queries::MakeSequencedValue(
      Beam::Queries::MakeIndexedValue(*value, index), value.GetSequence());
    m_dataStore.Store(indexedValue);
    queries.m_queries.Publish(indexedValue,
      [&] (ServiceProtocolClient& client, int id,
          const Queries::SequencedQueryVariant& value) {
        Beam::Services::SendRecordMessage<SecurityQueryMessage>(client, id,
          value);
      });
  }
}
}

#endif
