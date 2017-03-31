#include "Nexus/OrderExecutionServiceTests/OrderExecutionServletTester.hpp"
#include <Beam/ServiceLocator/NullAuthenticator.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;
using namespace std;

void OrderExecutionServletTester::setUp() {
  m_serviceLocatorEnvironment.Initialize();
  m_serviceLocatorEnvironment->Open();
  auto servicesDirectory =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory("services",
    DirectoryEntry::GetStarDirectory());
  auto administratorsDirectory =
    m_serviceLocatorEnvironment->GetRoot().MakeDirectory("administrators",
    DirectoryEntry::GetStarDirectory());
  auto administrationAccount =
    m_serviceLocatorEnvironment->GetRoot().MakeAccount(
    "administration_service", "", servicesDirectory);
  m_serviceLocatorEnvironment->GetRoot().StorePermissions(
    administrationAccount, DirectoryEntry::GetStarDirectory(),
    Permissions(~0));
  m_serviceLocatorEnvironment->GetRoot().MakeAccount("order_execution_service",
    "", servicesDirectory);
  auto clientEntry = m_serviceLocatorEnvironment->GetRoot().MakeAccount(
    "client", "", DirectoryEntry::GetStarDirectory());
  m_uidServiceEnvironment.Initialize();
  m_uidServiceEnvironment->Open();
  auto administationServiceLocatorClient =
    m_serviceLocatorEnvironment->BuildClient();
  administationServiceLocatorClient->SetCredentials("administration_service",
    "");
  administationServiceLocatorClient->Open();
  m_administrationServiceEnvironment.Initialize(
    std::move(administationServiceLocatorClient));
  m_administrationServiceEnvironment->Open();
  m_servletServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  m_servletServiceLocatorClient->SetCredentials("order_execution_service", "");
  m_servletServiceLocatorClient->Open();
  m_serverConnection.Initialize();
  m_clientProtocol.Initialize(Initialize(string{"test"},
    Ref(*m_serverConnection)), Initialize());
  RegisterQueryTypes(Store(m_clientProtocol->GetSlots().GetRegistry()));
  RegisterOrderExecutionServices(Store(m_clientProtocol->GetSlots()));
  RegisterOrderExecutionMessages(Store(m_clientProtocol->GetSlots()));
  m_driver.Initialize();
  m_dataStore.Initialize();
  m_servlet.Initialize(boost::posix_time::pos_infin,
    GetDefaultMarketDatabase(), GetDefaultDestinationDatabase(), Initialize(),
    m_servletServiceLocatorClient.get(),
    m_uidServiceEnvironment->BuildClient(),
    m_administrationServiceEnvironment->BuildClient(
    Ref(*m_servletServiceLocatorClient)), &*m_driver, &*m_dataStore);
  m_container.Initialize(Initialize(&*m_servletServiceLocatorClient,
    &*m_servlet), &*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientServiceLocatorClient = m_serviceLocatorEnvironment->BuildClient();
  m_clientServiceLocatorClient->SetCredentials("client", "");
  m_clientServiceLocatorClient->Open();
  m_clientProtocol->Open();
  SessionAuthenticator<ServiceLocatorClient> authenticator{
    Ref(*m_clientServiceLocatorClient)};
  authenticator(*m_clientProtocol);
  m_clientProtocol->SpawnMessageHandler();
  AccountQuery orderSubmissionQuery;
  orderSubmissionQuery.SetIndex(m_clientServiceLocatorClient->GetAccount());
  orderSubmissionQuery.SetRange(Beam::Queries::Range::RealTime());
  m_clientProtocol->SendRequest<QueryOrderSubmissionsService>(
    orderSubmissionQuery);
}

void OrderExecutionServletTester::tearDown() {
  m_clientProtocol.Reset();
  m_clientServiceLocatorClient.reset();
  m_container.Reset();
  m_servlet.Reset();
  m_servletServiceLocatorClient.reset();
  m_dataStore.Reset();
  m_driver.Reset();
  m_serverConnection.Reset();
  m_administrationServiceEnvironment.Reset();
  m_uidServiceEnvironment.Reset();
  m_serviceLocatorEnvironment.Reset();
}

void OrderExecutionServletTester::TestNewOrderSingle() {
  auto orderFields = OrderFields::BuildLimitOrder(
    m_clientServiceLocatorClient->GetAccount(),
    Security{"TST", DefaultMarkets::NYSE(), DefaultCountries::US()},
    DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
  ExecutionReport report;
  Async<void> messageAsync;
  AddMessageSlot<OrderUpdateMessage>(Store(m_clientProtocol->GetSlots()),
    [&] (ClientServiceProtocolClient& client,
        const ExecutionReport& receivedReport) {
      report = receivedReport;
      messageAsync.GetEval().SetResult();
    });
  auto newOrder = m_clientProtocol->SendRequest<NewOrderSingleService>(
    orderFields);
  auto driverMonitor = std::make_shared<Queue<PrimitiveOrder*>>();
  m_driver->GetPublisher().Monitor(driverMonitor);
  auto receivedOrder = driverMonitor->Top();
  driverMonitor->Pop();
  messageAsync.Get();
  messageAsync.Reset();
  CPPUNIT_ASSERT(report.m_status == OrderStatus::PENDING_NEW);
  ExecutionReport receivedOrderReport;
  receivedOrderReport.m_id = receivedOrder->GetInfo().m_orderId;
  receivedOrderReport = ExecutionReport::BuildUpdatedReport(report,
    OrderStatus::NEW, microsec_clock::universal_time());
  receivedOrder->Update(receivedOrderReport);
  messageAsync.Get();
  messageAsync.Reset();
  CPPUNIT_ASSERT(report.m_status == OrderStatus::NEW);
  receivedOrderReport = ExecutionReport::BuildUpdatedReport(report,
    OrderStatus::EXPIRED, microsec_clock::universal_time());
  receivedOrder->Update(receivedOrderReport);
  messageAsync.Get();
  CPPUNIT_ASSERT(report.m_status == OrderStatus::EXPIRED);
}
