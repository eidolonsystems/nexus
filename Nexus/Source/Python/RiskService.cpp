#include "Nexus/Python/RiskService.hpp"
#include <Beam/Python/Beam.hpp>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/Python/RiskClient.hpp"
#include "Nexus/Python/RiskDataStore.hpp"
#include "Nexus/RiskService/ApplicationDefinitions.hpp"
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Nexus/RiskService/SqlRiskDataStore.hpp"
#include "Nexus/RiskService/VirtualRiskClient.hpp"
#include "Nexus/RiskService/VirtualRiskDataStore.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Parsers;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  struct TrampolineRiskClient final : VirtualRiskClient {
    const RiskPortfolioUpdatePublisher&
        GetRiskPortfolioUpdatePublisher() override {
      PYBIND11_OVERLOAD_PURE_NAME(const RiskPortfolioUpdatePublisher&,
        VirtualRiskClient, "get_risk_portfolio_update_publisher",
        GetRiskPortfolioUpdatePublisher);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskClient, "open", Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskClient, "close", Close);
    }
  };

  struct TrampolineRiskDataStore final : VirtualRiskDataStore {
    PositionSnapshot LoadPositionSnapshot(
        const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(PositionSnapshot, VirtualRiskDataStore,
        "load_position_snapshot", LoadPositionSnapshot, account);
    }

    void Store(const DirectoryEntry& account,
        const PositionSnapshot& snapshot) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskDataStore, "store",
        Store, account, snapshot);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskDataStore, "open", Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualRiskDataStore, "close", Close);
    }
  };
}

void Nexus::Python::ExportApplicationRiskClient(pybind11::module& module) {
  using SessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    VirtualServiceLocatorClient, MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>, NullEncoder>,
    LiveTimer>;
  using Client = RiskClient<SessionBuilder>;
  class_<ToPythonRiskClient<Client>, VirtualRiskClient>(module,
      "ApplicationRiskClient")
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient) {
        auto addresses = LocateServiceAddresses(serviceLocatorClient,
          RiskService::SERVICE_NAME);
        auto delay = false;
        auto sessionBuilder = SessionBuilder(Ref(serviceLocatorClient),
          [=] () mutable {
            if(delay) {
              auto delayTimer = LiveTimer(seconds(3),
                Ref(*GetTimerThreadPool()));
              delayTimer.Start();
              delayTimer.Wait();
            }
            delay = true;
            return std::make_unique<TcpSocketChannel>(addresses,
              Ref(*GetSocketThreadPool()));
          },
          [=] {
            return std::make_unique<LiveTimer>(seconds(10),
              Ref(*GetTimerThreadPool()));
          });
        return MakeToPythonRiskClient(std::make_unique<Client>(sessionBuilder));
      }));
}

void Nexus::Python::ExportLocalRiskDataStore(pybind11::module& module) {
  class_<ToPythonRiskDataStore<LocalRiskDataStore>, VirtualRiskDataStore,
      std::shared_ptr<ToPythonRiskDataStore<LocalRiskDataStore>>>(module,
      "LocalRiskDataStore")
    .def(init(
      [] () {
        return MakeToPythonRiskDataStore(
          std::make_unique<LocalRiskDataStore>());
      }));
}

void Nexus::Python::ExportMySqlRiskDataStore(pybind11::module& module) {
  class_<ToPythonRiskDataStore<SqlRiskDataStore<Viper::MySql::Connection>>,
      VirtualRiskDataStore, std::shared_ptr<
      ToPythonRiskDataStore<SqlRiskDataStore<Viper::MySql::Connection>>>>(
      module, "MySqlRiskDataStore")
    .def(init(
      [] (std::string host, unsigned int port, std::string username,
          std::string password, std::string database) {
        return MakeToPythonRiskDataStore(
          std::make_unique<SqlRiskDataStore<Viper::MySql::Connection>>(
          std::make_unique<Viper::MySql::Connection>(host, port, username,
          password, database)));
      }));
}

void Nexus::Python::ExportRiskClient(pybind11::module& module) {
  class_<VirtualRiskClient, TrampolineRiskClient>(module, "RiskClient")
    .def("get_risk_portfolio_update_publisher",
      &VirtualRiskClient::GetRiskPortfolioUpdatePublisher)
    .def("open", &VirtualRiskClient::Open)
    .def("close", &VirtualRiskClient::Close);
}

void Nexus::Python::ExportRiskDataStore(pybind11::module& module) {
  class_<VirtualRiskDataStore, TrampolineRiskDataStore,
      std::shared_ptr<VirtualRiskDataStore>>(module, "RiskDataStore")
    .def("load_position_snapshot", &VirtualRiskDataStore::LoadPositionSnapshot)
    .def("store", &VirtualRiskDataStore::Store)
    .def("open", &VirtualRiskDataStore::Open)
    .def("close", &VirtualRiskDataStore::Close);
}

void Nexus::Python::ExportRiskService(pybind11::module& module) {
  auto submodule = module.def_submodule("risk_service");
  ExportRiskClient(submodule);
  ExportApplicationRiskClient(submodule);
  ExportRiskDataStore(submodule);
  ExportLocalRiskDataStore(submodule);
  ExportMySqlRiskDataStore(submodule);
  ExportRiskParameters(submodule);
  ExportRiskState(submodule);
  ExportSqliteRiskDataStore(submodule);
}

void Nexus::Python::ExportRiskParameters(pybind11::module& module) {
  class_<RiskParameters>(module, "RiskParameters")
    .def(init())
    .def(init<const RiskParameters&>())
    .def_readwrite("currency", &RiskParameters::m_currency)
    .def_readwrite("buying_power", &RiskParameters::m_buyingPower)
    .def_readwrite("allowed_state", &RiskParameters::m_allowedState)
    .def_readwrite("net_loss", &RiskParameters::m_netLoss)
    .def_readwrite("loss_from_top", &RiskParameters::m_lossFromTop)
    .def_readwrite("transition_time", &RiskParameters::m_transitionTime)
    .def(self == self)
    .def(self != self);
}

void Nexus::Python::ExportRiskState(pybind11::module& module) {
  auto outer = class_<RiskState>(module, "RiskState")
    .def(init())
    .def(init<RiskState::Type>())
    .def(init<RiskState::Type, ptime>())
    .def(init<const RiskState&>())
    .def_readwrite("type", &RiskState::m_type)
    .def_readwrite("expiry", &RiskState::m_expiry)
    .def(self == self)
    .def(self != self);
  enum_<RiskState::Type::Type>(outer, "Type")
    .value("NONE", RiskState::Type::NONE)
    .value("ACTIVE", RiskState::Type::ACTIVE)
    .value("CLOSE_ORDERS", RiskState::Type::CLOSE_ORDERS)
    .value("DISABLED", RiskState::Type::DISABLED);
}

void Nexus::Python::ExportSqliteRiskDataStore(pybind11::module& module) {
  class_<ToPythonRiskDataStore<SqlRiskDataStore<Viper::Sqlite3::Connection>>,
      VirtualRiskDataStore, std::shared_ptr<
      ToPythonRiskDataStore<SqlRiskDataStore<Viper::Sqlite3::Connection>>>>(
      module, "SqliteRiskDataStore")
    .def(init(
      [] (std::string path) {
        return MakeToPythonRiskDataStore(
          std::make_unique<SqlRiskDataStore<Viper::Sqlite3::Connection>>(
          std::make_unique<Viper::Sqlite3::Connection>(path)));
      }));
}
