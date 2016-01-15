#include "Spire/AccountViewer/AccountActivityReportWidget.hpp"
#include <Beam/Queues/QueuePublisher.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "ui_AccountActivityReportWidget.h"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

AccountActivityReportWidget::ReportModel::ReportModel(
    RefType<UserProfile> userProfile,
    const std::shared_ptr<OrderExecutionPublisher>& orderPublisher)
    : m_orderPublisher(orderPublisher),
      m_profitAndLossModel(Ref(userProfile->GetCurrencyDatabase()),
        Ref(userProfile->GetExchangeRates())),
      m_portfolioMonitor(Beam::Initialize(userProfile->GetMarketDatabase()),
        &userProfile->GetServiceClients().GetMarketDataClient(),
        *m_orderPublisher) {
  m_profitAndLossModel.SetPortfolioMonitor(Ref(m_portfolioMonitor));
}

AccountActivityReportWidget::AccountActivityReportWidget(QWidget* parent,
    Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_AccountActivityReportWidget>()),
      m_userProfile(nullptr) {
  m_ui->setupUi(this);
  connect(m_ui->m_updateButton, &QPushButton::clicked, this,
    &AccountActivityReportWidget::OnUpdate);
}

AccountActivityReportWidget::~AccountActivityReportWidget() {}

void AccountActivityReportWidget::Initialize(RefType<UserProfile> userProfile,
    const DirectoryEntry& account) {
  m_userProfile = userProfile.Get();
  m_account = account;
  m_ui->m_fromPeriodDateEdit->setDate(ToQDateTime(ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime())).date());
  m_ui->m_fromPeriodDateEdit->setTime(QTime(0, 0, 0, 0));
  m_ui->m_toPeriodDateEdit->setDate(ToQDateTime(ToLocalTime(
    m_userProfile->GetServiceClients().GetTimeClient().GetTime())).date());
  m_ui->m_toPeriodDateEdit->setTime(QTime(23, 59, 59, 0));
}

void AccountActivityReportWidget::OnUpdate(bool checked) {
  auto startTime = ToPosixTime(m_ui->m_fromPeriodDateEdit->dateTime().toUTC());
  auto endTime = ToPosixTime(m_ui->m_toPeriodDateEdit->dateTime().toUTC());
  AccountQuery query;
  query.SetIndex(m_account);
  query.SetRange(startTime, endTime);
  query.SetSnapshotLimit(SnapshotLimit::Unlimited());
  auto orderQueue = std::make_shared<Queue<const Order*>>();
  m_userProfile->GetServiceClients().GetOrderExecutionClient().
    QueryOrderSubmissions(query, orderQueue);
  auto orderPublisher = std::make_shared<QueuePublisher<
    SequencePublisher<const Order*>>>(orderQueue);
  m_model.Reset();
  m_model.Initialize(Ref(*m_userProfile), orderPublisher);
  m_ui->m_profitAndLossWidget->SetModel(Ref(*m_userProfile),
    Ref(m_model->m_profitAndLossModel));
}
