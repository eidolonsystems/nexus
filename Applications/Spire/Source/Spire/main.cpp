#include <filesystem>
#include <fstream>
#include <Beam/Network/SocketThreadPool.hpp>
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/DefinitionsService/VirtualDefinitionsClient.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Dashboard/SavedDashboards.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorProperties.hpp"
#include "Spire/KeyBindings/HotkeyOverride.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerProperties.hpp"
#include "Spire/RiskTimer/RiskTimerMonitor.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/LoginDialog.hpp"
#include "Spire/UI/Toolbar.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Spire/Version.hpp"
#include <QtPlugin>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

inline void InitializeResources() {
  Q_INIT_RESOURCE(Resources);
}

namespace {
  void LoadDefaultLayout(vector<QWidget*>& windows, UserProfile& userProfile) {
    auto instantiateSecurityWindows = true;
    QPoint nextPosition(0, 0);
    auto nextHeight = 0;
    auto resolution = QApplication::desktop()->availableGeometry();
    vector<Security> defaultSecurities;
    auto& marketEntry = userProfile.GetMarketDatabase().FromCode("XTSE");
    defaultSecurities.push_back(Security("RY", marketEntry.m_code,
      marketEntry.m_countryCode));
    defaultSecurities.push_back(Security("XIU", marketEntry.m_code,
      marketEntry.m_countryCode));
    defaultSecurities.push_back(Security("ABX", marketEntry.m_code,
      marketEntry.m_countryCode));
    defaultSecurities.push_back(Security("SU", marketEntry.m_code,
      marketEntry.m_countryCode));
    defaultSecurities.push_back(Security("COS", marketEntry.m_code,
      marketEntry.m_countryCode));
    auto index = std::size_t{0};
    while(instantiateSecurityWindows && index < defaultSecurities.size()) {
      auto width = 0;
      auto bookViewWindow = new BookViewWindow(Ref(userProfile),
        userProfile.GetDefaultBookViewProperties(), "", nullptr, 0);
      auto timeAndSalesWindow = new TimeAndSalesWindow(Ref(userProfile),
        userProfile.GetDefaultTimeAndSalesProperties(), "", nullptr, 0);
      bookViewWindow->Link(*timeAndSalesWindow);
      timeAndSalesWindow->Link(*bookViewWindow);
      bookViewWindow->move(nextPosition);
      bookViewWindow->show();
      nextPosition.rx() += bookViewWindow->frameSize().width();
      width += bookViewWindow->frameSize().width();
      nextHeight = bookViewWindow->frameSize().height();
      timeAndSalesWindow->resize(150, bookViewWindow->height());
      timeAndSalesWindow->move(nextPosition);
      timeAndSalesWindow->show();
      bookViewWindow->DisplaySecurity(defaultSecurities[index]);
      nextPosition.rx() += timeAndSalesWindow->frameSize().width();
      width += timeAndSalesWindow->frameSize().width();
      windows.push_back(bookViewWindow);
      windows.push_back(timeAndSalesWindow);
      instantiateSecurityWindows = index < defaultSecurities.size() &&
        (nextPosition.x() + width < resolution.width());
      ++index;
    }
    nextPosition.setX(0);
    nextPosition.setY(nextHeight);
    auto toolbar = new Toolbar(Ref(userProfile), nullptr, 0);
    toolbar->move(nextPosition);
    toolbar->show();
    nextPosition.ry() += toolbar->frameSize().height();
    windows.push_back(toolbar);
    auto& globalBlotter = BlotterWindow::GetBlotterWindow(Ref(userProfile),
      Ref(userProfile.GetBlotterSettings().GetConsolidatedBlotter()));
    globalBlotter.move(nextPosition);
    globalBlotter.show();
    globalBlotter.resize(globalBlotter.width(),
      resolution.height() - nextPosition.y() -
      (globalBlotter.frameSize().height() - globalBlotter.size().height()));
    windows.push_back(&globalBlotter);
  }
}

