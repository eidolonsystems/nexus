#include <Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp>
#include <Beam/ServicesTests/ServicesTests.hpp>
#include <Beam/TimeService/IncrementalTimeClient.hpp>
#include <Beam/UidServiceTests/UidServiceTestEnvironment.hpp>
#include <boost/functional/factory.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationServiceTests/AdministrationServiceTestEnvironment.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionServlet.hpp"
#include "Nexus/OrderExecutionServiceTests/MockOrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace Nexus::Queries;

namespace {
  const auto TST = Security("TST", DefaultMarkets::NYSE(),
    DefaultCountries::US());

  struct Fixture {
    using TestServletContainer =
      TestAuthenticatedServiceProtocolServletContainer<
        MetaOrderExecutionServlet<IncrementalTimeClient,
          ServiceLocatorClientBox, UidClientBox, AdministrationClientBox,
          std::shared_ptr<MockOrderExecutionDriver>,
          std::shared_ptr<TestOrderExecutionDataStore>>>;
    ServiceLocatorTestEnvironment m_serviceLocatorEnvironment;
    UidServiceTestEnvironment m_uidServiceEnvironment;
    optional<AdministrationServiceTestEnvironment>
      m_administrationServiceEnvironment;
    optional<ServiceLocatorClientBox> m_clientServiceLocatorClient;
    std::shared_ptr<MockOrderExecutionDriver> m_driver;
    std::shared_ptr<LocalOrderExecutionDataStore> m_baseDataStore;
    std::shared_ptr<TestOrderExecutionDataStore> m_dataStore;
    optional<TestServletContainer> m_container;
    optional<TestServiceProtocolClient> m_clientProtocol;

    Fixture() {
      auto servicesDirectory =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory("services",
          DirectoryEntry::GetStarDirectory());
      auto administratorsDirectory =
        m_serviceLocatorEnvironment.GetRoot().MakeDirectory("administrators",
          DirectoryEntry::GetStarDirectory());
      auto administrationAccount =
        m_serviceLocatorEnvironment.GetRoot().MakeAccount(
          "administration_service", "", servicesDirectory);
      m_serviceLocatorEnvironment.GetRoot().StorePermissions(
        administrationAccount, DirectoryEntry::GetStarDirectory(),
        Permissions(~0));
      m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "order_execution_service", "", servicesDirectory);
      auto clientEntry = m_serviceLocatorEnvironment.GetRoot().MakeAccount(
        "client", "", DirectoryEntry::GetStarDirectory());
      m_administrationServiceEnvironment.emplace(
        m_serviceLocatorEnvironment.MakeClient("administration_service", ""));
      auto servletServiceLocatorClient = m_serviceLocatorEnvironment.MakeClient(
        "order_execution_service", "");
      m_driver = std::make_shared<MockOrderExecutionDriver>();
      m_baseDataStore = std::make_shared<LocalOrderExecutionDataStore>();
      m_dataStore = std::make_shared<TestOrderExecutionDataStore>(
        OrderExecutionDataStoreBox(m_baseDataStore));
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_container.emplace(Initialize(servletServiceLocatorClient,
        Initialize(pos_infin, GetDefaultMarketDatabase(),
          GetDefaultDestinationDatabase(), Initialize(),
          servletServiceLocatorClient, m_uidServiceEnvironment.MakeClient(),
          m_administrationServiceEnvironment->MakeClient(
            servletServiceLocatorClient), m_driver, m_dataStore)),
          serverConnection, factory<std::unique_ptr<TriggerTimer>>());
      m_clientProtocol.emplace(Initialize("test", *serverConnection),
        Initialize());
      Nexus::Queries::RegisterQueryTypes(
        Store(m_clientProtocol->GetSlots().GetRegistry()));
      RegisterOrderExecutionServices(Store(m_clientProtocol->GetSlots()));
      RegisterOrderExecutionMessages(Store(m_clientProtocol->GetSlots()));
      m_clientServiceLocatorClient.emplace(
        m_serviceLocatorEnvironment.MakeClient("client", ""));
      auto authenticator = SessionAuthenticator(*m_clientServiceLocatorClient);
      authenticator(*m_clientProtocol);
      m_clientProtocol->SpawnMessageHandler();
      auto orderSubmissionQuery = AccountQuery();
      orderSubmissionQuery.SetIndex(m_clientServiceLocatorClient->GetAccount());
      orderSubmissionQuery.SetRange(Beam::Queries::Range::RealTime());
      m_clientProtocol->SendRequest<QueryOrderSubmissionsService>(
        orderSubmissionQuery);
    }
  };
}

