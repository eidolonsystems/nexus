#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Compliance/ComplianceServlet.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;

namespace {
  struct Fixture {
    using TestComplianceRuleSet = ComplianceRuleSet<
      LocalComplianceRuleDataStore*, ServiceLocatorClientBox>;
    using ServletContainer = TestAuthenticatedServiceProtocolServletContainer<
      MetaComplianceServlet<ServiceLocatorClientBox, AdministrationClientBox,
        TestComplianceRuleSet*, IncrementalTimeClient>>;

    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    ServiceLocatorClientBox m_serviceLocatorClient;
    AdministrationServiceTestEnvironment m_administrationServiceEnvironment;
    LocalComplianceRuleDataStore m_dataStore;

    Fixture()
      : m_serviceLocatorClient(m_serviceLocatorEnvironment.MakeClient()),
        m_administrationServiceEnvironment(m_serviceLocatorClient) {}
  };
}
