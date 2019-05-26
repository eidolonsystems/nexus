#include <Beam/Threading/TimerThreadPool.hpp>
#include <QApplication>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSalesUiTester/TimeAndSalesTestControllerWindow.hpp"

using namespace Beam;
using namespace Beam::Threading;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Time and Sales UI Tester"));
  initialize_resources();
  TimerThreadPool timer_thread_pool;
  LocalSecurityInputModel model;
  model.add(SecurityInfo(
    Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
    "Microsoft Corp", "Software"));
  model.add(SecurityInfo(
    Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Magna International Inc.", "Automotive, probably"));
  model.add(SecurityInfo(
    Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Manulife Financial Corporation", "Finance"));
  model.add(SecurityInfo(
    Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Methanex Corporation", ""));
  model.add(SecurityInfo(
    Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
    "Metro Inc.", ""));
  model.add(SecurityInfo(
    Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Monsanto Co.", ""));
  model.add(SecurityInfo(
    Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
    "Morgan Stanley", "Finance"));
  auto test_window = new TimeAndSalesWindow(TimeAndSalesProperties(),
    Ref(model));
  auto tester = new TimeAndSalesTestControllerWindow(test_window,
    timer_thread_pool);
  test_window->show();
  tester->show();
  tester->move(test_window->pos().x() + test_window->width() + 100,
    tester->pos().y());
  test_window->activateWindow();
  application->exec();
}