TEST_SUITE("OrderExecutionServlet") {
  TEST_CASE_FIXTURE(Fixture, "new_order_single") {
    auto clientReportAsync = Async<ExecutionReport>();
    AddMessageSlot<OrderUpdateMessage>(Store(m_clientProtocol->GetSlots()),
      [&] (auto& client, auto& report) {
        clientReportAsync.GetEval().SetResult(report);
      });
    auto orderFields = OrderFields::BuildLimitOrder(
      m_clientServiceLocatorClient->GetAccount(), TST,
      DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
    m_clientProtocol->SendRequest<NewOrderSingleService>(orderFields);
    auto driverMonitor = std::make_shared<Queue<PrimitiveOrder*>>();
    m_driver->GetPublisher().Monitor(driverMonitor);
    auto serverOrder = driverMonitor->Pop();
    auto clientInitialReport = clientReportAsync.Get();
    clientReportAsync.Reset();
    REQUIRE(clientInitialReport.m_status == OrderStatus::PENDING_NEW);
    auto serverNewReport = ExecutionReport();
    serverNewReport.m_id = serverOrder->GetInfo().m_orderId;
    serverNewReport = ExecutionReport::BuildUpdatedReport(clientInitialReport,
      OrderStatus::NEW, microsec_clock::universal_time());
    serverOrder->Update(serverNewReport);
    auto clientNewReport = clientReportAsync.Get();
    clientReportAsync.Reset();
    REQUIRE(clientNewReport.m_status == OrderStatus::NEW);
    auto serverExpiredReport = ExecutionReport::BuildUpdatedReport(
      clientNewReport, OrderStatus::EXPIRED, microsec_clock::universal_time());
    serverOrder->Update(serverExpiredReport);
    auto clientExpiredReport = clientReportAsync.Get();
    REQUIRE(clientExpiredReport.m_status == OrderStatus::EXPIRED);
  }

  TEST_CASE_FIXTURE(Fixture, "query_order_ids") {
    auto account = m_clientServiceLocatorClient->GetAccount();
    auto orderA = OrderInfo(OrderFields::BuildLimitOrder(TST, Side::BID, 100,
      Money::ONE), 12, time_from_string("2020-03-12 16:06:12"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderA, account),
      Beam::Queries::Sequence(44)));
    auto orderB = OrderInfo(OrderFields::BuildLimitOrder(TST, Side::ASK, 200,
      Money::ONE), 17, time_from_string("2020-03-12 16:06:13"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderB, account),
      Beam::Queries::Sequence(45)));
    auto orderC = OrderInfo(OrderFields::BuildLimitOrder(TST, Side::ASK, 300,
      Money::ONE), 36, time_from_string("2020-03-12 16:06:14"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderC, account),
      Beam::Queries::Sequence(46)));
    auto query = AccountQuery();
    query.SetIndex(account);
    query.SetRange(Range::Historical());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto info = ParameterExpression(0, OrderInfoType());
    auto field = MemberAccessExpression("order_id", NativeDataType<OrderId>(),
      info);
    auto clauses = std::vector<Expression>();
    auto ids = std::vector<OrderId>();
    ids.push_back(12);
    ids.push_back(36);
    std::transform(ids.begin(), ids.end(), std::back_inserter(clauses),
      [&] (auto& id) {
        return MakeEqualsExpression(field, ConstantExpression(NativeValue(id)));
      });
    query.SetFilter(MakeOrExpression(clauses.begin(), clauses.end()));
    auto snapshot = m_clientProtocol->SendRequest<QueryOrderSubmissionsService>(
      query);
    REQUIRE(snapshot.m_snapshot.size() == 2);
    REQUIRE(snapshot.m_snapshot[0]->m_info.m_orderId == 12);
    REQUIRE(snapshot.m_snapshot[1]->m_info.m_orderId == 36);
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_without_permission") {
    auto account = DirectoryEntry::MakeAccount(599, "sephi");
    auto orderA = OrderInfo(OrderFields::BuildLimitOrder(TST, Side::BID, 100,
      Money::ONE), 12, time_from_string("2020-03-12 16:06:12"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderA, account),
      Beam::Queries::Sequence(44)));
    auto order = m_clientProtocol->SendRequest<LoadOrderByIdService>(12);
    REQUIRE(!order);
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_with_permission") {
    auto account = m_clientServiceLocatorClient->GetAccount();
    auto orderA = OrderInfo(OrderFields::BuildLimitOrder(TST, Side::BID, 100,
      Money::ONE), 12, time_from_string("2020-03-12 16:06:12"));
    m_dataStore->Store(SequencedValue(IndexedValue(orderA, account),
      Beam::Queries::Sequence(44)));
    auto order = m_clientProtocol->SendRequest<LoadOrderByIdService>(12);
    REQUIRE(order.has_value());
    REQUIRE((*order)->GetValue().m_info == orderA);
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_with_updates") {
    auto reportAsync = Async<ExecutionReport>();
    AddMessageSlot<OrderUpdateMessage>(Store(m_clientProtocol->GetSlots()),
      [&] (auto& client, auto& report) {
        reportAsync.GetEval().SetResult(report);
      });
    auto orderFields = OrderFields::BuildLimitOrder(TST,
      DefaultCurrencies::USD(), Side::BID, "TEST", 100, Money::CENT);
    auto newOrder = m_clientProtocol->SendRequest<NewOrderSingleService>(
      orderFields);
    auto driverMonitor = std::make_shared<Queue<PrimitiveOrder*>>();
    m_driver->GetPublisher().Monitor(driverMonitor);
    auto receivedOrder = driverMonitor->Pop();
    auto initialReport = reportAsync.Get();
    REQUIRE(initialReport.m_status == OrderStatus::PENDING_NEW);
    m_dataStore->SetMode(TestOrderExecutionDataStore::Mode::SUPERVISED);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestOrderExecutionDataStore::Operation>>>();
    m_dataStore->GetPublisher().Monitor(operations);
    auto orderId = (*newOrder)->m_orderId;
    auto loadResult = Async<optional<SequencedAccountOrderRecord>>();
    Spawn([&] {
      return m_clientProtocol->SendRequest<LoadOrderByIdService>(orderId);
    }, loadResult.GetEval());
    auto loadOperation = std::dynamic_pointer_cast<
      TestOrderExecutionDataStore::LoadOrderOperation>(operations->Pop());
    REQUIRE(loadOperation);
    REQUIRE(*loadOperation->m_id == orderId);
    auto account = m_clientServiceLocatorClient->GetAccount();
    auto initialLoadResult = SequencedValue(IndexedValue(OrderRecord(OrderInfo(
      orderFields, account, orderId, false,
      time_from_string("2016-11-30 08:11:53")), std::vector{initialReport}),
      account), Beam::Queries::Sequence(417));
    loadOperation->m_result.SetResult(initialLoadResult);
    auto reloadOperation = std::dynamic_pointer_cast<
      TestOrderExecutionDataStore::LoadOrderOperation>(operations->Pop());
    REQUIRE(reloadOperation);
    REQUIRE(*reloadOperation->m_id == orderId);
    auto newReport = ExecutionReport::BuildUpdatedReport(initialReport,
      OrderStatus::NEW, time_from_string("2016-11-30 08:11:54"));
    receivedOrder->Update(newReport);
    auto storeOperation = std::dynamic_pointer_cast<
      TestOrderExecutionDataStore::StoreExecutionReportOperation>(
        operations->Pop());
    REQUIRE(storeOperation);
    REQUIRE((**storeOperation->m_executionReport)->m_status ==
      OrderStatus::NEW);
    storeOperation->m_result.SetResult();
    reloadOperation->m_result.SetResult(initialLoadResult);
    auto receivedRecord = loadResult.Get();
    REQUIRE((**receivedRecord)->m_executionReports.size() == 2);
    m_dataStore->SetMode(TestOrderExecutionDataStore::Mode::UNSUPERVISED);
  }
}
