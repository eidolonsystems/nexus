#ifndef NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/TcpSocketChannel.hpp>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp>
#include <Beam/Threading/LiveTimer.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceClient.hpp"

namespace Nexus {
namespace Compliance {
namespace Details {
  using ComplianceClientSessionBuilder =
    Beam::Services::AuthenticatedServiceProtocolClientBuilder<
    Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
    Beam::Services::MessageProtocol<
    std::unique_ptr<Beam::Network::TcpSocketChannel>,
    Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
    Beam::Codecs::NullEncoder>, Beam::Threading::LiveTimer>;
}

  /*! \class ApplicationComplianceClient
      \brief Encapsulates a standard ComplianceClient used in an application.
   */
  class ApplicationComplianceClient : private boost::noncopyable {
    public:

      //! Defines the standard ComplianceClient used for applications.
      using Client = ComplianceClient<Details::ComplianceClientSessionBuilder>;

      //! Constructs an ApplicationComplianceClient.
      ApplicationComplianceClient() = default;

      //! Builds the session.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate sessions.
        \param socketThreadPool The SocketThreadPool used for the socket
               connection.
        \param timerThreadPool The TimerThreadPool used for heartbeats.
      */
      void BuildSession(Beam::Ref<
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
        serviceLocatorClient, Beam::Ref<Beam::Network::SocketThreadPool>
        socketThreadPool, Beam::Ref<Beam::Threading::TimerThreadPool>
        timerThreadPool);

      //! Returns a reference to the Client.
      Client& operator *();

      //! Returns a reference to the Client.
      const Client& operator *() const;

      //! Returns a pointer to the Client.
      Client* operator ->();

      //! Returns a pointer to the Client.
      const Client* operator ->() const;

      //! Returns a pointer to the Client.
      Client* Get();

      //! Returns a pointer to the Client.
      const Client* Get() const;

    private:
      Beam::DelayPtr<Client> m_client;
  };

  inline void ApplicationComplianceClient::BuildSession(Beam::Ref<
      Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient, Beam::Ref<Beam::Network::SocketThreadPool>
      socketThreadPool, Beam::Ref<Beam::Threading::TimerThreadPool>
      timerThreadPool) {
    if(m_client.IsInitialized()) {
      m_client->Close();
      m_client.Reset();
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto socketThreadPoolHandle = socketThreadPool.Get();
    auto timerThreadPoolHandle = timerThreadPool.Get();
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, Compliance::SERVICE_NAME);
    auto delay = false;
    auto sessionBuilder = Details::ComplianceClientSessionBuilder(
      Beam::Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          auto delayTimer = Beam::Threading::LiveTimer(
            boost::posix_time::seconds(3), Beam::Ref(*timerThreadPoolHandle));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<Beam::Network::TcpSocketChannel>(addresses,
          Beam::Ref(*socketThreadPoolHandle));
      },
      [=] {
        return std::make_unique<Beam::Threading::LiveTimer>(
          boost::posix_time::seconds(10), Beam::Ref(*timerThreadPoolHandle));
      });
    m_client.Initialize(sessionBuilder);
  }

  inline ApplicationComplianceClient::Client&
      ApplicationComplianceClient::operator *() {
    return m_client.Get();
  }

  inline const ApplicationComplianceClient::Client&
      ApplicationComplianceClient::operator *() const {
    return m_client.Get();
  }

  inline ApplicationComplianceClient::Client*
      ApplicationComplianceClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationComplianceClient::Client*
      ApplicationComplianceClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationComplianceClient::Client*
      ApplicationComplianceClient::Get() {
    return &*m_client;
  }

  inline const ApplicationComplianceClient::Client*
      ApplicationComplianceClient::Get() const {
    return &*m_client;
  }
}
}

#endif
