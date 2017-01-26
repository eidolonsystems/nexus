#ifndef NEXUS_VIRTUALSERVICECLIENTS_HPP
#define NEXUS_VIRTUALSERVICECLIENTS_HPP
#include <Beam/RegistryService/VirtualRegistryClient.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <Beam/Utilities/BeamWorkaround.hpp>
#include <Beam/Utilities/Remote.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/ChartingService/VirtualChartingClient.hpp"
#include "Nexus/Compliance/VirtualComplianceClient.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"

namespace Nexus {

  /*! \class VirtualServiceClients
      \brief Provides a pure virtual interface to the set of ServiceClients.
   */
  class VirtualServiceClients : private boost::noncopyable {
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

      virtual ~VirtualServiceClients() = default;

      virtual ServiceLocatorClient& GetServiceLocatorClient() = 0;

      virtual RegistryClient& GetRegistryClient() = 0;

      virtual AdministrationClient& GetAdministrationClient() = 0;

      virtual DefinitionsClient& GetDefinitionsClient() = 0;

      virtual MarketDataClient& GetMarketDataClient() = 0;

      virtual ChartingClient& GetChartingClient() = 0;

      virtual ComplianceClient& GetComplianceClient() = 0;

      virtual OrderExecutionClient& GetOrderExecutionClient() = 0;

      virtual RiskClient& GetRiskClient() = 0;

      virtual TimeClient& GetTimeClient() = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualServiceClients.
      VirtualServiceClients() = default;
  };

  /*! \class WrapperServiceClients
      \brief Wraps ServiceClients providing it with a virtual interface.
      \tparam ClientType The type of ServiceClients to wrap.
   */
  template<typename ClientType>
  class WrapperServiceClients : public VirtualServiceClients {
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

      //! The ServiceClients to wrap.
      using Client = Beam::GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperServiceClients.
      /*!
        \param client The ServiceClients to wrap.
      */
      template<typename ServiceClientsForward>
      WrapperServiceClients(ServiceClientsForward&& client);

      virtual ~WrapperServiceClients() override = default;

      virtual ServiceLocatorClient& GetServiceLocatorClient() override;

      virtual RegistryClient& GetRegistryClient() override;

      virtual AdministrationClient& GetAdministrationClient() override;

      virtual DefinitionsClient& GetDefinitionsClient() override;

      virtual MarketDataClient& GetMarketDataClient() override;

      virtual ChartingClient& GetChartingClient() override;

      virtual ComplianceClient& GetComplianceClient() override;

      virtual OrderExecutionClient& GetOrderExecutionClient() override;

      virtual RiskClient& GetRiskClient() override;

      virtual TimeClient& GetTimeClient() override;

      virtual void Open() override;

      virtual void Close() override;

    private:
      Beam::GetOptionalLocalPtr<ClientType> m_client;
      Beam::Remote<std::unique_ptr<ServiceLocatorClient>,
        Beam::Threading::Mutex> m_serviceLocatorClient;
      Beam::Remote<std::unique_ptr<RegistryClient>, Beam::Threading::Mutex>
        m_registryClient;
      Beam::Remote<std::unique_ptr<AdministrationClient>,
        Beam::Threading::Mutex> m_administrationClient;
      Beam::Remote<std::unique_ptr<DefinitionsClient>, Beam::Threading::Mutex>
        m_definitionsClient;
      Beam::Remote<std::unique_ptr<MarketDataClient>, Beam::Threading::Mutex>
        m_marketDataClient;
      Beam::Remote<std::unique_ptr<ChartingClient>, Beam::Threading::Mutex>
        m_chartingClient;
      Beam::Remote<std::unique_ptr<ComplianceClient>, Beam::Threading::Mutex>
        m_complianceClient;
      Beam::Remote<std::unique_ptr<OrderExecutionClient>,
        Beam::Threading::Mutex> m_orderExecutionClient;
      Beam::Remote<std::unique_ptr<RiskClient>, Beam::Threading::Mutex>
        m_riskClient;
      Beam::Remote<std::unique_ptr<TimeClient>, Beam::Threading::Mutex>
        m_timeClient;
  };

  //! Wraps ServiceClients into a VirtualServiceClients.
  /*!
    \param client The client to wrap.
  */
  template<typename ServiceClients>
  std::unique_ptr<VirtualServiceClients> MakeVirtualServiceClients(
      ServiceClients&& client) {
    return std::make_unique<WrapperServiceClients<ServiceClients>>(
      std::forward<ServiceClients>(client));
  }

