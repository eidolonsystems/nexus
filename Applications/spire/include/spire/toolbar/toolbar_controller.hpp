#ifndef SPIRE_TOOLBAR_CONTROLLER_HPP
#define SPIRE_TOOLBAR_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/ServiceClients/ServiceClients.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace Spire {

  //! Launches the main application windows.
  class ToolbarController : private boost::noncopyable {
    public:

      //! Signals that this toolbar and all its windows have closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs the toolbar controller.
      /*!
        \param service_clients The service clients logged into Spire.
      */
      ToolbarController(
        Beam::Ref<Nexus::VirtualServiceClients> service_clients);

      ~ToolbarController();

      //! Begins displaying the toolbar window.
      void open();

      //! Closes the toolbar window and all associated windows.
      void close();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      struct BaseController;
      template<typename> struct Controller;
      mutable ClosedSignal m_closed_signal;
      Nexus::VirtualServiceClients* m_service_clients;
      RecentlyClosedModel m_model;
      std::unique_ptr<SecurityInputModel> m_security_input_model;
      std::unique_ptr<ToolbarWindow> m_toolbar_window;
      std::vector<std::unique_ptr<BaseController>> m_controllers;

      void on_open_window(RecentlyClosedModel::Type window);
      void on_closed();
      void on_controller_closed(const BaseController& controller);
  };
}

#endif
