#ifndef NEXUS_MARKET_DATA_SERVICE_HPP
#define NEXUS_MARKET_DATA_SERVICE_HPP
#include <string>

namespace Nexus::MarketDataService {
  class ApplicationMarketDataClient;
  template<typename HistoricalDataStoreType> class BufferedHistoricalDataStore;
  template<typename HistoricalDataStoreType> class CachedHistoricalDataStore;
  template<typename MarketDataClientType> class ClientHistoricalDataStore;
  template<typename D> class DataStoreMarketDataClient;
  template<typename MarketExpressionType> class DefaultCurrencyExpression;
  class DistributedMarketDataClient;
  class EntitlementDatabase;
  struct EntitlementKey;
  class EntitlementSet;
  struct HistoricalDataStore;
  class HistoricalDataStoreException;
  template<typename T, typename DataStoreType>
    class HistoricalDataStoreQueryWrapper;
  class LocalHistoricalDataStore;
  template<typename ServiceProtocolClientBuilderType> class MarketDataClient;
  template<typename OrderIdType, typename SamplingTimerType,
    typename MessageProtocolType, typename HeartbeatTimerType>
    class MarketDataFeedClient;
  template<typename ContainerType, typename MarketDataRegistryType>
    class MarketDataFeedServlet;
  template<typename MarketDataType> struct MarketDataQueryType;
  class MarketDataRegistry;
  template<typename ContainerType, typename MarketDataRegistryType,
    typename HistoricalDataStoreType, typename AdministrationClientType>
    class MarketDataRegistryServlet;
  class MarketDataRegistrySession;
  template<typename ContainerType, typename MarketDataClientType,
    typename AdministrationClientType> class MarketDataRelayServlet;
  class MarketEntry;
  template<typename MarketDataClientType, typename MarketExpressionType,
    typename TimeRangeExpressionType> class MarketOrderImbalanceExpression;
  template<typename MarketDataClientType> class RealTimeMarketDataPublisher;
  class SecurityEntry;
  struct SecuritySnapshot;
  template<typename HistoricalDataStoreType>
    class SessionCachedHistoricalDataStore;
  template<typename C> class SqlHistoricalDataStore;
  class VirtualHistoricalDataStore;
  class VirtualMarketDataClient;
  class VirtualMarketDataFeedClient;
  template<typename C> class WrapperHistoricalDataStore;
  template<typename ClientType> class WrapperMarketDataClient;
  template<typename ClientType> class WrapperMarketDataFeedClient;

  // Standard name for the market data feed service.
  static const std::string FEED_SERVICE_NAME = "market_data_feed_service";

  // Standard name for the market data registry service.
  static const std::string REGISTRY_SERVICE_NAME =
    "market_data_registry_service";

  // Standard name for the market data relay service.
  static const std::string RELAY_SERVICE_NAME = "market_data_relay_service";
}

#endif