  template<typename ClientType>
  template<typename ServiceClientsForward>
  WrapperServiceClients<ClientType>::WrapperServiceClients(
      ServiceClientsForward&& client)
      BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_client{std::forward<ServiceClientsForward>(client)},
        m_serviceLocatorClient{
          [=] (Beam::DelayPtr<std::unique_ptr<ServiceLocatorClient>>& client) {
            client.Initialize(
              Beam::ServiceLocator::MakeVirtualServiceLocatorClient(
              &m_client->GetServiceLocatorClient()));
          }
        },
        m_registryClient{
          [=] (Beam::DelayPtr<std::unique_ptr<RegistryClient>>& client) {
            client.Initialize(Beam::RegistryService::MakeVirtualRegistryClient(
              &m_client->GetRegistryClient()));
          }
        },
        m_administrationClient{
          [=] (Beam::DelayPtr<std::unique_ptr<AdministrationClient>>& client) {
            client.Initialize(
              AdministrationService::MakeVirtualAdministrationClient(
              &m_client->GetAdministrationClient()));
          }
        },
        m_definitionsClient{
          [=] (Beam::DelayPtr<std::unique_ptr<DefinitionsClient>>& client) {
            client.Initialize(DefinitionsService::MakeVirtualDefinitionsClient(
              &m_client->GetDefinitionsClient()));
          }
        },
        m_marketDataClient{
          [=] (Beam::DelayPtr<std::unique_ptr<MarketDataClient>>& client) {
            client.Initialize(MarketDataService::MakeVirtualMarketDataClient(
              &m_client->GetMarketDataClient()));
          }
        },
        m_chartingClient{
          [=] (Beam::DelayPtr<std::unique_ptr<ChartingClient>>& client) {
            client.Initialize(ChartingService::MakeVirtualChartingClient(
              &m_client->GetChartingClient()));
          }
        },
        m_complianceClient{
          [=] (Beam::DelayPtr<std::unique_ptr<ComplianceClient>>& client) {
            client.Initialize(Compliance::MakeVirtualComplianceClient(
              &m_client->GetComplianceClient()));
          }
        },
        m_orderExecutionClient{
          [=] (Beam::DelayPtr<std::unique_ptr<OrderExecutionClient>>& client) {
            client.Initialize(
              OrderExecutionService::MakeVirtualOrderExecutionClient(
              &m_client->GetOrderExecutionClient()));
          }
        },
        m_riskClient{
          [=] (Beam::DelayPtr<std::unique_ptr<RiskClient>>& client) {
            client.Initialize(RiskService::MakeVirtualRiskClient(
              &m_client->GetRiskClient()));
          }
        },
        m_timeClient{
          [=] (Beam::DelayPtr<std::unique_ptr<TimeClient>>& client) {
            client.Initialize(Beam::TimeService::MakeVirtualTimeClient(
              &m_client->GetTimeClient()));
          }
        } {}
      BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::ServiceLocatorClient&
      WrapperServiceClients<ClientType>::GetServiceLocatorClient() {
    return **m_serviceLocatorClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::RegistryClient&
      WrapperServiceClients<ClientType>::GetRegistryClient() {
    return **m_registryClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::AdministrationClient&
      WrapperServiceClients<ClientType>::GetAdministrationClient() {
    return **m_administrationClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::DefinitionsClient&
      WrapperServiceClients<ClientType>::GetDefinitionsClient() {
    return **m_definitionsClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::MarketDataClient&
      WrapperServiceClients<ClientType>::GetMarketDataClient() {
    return **m_marketDataClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::ChartingClient&
      WrapperServiceClients<ClientType>::GetChartingClient() {
    return **m_chartingClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::ComplianceClient&
      WrapperServiceClients<ClientType>::GetComplianceClient() {
    return **m_complianceClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::OrderExecutionClient&
      WrapperServiceClients<ClientType>::GetOrderExecutionClient() {
    return **m_orderExecutionClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::RiskClient&
      WrapperServiceClients<ClientType>::GetRiskClient() {
    return **m_riskClient;
  }

  template<typename ClientType>
  typename WrapperServiceClients<ClientType>::TimeClient&
      WrapperServiceClients<ClientType>::GetTimeClient() {
    return **m_timeClient;
  }

  template<typename ClientType>
  void WrapperServiceClients<ClientType>::Open() {
    m_client->Open();
  }

  template<typename ClientType>
  void WrapperServiceClients<ClientType>::Close() {
    m_client->Close();
  }
}

#endif
