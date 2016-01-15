#ifndef NEXUS_RISKSERVICE_HPP
#define NEXUS_RISKSERVICE_HPP
#include <string>

namespace Nexus {
namespace RiskService {
  struct AccountRecord;
  template<typename ServiceProtocolClientBuilderType> class RiskClient;
  struct RiskParameters;
  struct RiskPositionUpdate;
  struct RiskProfitAndLossUpdate;
  template<typename ContainerType, typename AdministrationClientType,
    typename OrderExecutionClientType, typename RiskStateMonitorType>
    class RiskServlet;
  class RiskSession;
  struct RiskState;
  template<typename RiskStateTrackerType, typename AdministrationClientType,
    typename MarketDataClientType, typename TransitionTimerType,
    typename TimeClientType> class RiskStateMonitor;
  template<typename PortfolioType, typename TimeClientType>
    class RiskStateTracker;
  struct RiskStateUpdate;
  template<typename AdministrationClientType, typename OrderExecutionClientType>
    class RiskTransitionController;
  template<typename OrderExecutionClientType> class RiskTransitionTracker;
  class VirtualRiskClient;
  template<typename ClientType> class WrapperRiskClient;

  // Standard name for the risk service.
  static const std::string SERVICE_NAME = "risk_service";
}
}

#endif
