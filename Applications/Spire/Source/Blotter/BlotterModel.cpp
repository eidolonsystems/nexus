#include "Spire/Blotter/BlotterModel.hpp"
#include <Beam/ServiceLocator/VirtualServiceLocatorClient.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/DateTime.hpp>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"
#include "Spire/Blotter/CancelOnFillController.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Spire;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 100;
}

BlotterModel::BlotterModel(const string& name,
    const DirectoryEntry& executingAccount, bool isConsolidated,
    RefType<UserProfile> userProfile,
    const BlotterTaskProperties& taskProperties,
    const OrderLogProperties& orderLogProperties)
    : m_name(name),
      m_executingAccount(executingAccount),
      m_isConsolidated(isConsolidated),
      m_userProfile(userProfile.Get()),
      m_isPersistent(false),
      m_tasksModel(Ref(*m_userProfile), executingAccount, m_isConsolidated,
        taskProperties),
      m_orderLogModel(orderLogProperties),
      m_profitAndLossModel(Ref(m_userProfile->GetCurrencyDatabase()),
        Ref(userProfile->GetExchangeRates())) {
  m_userProfile->GetServiceClients().GetAdministrationClient().
    GetRiskParametersPublisher(m_executingAccount).Monitor(
    m_slotHandler.GetSlot<RiskParameters>(
    std::bind(&BlotterModel::OnRiskParametersChanged, this,
    std::placeholders::_1)));
  InitializeModels();
  QObject::connect(&m_updateTimer, &QTimer::timeout,
    std::bind(&BlotterModel::OnUpdateTimer, this));
  m_updateTimer.start(UPDATE_INTERVAL);
}

BlotterModel::~BlotterModel() {
  while(!m_incomingLinks.empty()) {
    Unlink(*m_incomingLinks.back());
  }
  while(!m_outgoingLinks.empty()) {
    m_outgoingLinks.back()->Unlink(*this);
  }
}

const string& BlotterModel::GetName() const {
  return m_name;
}

const DirectoryEntry& BlotterModel::GetExecutingAccount() const {
  return m_executingAccount;
}

bool BlotterModel::IsConsolidated() const {
  return m_isConsolidated;
}

bool BlotterModel::IsPersistent() const {
  return m_isPersistent;
}

void BlotterModel::SetPersistent(bool value) {
  m_isPersistent = value;
}

const BlotterTasksModel& BlotterModel::GetTasksModel() const {
  return m_tasksModel;
}

BlotterTasksModel& BlotterModel::GetTasksModel() {
  return m_tasksModel;
}

const OrderLogModel& BlotterModel::GetOrderLogModel() const {
  return m_orderLogModel;
}

OrderLogModel& BlotterModel::GetOrderLogModel() {
  return m_orderLogModel;
}

const OpenPositionsModel& BlotterModel::GetOpenPositionsModel() const {
  return m_openPositionsModel;
}

OpenPositionsModel& BlotterModel::GetOpenPositionsModel() {
  return m_openPositionsModel;
}

const ProfitAndLossModel& BlotterModel::GetProfitAndLossModel() const {
  return m_profitAndLossModel;
}

ProfitAndLossModel& BlotterModel::GetProfitAndLossModel() {
  return m_profitAndLossModel;
}

const ActivityLogModel& BlotterModel::GetActivityLogModel() const {
  return m_activityLogModel;
}

ActivityLogModel& BlotterModel::GetActivityLogModel() {
  return m_activityLogModel;
}

const vector<BlotterModel*>& BlotterModel::GetLinkedBlotters() const {
  return m_incomingLinks;
}

void BlotterModel::Link(RefType<BlotterModel> blotter) {
  if(find(m_incomingLinks.begin(), m_incomingLinks.end(), blotter.Get()) !=
      m_incomingLinks.end()) {
    return;
  }
  m_incomingLinks.push_back(blotter.Get());
  blotter->m_outgoingLinks.push_back(this);
  m_tasksModel.Link(Ref(blotter->GetTasksModel()));
}

void BlotterModel::Unlink(BlotterModel& blotter) {
  auto blotterIterator = find(m_incomingLinks.begin(), m_incomingLinks.end(),
    &blotter);
  if(blotterIterator == m_incomingLinks.end()) {
    return;
  }
  m_tasksModel.Unlink(blotter.GetTasksModel());
  m_incomingLinks.erase(blotterIterator);
  blotter.m_outgoingLinks.erase(find(blotter.m_outgoingLinks.begin(),
    blotter.m_outgoingLinks.end(), this));
  auto outgoingLinks = m_outgoingLinks;
  for(auto& outgoingLink : outgoingLinks) {
    outgoingLink->Unlink(*this);
  }
  InitializeModels();
  for(auto& outgoingLink : outgoingLinks) {
    outgoingLink->Link(Ref(*this));
  }
}

void BlotterModel::InitializeModels() {
  auto& orderExecutionPublisher = m_tasksModel.GetOrderExecutionPublisher();
  m_portfolioMonitor.Initialize(Initialize(m_userProfile->GetMarketDatabase()),
    &m_userProfile->GetServiceClients().GetMarketDataClient(),
    orderExecutionPublisher);
  m_orderLogModel.SetOrderExecutionPublisher(Ref(orderExecutionPublisher));
  m_openPositionsModel.SetPortfolioMonitor(Ref(*m_portfolioMonitor));
  m_profitAndLossModel.SetPortfolioMonitor(Ref(*m_portfolioMonitor));
  m_activityLogModel.SetOrderExecutionPublisher(Ref(orderExecutionPublisher));
  auto currentAccount =
    m_userProfile->GetServiceClients().GetServiceLocatorClient().GetAccount();
  if(m_isConsolidated && m_executingAccount == currentAccount) {
    m_cancelOnFillController = std::make_unique<CancelOnFillController>(
      Ref(*m_userProfile));
    m_cancelOnFillController->SetOrderExecutionPublisher(
      Ref(orderExecutionPublisher));
  }
}

void BlotterModel::OnRiskParametersChanged(
    const RiskParameters& riskParameters) {
  m_profitAndLossModel.SetCurrency(riskParameters.m_currency);
}

void BlotterModel::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
}
