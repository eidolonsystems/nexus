#include "Spire/Dashboard/LastPriceDashboardCellBuilder.hpp"
#include <Beam/Queues/ConverterReaderQueue.hpp>
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/Dashboard/QueueDashboardCell.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;

std::unique_ptr<DashboardCell> LastPriceDashboardCellBuilder::Build(
    const DashboardCell::Value& index, Ref<UserProfile> userProfile) const {
  auto& security = boost::get<Security>(index);
  auto& marketDataClient =
    userProfile.Get()->GetServiceClients().GetMarketDataClient();
  auto baseQueue = std::make_shared<Queue<TimeAndSale>>();
  std::shared_ptr<QueueReader<Money>> queue =
    MakeConverterReaderQueue<Money>(baseQueue,
    [] (const TimeAndSale& timeAndSale) {
      return timeAndSale.m_price;
    });
  auto query = BuildCurrentQuery(security);
  marketDataClient.QueryTimeAndSales(query, baseQueue);
  auto last = std::make_unique<QueueDashboardCell>(queue);
  return std::move(last);
}

std::unique_ptr<DashboardCellBuilder>
    LastPriceDashboardCellBuilder::Clone() const {
  return std::make_unique<LastPriceDashboardCellBuilder>();
}
