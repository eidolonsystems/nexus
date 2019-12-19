#include "Nexus/Python/Backtester.hpp"
#include <Beam/Python/Beam.hpp>
#include "Nexus/Backtester/BacktesterEnvironment.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"
#include "Nexus/Backtester/BacktesterServiceClients.hpp"
#include "Nexus/Python/ServiceClients.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::Python;
using namespace pybind11;

namespace {
  struct TrampolineBacktesterEnvironment final : BacktesterEnvironment {
    std::shared_ptr<VirtualServiceClients> m_serviceClients;

    TrampolineBacktesterEnvironment(ptime startTime,
      std::shared_ptr<VirtualServiceClients> serviceClients)
      : BacktesterEnvironment(startTime, Ref(*serviceClients)),
        m_serviceClients(std::move(serviceClients)) {}

    TrampolineBacktesterEnvironment(ptime startTime, ptime endTime,
      std::shared_ptr<VirtualServiceClients> serviceClients)
      : BacktesterEnvironment{startTime, endTime, Ref(*serviceClients)},
        m_serviceClients(std::move(serviceClients)) {}
  };
}

void Nexus::Python::ExportBacktester(pybind11::module& module) {
  ExportBacktesterEnvironment(module);
  ExportBacktesterEventHandler(module);
  ExportBacktesterServiceClients(module);
}

void Nexus::Python::ExportBacktesterEnvironment(pybind11::module& module) {
  class_<BacktesterEnvironment, std::shared_ptr<BacktesterEnvironment>>(module,
      "BacktesterEnvironment")
    .def(init(
      [] (ptime startTime,
          std::shared_ptr<VirtualServiceClients> serviceClients) {
        return std::make_unique<TrampolineBacktesterEnvironment>(startTime,
          std::move(serviceClients));
      }))
    .def(init(
      [] (ptime startTime, ptime endTime,
          std::shared_ptr<VirtualServiceClients> serviceClients) {
        return std::make_unique<TrampolineBacktesterEnvironment>(startTime,
          endTime, std::move(serviceClients));
      }))
    .def_property_readonly("event_handler",
      static_cast<BacktesterEventHandler& (BacktesterEnvironment::*)()>(
      &BacktesterEnvironment::GetEventHandler),
      return_value_policy::reference_internal)
    .def_property_readonly("market_data_server",
      static_cast<BacktesterMarketDataService& (BacktesterEnvironment::*)()>(
      &BacktesterEnvironment::GetMarketDataService),
      return_value_policy::reference_internal)
    .def("open", &BacktesterEnvironment::Open, call_guard<GilRelease>())
    .def("close", &BacktesterEnvironment::Close,
      call_guard<GilRelease>());
}

void Nexus::Python::ExportBacktesterEventHandler(pybind11::module& module) {
  class_<BacktesterEventHandler>(module, "BacktesterEventHandler")
    .def(init<ptime>())
    .def(init<ptime, ptime>())
    .def_property_readonly("start_time", &BacktesterEventHandler::GetStartTime)
    .def_property_readonly("end_time", &BacktesterEventHandler::GetEndTime)
    .def("add", static_cast<void (BacktesterEventHandler::*)(
      std::shared_ptr<BacktesterEvent>)>(&BacktesterEventHandler::Add))
    .def("add",
      [] (BacktesterEventHandler& self, const object& events) {
        auto e = std::vector<std::shared_ptr<BacktesterEvent>>();
        for(auto& event : events) {
          e.push_back(event.cast<std::shared_ptr<BacktesterEvent>>());
        }
        self.Add(std::move(e));
      })
    .def("open", &BacktesterEventHandler::Open,
      call_guard<GilRelease>())
    .def("close", &BacktesterEventHandler::Close,
      call_guard<GilRelease>());
}

void Nexus::Python::ExportBacktesterServiceClients(pybind11::module& module) {
  class_<ToPythonServiceClients<BacktesterServiceClients>,
      std::shared_ptr<ToPythonServiceClients<BacktesterServiceClients>>,
      VirtualServiceClients>(module, "BacktesterServiceClients")
    .def(init(
      [] (std::shared_ptr<BacktesterEnvironment> environment) {
        return MakeToPythonServiceClients(
          std::make_unique<BacktesterServiceClients>(Ref(*environment)));
      }));
}