int main(int argc, char* argv[]) {
#ifdef _DEBUG
  freopen("stdout.log", "w", stdout);
  freopen("stderr.log", "w", stderr);
#endif
  QApplication application(argc, argv);
  application.setOrganizationName(QObject::tr("Eidolon Systems"));
  application.setApplicationName(QObject::tr("Spire"));
  application.setApplicationVersion(SPIRE_VERSION);
  RegisterCustomQtVariants();
  InitializeResources();
  auto applicationPath = QStandardPaths::writableLocation(
    QStandardPaths::DataLocation);
  path configPath = applicationPath.toStdString();
  if(!exists(configPath)) {
    create_directories(configPath);
  }
  configPath /= "config.yml";
  if(!is_regular_file(configPath)) {
    std::ofstream configFile(configPath);
    configFile <<
      "---\n"
      "address: 127.0.0.1:20000\n"
      "username: \"\"\n"
      "save_login: true\n"
      "...";
  }
  YAML::Node config;
  try {
    std::ifstream configStream(configPath);
    if(!configStream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return -1;
    }
    config = YAML::Load(configStream);
  } catch(YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  IpAddress address;
  string username;
  bool saveLoginInfo;
  string orderExecutionServiceName;
  try {
    address = Extract<IpAddress>(config, "address");
    username = Extract<string>(config, "username");
    saveLoginInfo = Extract<bool>(config, "save_login");
    orderExecutionServiceName = Extract<string>(config,
      "order_execution_service", OrderExecutionService::SERVICE_NAME);
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;
  LoginDialog loginDialog(address, username, saveLoginInfo,
    Ref(socketThreadPool), Ref(timerThreadPool));
  auto loginResultCode = loginDialog.exec();
  if(loginResultCode == QDialog::Rejected) {
    return -1;
  }
  ServiceClients serviceClients{loginDialog.GetServiceLocatorClient(),
    Ref(socketThreadPool), Ref(timerThreadPool)};
  try {
    serviceClients.Open();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr(e.what()));
    return -1;
  }
  auto isAdministrator =
    serviceClients.GetAdministrationClient().CheckAdministrator(
    serviceClients.GetServiceLocatorClient().GetAccount());
  auto isManager = isAdministrator ||
    !serviceClients.GetAdministrationClient().LoadManagedTradingGroups(
    serviceClients.GetServiceLocatorClient().GetAccount()).empty();
  UserProfile userProfile{loginDialog.GetUsername(), isAdministrator, isManager,
    serviceClients.GetDefinitionsClient().LoadCountryDatabase(),
    serviceClients.GetDefinitionsClient().LoadTimeZoneDatabase(),
    serviceClients.GetDefinitionsClient().LoadCurrencyDatabase(),
    serviceClients.GetDefinitionsClient().LoadExchangeRates(),
    serviceClients.GetDefinitionsClient().LoadMarketDatabase(),
    serviceClients.GetDefinitionsClient().LoadDestinationDatabase(),
    Ref(timerThreadPool), Ref(serviceClients)};
  try {
    userProfile.CreateProfilePath();
  } catch(std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Error creating profile path."));
    return -1;
  }
  BlotterSettings::Load(Store(userProfile));
  CatalogSettings::Load(Store(userProfile));
  BookViewProperties::Load(Store(userProfile));
  RiskTimerProperties::Load(Store(userProfile));
  TimeAndSalesProperties::Load(Store(userProfile));
  PortfolioViewerProperties::Load(Store(userProfile));
  KeyBindings::Load(Store(userProfile));
  InteractionsProperties::Load(Store(userProfile));
  OrderImbalanceIndicatorProperties::Load(Store(userProfile));
  SavedDashboards::Load(Store(userProfile));
  auto windowSettings = WindowSettings::Load(userProfile);
  vector<QWidget*> windows;
  HotkeyOverride hotkeyOverride;
  if(!windowSettings.empty()) {
    for(auto& settings : windowSettings) {
      auto window = settings->Reopen(Ref(userProfile));
      if(window != nullptr) {
        windows.push_back(window);
      }
    }
  } else {
    LoadDefaultLayout(windows, userProfile);
  }
  for(auto& window : windows) {
    window->show();
  }
  auto riskMonitor = RiskTimerMonitor(Ref(userProfile));
  riskMonitor.Load();
  application.exec();
  SavedDashboards::Save(userProfile);
  OrderImbalanceIndicatorProperties::Save(userProfile);
  InteractionsProperties::Save(userProfile);
  KeyBindings::Save(userProfile);
  PortfolioViewerProperties::Save(userProfile);
  TimeAndSalesProperties::Save(userProfile);
  RiskTimerProperties::Save(userProfile);
  BookViewProperties::Save(userProfile);
  CatalogSettings::Save(userProfile);
  BlotterSettings::Save(userProfile);
  return 0;
}
