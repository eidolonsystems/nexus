#ifndef NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {

  /** Encapsulates a standard DefinitionsClient used in an application. */
  class ApplicationDefinitionsClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder =
        Beam::Services::AuthenticatedServiceProtocolClientBuilder<
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
        Beam::Services::MessageProtocol<std::unique_ptr<
        Beam::Network::TcpSocketChannel>, Beam::Serialization::BinarySender<
        Beam::IO::SharedBuffer>, Beam::Codecs::NullEncoder>,
        Beam::Threading::LiveTimer>;

      /** Defines the standard DefinitionsClient used for applications. */
      using Client = DefinitionsClient<SessionBuilder>;

      /** Constructs an ApplicationDefinitionsClient. */
      ApplicationDefinitionsClient() = default;

      /**
       * Builds the session.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      void BuildSession(
        Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
        serviceLocatorClient);

      /** Returns a reference to the Client. */
      Client& operator *();

      /** Returns a reference to the Client. */
      const Client& operator *() const;

      /** Returns a pointer to the Client. */
      Client* operator ->();

      /** Returns a pointer to the Client. */
      const Client* operator ->() const;

      /** Returns a pointer to the Client. */
      Client* Get();

      /** Returns a pointer to the Client. */
      const Client* Get() const;

    private:
      boost::optional<Client> m_client;

      ApplicationDefinitionsClient(
        const ApplicationDefinitionsClient&) = delete;
      ApplicationDefinitionsClient& operator =(
        const ApplicationDefinitionsClient&) = delete;
  };

  inline void ApplicationDefinitionsClient::BuildSession(
      Beam::Ref<Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    m_client = boost::none;
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClient, SERVICE_NAME);
    auto sessionBuilder = SessionBuilder(Beam::Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(std::move(sessionBuilder));
  }

  inline ApplicationDefinitionsClient::Client&
      ApplicationDefinitionsClient::operator *() {
    return *m_client;
  }

  inline const ApplicationDefinitionsClient::Client&
      ApplicationDefinitionsClient::operator *() const {
    return *m_client;
  }

  inline ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::Get() {
    return &*m_client;
  }

  inline const ApplicationDefinitionsClient::Client*
      ApplicationDefinitionsClient::Get() const {
    return &*m_client;
  }
}

#endif
