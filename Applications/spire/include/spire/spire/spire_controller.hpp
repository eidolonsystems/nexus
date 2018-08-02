#ifndef SPIRE_CONTROLLER_HPP
#define SPIRE_CONTROLLER_HPP
#include <memory>
#include <optional>
#include <Beam/Network/Network.hpp>
#include <Beam/Threading/Threading.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/login/login.hpp"
#include "spire/spire/spire.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! Controller for the application as a whole.
  class SpireController : private boost::noncopyable {
    public:

      //! Constructs a controller in a state ready to execute a new instance of
      //! the application.
      SpireController();

      ~SpireController();

      //! Begins running the application.
      void open();

    private:
      enum class State {
        NONE,
        LOGIN,
        TOOLBAR
      };
      State m_state;
      std::unique_ptr<Beam::Network::SocketThreadPool> m_socket_thread_pool;
      std::unique_ptr<Beam::Threading::TimerThreadPool> m_timer_thread_pool;
      std::unique_ptr<LoginController> m_login_controller;
      std::unique_ptr<Nexus::VirtualServiceClients> m_service_clients;
      std::unique_ptr<ToolbarController> m_toolbar_controller;

      std::optional<Beam::Network::IpAddress> load_ip_address();
      void on_login();
  };
}

#endif
