#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;
}

TimeAndSalesModel::TimeAndSalesModel(RefType<UserProfile> userProfile,
    const TimeAndSalesProperties& properties, const Security& security)
    : m_userProfile(userProfile.Get()),
      m_properties(properties) {
  connect(&m_updateTimer, &QTimer::timeout, this,
    &TimeAndSalesModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  if(security == Security()) {
    return;
  }
  ptime marketStartOfDay = MarketDateToUtc(security.GetMarket(),
    m_userProfile->GetServiceClients().GetTimeClient().GetTime(),
    m_userProfile->GetMarketDatabase(), m_userProfile->GetTimeZoneDatabase());
  SecurityMarketDataQuery query;
  query.SetIndex(security);
  query.SetRange(marketStartOfDay, Beam::Queries::Sequence::Last());
  query.SetSnapshotLimit(SnapshotLimit::Type::TAIL, 50);
  query.SetInterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryTimeAndSales(
    query, m_slotHandler.GetSlot<TimeAndSale>(
    std::bind(&TimeAndSalesModel::OnTimeAndSale, this, std::placeholders::_1)));
  SecurityMarketDataQuery bboQuery = BuildRealTimeWithSnapshotQuery(security);
  bboQuery.SetInterruptionPolicy(InterruptionPolicy::IGNORE_CONTINUE);
  m_userProfile->GetServiceClients().GetMarketDataClient().QueryBboQuotes(
    bboQuery, m_slotHandler.GetSlot<BboQuote>(
    std::bind(&TimeAndSalesModel::OnBbo, this, std::placeholders::_1)));
}

void TimeAndSalesModel::SetProperties(
    const TimeAndSalesProperties& properties) {
  m_properties = properties;
  if(!m_entries.empty()) {
    Q_EMIT dataChanged(index(0, 0),
      index(static_cast<int>(m_entries.size()) - 1,
      TimeAndSalesProperties::COLUMN_COUNT - 1));
  }
  beginResetModel();
  endResetModel();
}

int TimeAndSalesModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int TimeAndSalesModel::columnCount(const QModelIndex& parent) const {
  return TimeAndSalesProperties::COLUMN_COUNT;
}

QVariant TimeAndSalesModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  const pair<TimeAndSale, PriceRange>& entry = m_entries[
    m_entries.size() - index.row() - 1];
  if(role == Qt::TextAlignmentRole) {
    if(index.column() == MARKET_COLUMN || index.column() == CONDITION_COLUMN) {
      return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::BackgroundRole) {
    return m_properties.GetPriceRangeBackgroundColor()[entry.second];
  } else if(role == Qt::ForegroundRole) {
    return m_properties.GetPriceRangeForegroundColor()[entry.second];
  } else if(role == Qt::DisplayRole) {
    const TimeAndSale& timeAndSale = entry.first;
    if(index.column() == TIME_COLUMN) {
      return QVariant::fromValue(timeAndSale.m_timestamp);
    } else if(index.column() == PRICE_COLUMN) {
      return QVariant::fromValue(timeAndSale.m_price);
    } else if(index.column() == SIZE_COLUMN) {
      return timeAndSale.m_size;
    } else if(index.column() == MARKET_COLUMN) {
      return QString::fromStdString(timeAndSale.m_marketCenter);
    } else if(index.column() == CONDITION_COLUMN) {
      return QString::fromStdString(timeAndSale.m_condition.m_code);
    }
  }
  return QVariant();
}

QVariant TimeAndSalesModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == TIME_COLUMN) {
      return tr("Time");
    } else if(section == PRICE_COLUMN) {
      return tr("Px");
    } else if(section == SIZE_COLUMN) {
      return tr("Qty");
    } else if(section == MARKET_COLUMN) {
      return tr("Mkt");
    } else if(section == CONDITION_COLUMN) {
      return tr("Cond");
    }
  }
  return QVariant();
}

void TimeAndSalesModel::OnBbo(const BboQuote& bbo) {
  m_bbo = bbo;
}

void TimeAndSalesModel::OnTimeAndSale(const TimeAndSale& timeAndSale) {
  PriceRange priceRange;
  if(m_bbo.m_ask.m_side == Side::NONE) {
    priceRange = UNKNOWN;
  } else if(timeAndSale.m_price == m_bbo.m_bid.m_price) {
    priceRange = AT_BID;
  } else if(timeAndSale.m_price < m_bbo.m_bid.m_price) {
    priceRange = BELOW_BID;
  } else if(timeAndSale.m_price == m_bbo.m_ask.m_price) {
    priceRange = AT_ASK;
  } else if(timeAndSale.m_price > m_bbo.m_ask.m_price) {
    priceRange = ABOVE_ASK;
  } else {
    priceRange = INSIDE;
  }
  beginInsertRows(QModelIndex(), 0, 0);
  m_entries.push_back(make_pair(timeAndSale, priceRange));
  endInsertRows();
}

void TimeAndSalesModel::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
