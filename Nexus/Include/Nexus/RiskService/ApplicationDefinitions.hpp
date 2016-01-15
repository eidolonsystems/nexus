#ifndef NEXUS_RISKAPPLICATIONDEFINITIONS_HPP
#define NEXUS_RISKAPPLICATIONDEFINITIONS_HPP
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
#include "Nexus/RiskService/RiskClient.hpp"
#include "Nexus/RiskService/RiskService.hpp"

namespace Nexus {
namespace RiskService {
namespace Details {
  using RiskClientSessionBuilder =
    Beam::Services::AuthenticatedServiceProtocolClientBuilder<
    Beam::ServiceLocator::ApplicationServiceLocatorClient::Client,
    Beam::Services::MessageProtocol<
    std::unique_ptr<Beam::Network::TcpSocketChannel>,
    Beam::Serialization::BinarySender<Beam::IO::SharedBuffer>,
    Beam::Codecs::NullEncoder>, Beam::Threading::LiveTimer>;
}

  /*! \class ApplicationRiskClient
      \brief Encapsulates a standard RiskClient used in an
             application.
   */
  class ApplicationRiskClient : private boost::noncopyable {
    public:

      //! Defines the standard RiskClient used for applications.
      using Client = RiskClient<Details::RiskClientSessionBuilder>;

      //! Constructs an ApplicationRiskClient.
      ApplicationRiskClient() = default;

      //! Builds the session.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate sessions.
        \param socketThreadPool The SocketThreadPool used for the socket
               connection.
        \param timerThreadPool The TimerThreadPool used for heartbeats.
      */
      void BuildSession(Beam::RefType<
        Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
        serviceLocatorClient, Beam::RefType<Beam::Network::SocketThreadPool>
        socketThreadPool, Beam::RefType<Beam::Threading::TimerThreadPool>
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

  inline void ApplicationRiskClient::BuildSession(Beam::RefType<
      Beam::ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient, Beam::RefType<Beam::Network::SocketThreadPool>
      socketThreadPool, Beam::RefType<Beam::Threading::TimerThreadPool>
      timerThreadPool) {
    if(m_client.IsInitialized()) {
      m_client->Close();
      m_client.Reset();
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto socketThreadPoolHandle = socketThreadPool.Get();
    auto timerThreadPoolHandle = timerThreadPool.Get();
    auto addresses = Beam::ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, SERVICE_NAME);
    auto delay = false;
    Details::RiskClientSessionBuilder sessionBuilder(
      Beam::Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          Beam::Threading::LiveTimer delayTimer(boost::posix_time::seconds(3),
            Beam::Ref(*timerThreadPoolHandle));
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

  inline ApplicationRiskClient::Client& ApplicationRiskClient::operator *() {
    return m_client.Get();
  }

  inline const ApplicationRiskClient::Client&
      ApplicationRiskClient::operator *() const {
    return m_client.Get();
  }

  inline ApplicationRiskClient::Client* ApplicationRiskClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationRiskClient::Client*
      ApplicationRiskClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationRiskClient::Client* ApplicationRiskClient::Get() {
    return &*m_client;
  }

  inline const ApplicationRiskClient::Client*
      ApplicationRiskClient::Get() const {
    return &*m_client;
  }
}
}

#endif
