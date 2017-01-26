#ifndef NEXUS_TESTSERVICECLIENTS_HPP
#define NEXUS_TESTSERVICECLIENTS_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/RegistryService/VirtualRegistryClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/ChartingService/VirtualChartingClient.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"
#include "Nexus/ServiceClients/TestServiceClientsInstance.hpp"

namespace Nexus {

  /*! \class TestServiceClients
      \brief Implements the ServiceClients interface for testing.
   */
  class TestServiceClients : private boost::noncopyable {
    public:
      using ServiceLocatorClient =
        Beam::ServiceLocator::VirtualServiceLocatorClient;

      using RegistryClient = Beam::RegistryService::VirtualRegistryClient;

      using AdministrationClient =
        AdministrationService::VirtualAdministrationClient;

      using DefinitionsClient = DefinitionsService::VirtualDefinitionsClient;

      using MarketDataClient = MarketDataService::VirtualMarketDataClient;

      using ChartingClient = ChartingService::VirtualChartingClient;

      using ComplianceClient = Compliance::VirtualComplianceClient;

      using OrderExecutionClient =
        OrderExecutionService::VirtualOrderExecutionClient;

      using RiskClient = RiskService::VirtualRiskClient;

      using TimeClient = Beam::TimeService::VirtualTimeClient;

      //! Constructs a TestServiceClients.
      /*!
        \param instance The TestServiceClientsInstance to connect to.
      */
      TestServiceClients(Beam::RefType<TestServiceClientsInstance> instance);

      ~TestServiceClients();

      ServiceLocatorClient& GetServiceLocatorClient();

      RegistryClient& GetRegistryClient();

      AdministrationClient& GetAdministrationClient();

      DefinitionsClient& GetDefinitionsClient();

      MarketDataClient& GetMarketDataClient();

      ChartingClient& GetChartingClient();

      ComplianceClient& GetComplianceClient();

      OrderExecutionClient& GetOrderExecutionClient();

      RiskClient& GetRiskClient();

      TimeClient& GetTimeClient();

      void Open();

      void Close();

    private:
      TestServiceClientsInstance* m_instance;
      std::unique_ptr<ServiceLocatorClient> m_serviceLocatorClient;
      std::unique_ptr<RegistryClient> m_registryClient;
      std::unique_ptr<AdministrationClient> m_administrationClient;
      std::unique_ptr<DefinitionsClient> m_definitionsClient;
      std::unique_ptr<MarketDataClient> m_marketDataClient;
      std::unique_ptr<ChartingClient> m_chartingClient;
      std::unique_ptr<ComplianceClient> m_complianceClient;
      std::unique_ptr<OrderExecutionClient> m_orderExecutionClient;
      std::unique_ptr<RiskClient> m_riskClient;
      std::unique_ptr<TimeClient> m_timeClient;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  inline TestServiceClients::TestServiceClients(
      Beam::RefType<TestServiceClientsInstance> instance)
      : m_instance{instance.Get()} {}

  inline TestServiceClients::~TestServiceClients() {
    Close();
  }

  inline TestServiceClients::ServiceLocatorClient&
      TestServiceClients::GetServiceLocatorClient() {
    return *m_serviceLocatorClient;
  }

  inline TestServiceClients::RegistryClient&
      TestServiceClients::GetRegistryClient() {
    return *m_registryClient;
  }

  inline TestServiceClients::AdministrationClient&
      TestServiceClients::GetAdministrationClient() {
    return *m_administrationClient;
  }

  inline TestServiceClients::DefinitionsClient&
      TestServiceClients::GetDefinitionsClient() {
    return *m_definitionsClient;
  }

  inline TestServiceClients::MarketDataClient&
      TestServiceClients::GetMarketDataClient() {
    return *m_marketDataClient;
  }

  inline TestServiceClients::ChartingClient&
      TestServiceClients::GetChartingClient() {
    return *m_chartingClient;
  }

  inline TestServiceClients::ComplianceClient&
      TestServiceClients::GetComplianceClient() {
    return *m_complianceClient;
  }

  inline TestServiceClients::OrderExecutionClient&
      TestServiceClients::GetOrderExecutionClient() {
    return *m_orderExecutionClient;
  }

  inline TestServiceClients::RiskClient& TestServiceClients::GetRiskClient() {
    return *m_riskClient;
  }

  inline TestServiceClients::TimeClient& TestServiceClients::GetTimeClient() {
    return *m_timeClient;
  }

  inline void TestServiceClients::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_serviceLocatorClient =
        m_instance->GetServiceLocatorInstance().BuildClient();
      m_serviceLocatorClient->SetCredentials("root", "");
      m_serviceLocatorClient->Open();
      m_administrationClient =
        m_instance->GetAdministrationInstance().BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      m_administrationClient->Open();
      m_marketDataClient = m_instance->GetMarketDataInstance().BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      m_marketDataClient->Open();
      m_orderExecutionClient =
        m_instance->GetOrderExecutionInstance().BuildClient(
        Beam::Ref(*m_serviceLocatorClient));
      m_timeClient = Beam::TimeService::MakeVirtualTimeClient(
        std::make_unique<Beam::TimeService::FixedTimeClient>());
      m_timeClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void TestServiceClients::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void TestServiceClients::Shutdown() {
    if(m_timeClient != nullptr) {
      m_timeClient->Close();
    }
    if(m_riskClient != nullptr) {
      m_riskClient->Close();
    }
    if(m_orderExecutionClient != nullptr) {
      m_orderExecutionClient->Close();
    }
    if(m_complianceClient != nullptr) {
      m_complianceClient->Close();
    }
    if(m_chartingClient != nullptr) {
      m_chartingClient->Close();
    }
    if(m_marketDataClient != nullptr) {
      m_marketDataClient->Close();
    }
    if(m_definitionsClient != nullptr) {
      m_definitionsClient->Close();
    }
    if(m_administrationClient != nullptr) {
      m_administrationClient->Close();
    }
    if(m_registryClient != nullptr) {
      m_registryClient->Close();
    }
    if(m_serviceLocatorClient != nullptr) {
      m_serviceLocatorClient->Close();
    }
    m_openState.SetClosed();
  }
}

#endif
