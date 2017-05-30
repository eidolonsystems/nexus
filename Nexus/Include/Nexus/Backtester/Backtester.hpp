#ifndef NEXUS_BACKTESTER_HPP
#define NEXUS_BACKTESTER_HPP

namespace Nexus {
  class BacktesterEvent;
  class BacktesterEventHandler;
  class BacktesterMarketDataClient;
  class BacktesterOrderExecutionClient;
  class BacktesterServiceClients;
  class BacktesterTimer;
  template<typename IndexType, typename MarketDataTypeType>
    class MarketDataEvent;
  template<typename MarketDataTypeType> class MarketDataLoadEvent;
  template<typename MarketDataTypeType> class MarketDataQueryEvent;
  class TimerBacktesterEvent;
}

#endif
