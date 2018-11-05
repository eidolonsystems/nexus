#include "spire/toolbar/toolbar_controller.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "spire/security_input/services_security_input_model.hpp"
#include "spire/time_and_sales/time_and_sales_controller.hpp"
#include "spire/toolbar/toolbar_window.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

struct ToolbarController::BaseController {
  using closed_signal = Signal<void (const BaseController& controller)>;

  virtual ~BaseController() = default;
  virtual void open() = 0;
  virtual connection connect_closed_signal(
    const closed_signal::slot_type& slot) const = 0;
};

template<typename T>
struct ToolbarController::Controller final : ToolbarController::BaseController {
  using Type = T;
  Type m_controller;

  template<typename... Args>
  Controller(Args&&... args)
      : m_controller(std::forward<Args>(args)...) {}

  void open() override {
    m_controller.open();
  }

  connection connect_closed_signal(
      const closed_signal::slot_type& slot) const override {
    return m_controller.connect_closed_signal([=] { slot(*this); });
  }
};

ToolbarController::ToolbarController(Ref<VirtualServiceClients> service_clients)
    : m_service_clients(service_clients.Get()),
      m_security_input_model(std::make_unique<ServicesSecurityInputModel>(
        Ref(m_service_clients->GetMarketDataClient()))) {}

ToolbarController::~ToolbarController() {
  close();
}

void ToolbarController::open() {
  if(m_toolbar_window != nullptr) {
    return;
  }
  m_toolbar_window = std::make_unique<ToolbarWindow>(m_model,
    m_service_clients->GetServiceLocatorClient().GetAccount());
  m_toolbar_window->connect_open_signal(
    [=] (auto window) { on_open_window(window); });
  m_toolbar_window->connect_closed_signal([=] { on_closed(); });
  m_toolbar_window->show();
}

void ToolbarController::close() {
  if(m_toolbar_window == nullptr) {
    return;
  }
  auto controllers = std::move(m_controllers);
  controllers.clear();
  m_toolbar_window.reset();
  m_closed_signal();
}

connection ToolbarController::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

void ToolbarController::on_open_window(RecentlyClosedModel::Type window) {
  auto controller =
    [&] () -> std::unique_ptr<BaseController> {
      if(window == RecentlyClosedModel::Type::TIME_AND_SALE) {
        return std::make_unique<Controller<TimeAndSalesController>>(
          Ref(*m_security_input_model), Ref(*m_service_clients));
      }
      return nullptr;
    }();
  if(controller == nullptr) {
    return;
  }
  controller->connect_closed_signal(
    [=] (const auto& controller) { on_controller_closed(controller); });
  controller->open();
  m_controllers.push_back(std::move(controller));
}

void ToolbarController::on_closed() {
  close();
}

void ToolbarController::on_controller_closed(const BaseController& controller) {
  auto i = std::find_if(m_controllers.begin(), m_controllers.end(),
    [&] (const auto& c) {
      return c.get() == &controller;
    });
  if(i == m_controllers.end()) {
    return;
  }
  m_controllers.erase(i);
}
