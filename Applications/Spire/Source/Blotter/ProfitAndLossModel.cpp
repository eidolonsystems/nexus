#include "Spire/Blotter/ProfitAndLossModel.hpp"
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/Blotter/ProfitAndLossEntryModel.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::Accounting;
using namespace Spire;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 100;
}

ProfitAndLossModel::ProfitAndLossModel(
    RefType<const CurrencyDatabase> currencyDatabase,
    RefType<const ExchangeRateTable> exchangeRates)
    : m_currencyDatabase(currencyDatabase.Get()),
      m_exchangeRates(exchangeRates.Get()),
      m_portfolioMonitor(nullptr) {
  m_slotHandler.Initialize();
  connect(&m_updateTimer, &QTimer::timeout, this,
    &ProfitAndLossModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
}

ProfitAndLossModel::~ProfitAndLossModel() {}

void ProfitAndLossModel::SetPortfolioMonitor(
    RefType<SpirePortfolioMonitor> portfolioMonitor) {
  for(auto& model : m_currencyToModel | boost::adaptors::map_values) {
    m_profitAndLossEntryModelRemovedSignal(*model);
    model.reset();
  }
  m_currencyToModel.clear();
  m_update = SpirePortfolioMonitor::UpdateEntry();
  m_update.m_currencyInventory.m_position.m_key.m_currency = m_currency;
  m_update.m_securityInventory.m_position.m_key.m_currency = m_currency;
  m_slotHandler.Reset();
  m_slotHandler.Initialize();
  m_portfolioMonitor = portfolioMonitor.Get();
  m_portfolioMonitor->GetPublisher().Monitor(
    m_slotHandler->GetSlot<SpirePortfolioMonitor::UpdateEntry>(
    std::bind(&ProfitAndLossModel::OnPortfolioUpdate, this,
    std::placeholders::_1)));
  m_profitAndLossUpdateSignal(m_update);
}

void ProfitAndLossModel::SetCurrency(CurrencyId currency) {
  if(currency == m_currency) {
    return;
  }
  m_currency = currency;
  if(m_portfolioMonitor != nullptr) {
    SetPortfolioMonitor(Ref(*m_portfolioMonitor));
  } else {
    m_update = SpirePortfolioMonitor::UpdateEntry();
    m_update.m_currencyInventory.m_position.m_key.m_currency = m_currency;
    m_update.m_securityInventory.m_position.m_key.m_currency = m_currency;
  }
}

connection ProfitAndLossModel::ConnectProfitAndLossUpdateSignal(
    const ProfitAndLossUpdateSignal::slot_type& slot) const {
  slot(m_update);
  return m_profitAndLossUpdateSignal.connect(slot);
}

connection ProfitAndLossModel::ConnectProfitAndLossEntryModelAddedSignal(
    const ProfitAndLossEntryModelAddedSignal::slot_type& slot) const {
  return m_profitAndLossEntryModelAddedSignal.connect(slot);
}

connection ProfitAndLossModel::ConnectProfitAndLossEntryModelRemovedSignal(
    const ProfitAndLossEntryModelRemovedSignal::slot_type& slot) const {
  return m_profitAndLossEntryModelRemovedSignal.connect(slot);
}

void ProfitAndLossModel::OnPortfolioUpdate(
    const SpirePortfolioMonitor::UpdateEntry& update) {
  auto& key = update.m_securityInventory.m_position.m_key;
  auto& model = m_currencyToModel[key.m_currency];
  if(model == nullptr) {
    model = std::make_unique<ProfitAndLossEntryModel>(
      m_currencyDatabase->FromId(key.m_currency));
    m_profitAndLossEntryModelAddedSignal(*model);
    m_currencyToPortfolio.insert(std::make_pair(key.m_currency, update));
  } else {
    auto& previous = m_currencyToPortfolio[key.m_currency];
    if(m_currency != CurrencyId::NONE()) {
      m_update.m_currencyInventory.m_position.m_quantity -=
        previous.m_currencyInventory.m_position.m_quantity;
      m_update.m_currencyInventory.m_position.m_costBasis -=
        m_exchangeRates->Convert(
        previous.m_currencyInventory.m_position.m_costBasis, key.m_currency,
        m_currency);
      m_update.m_currencyInventory.m_grossProfitAndLoss -=
        m_exchangeRates->Convert(
        previous.m_currencyInventory.m_grossProfitAndLoss, key.m_currency,
        m_currency);
      m_update.m_currencyInventory.m_fees -= m_exchangeRates->Convert(
        previous.m_currencyInventory.m_fees, key.m_currency, m_currency);
      m_update.m_currencyInventory.m_volume -=
        previous.m_currencyInventory.m_volume;
      m_update.m_currencyInventory.m_transactionCount -=
        previous.m_currencyInventory.m_transactionCount;
      m_update.m_unrealizedCurrency -= m_exchangeRates->Convert(
        previous.m_unrealizedCurrency, key.m_currency, m_currency);
    }
    previous = update;
  }
  model->OnPortfolioUpdate(update);
  if(m_currency != CurrencyId::NONE()) {
    m_update.m_currencyInventory.m_position.m_quantity +=
      update.m_currencyInventory.m_position.m_quantity;
    m_update.m_currencyInventory.m_position.m_costBasis +=
      m_exchangeRates->Convert(
      update.m_currencyInventory.m_position.m_costBasis,
      key.m_currency, m_currency);
    m_update.m_currencyInventory.m_grossProfitAndLoss +=
      m_exchangeRates->Convert(update.m_currencyInventory.m_grossProfitAndLoss,
      key.m_currency, m_currency);
    m_update.m_currencyInventory.m_fees += m_exchangeRates->Convert(
      update.m_currencyInventory.m_fees, key.m_currency, m_currency);
    m_update.m_currencyInventory.m_volume +=
      update.m_currencyInventory.m_volume;
    m_update.m_currencyInventory.m_transactionCount +=
      update.m_currencyInventory.m_transactionCount;
    m_update.m_unrealizedCurrency += m_exchangeRates->Convert(
      update.m_unrealizedCurrency, key.m_currency, m_currency);
    m_profitAndLossUpdateSignal(m_update);
  }
}

void ProfitAndLossModel::OnUpdateTimer() {
  HandleTasks(*m_slotHandler);
}
