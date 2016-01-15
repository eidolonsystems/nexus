#include "Spire/BookView/BookViewWindow.hpp"
#include <tuple>
#include <Beam/Utilities/Utilities.hpp>
#include <boost/lexical_cast.hpp>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QTableView>
#include "Nexus/OrderTasks/SingleOrderTask.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewPanel.hpp"
#include "Spire/BookView/BookViewPropertiesDialog.hpp"
#include "Spire/BookView/BookViewWindowSettings.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/LinkSecurityContextAction.hpp"
#include "Spire/UI/ValueLabel.hpp"
#include "Spire/Utilities/SecurityTechnicalsModel.hpp"
#include "ui_BookViewWindow.h"

using namespace Beam;
using namespace Beam::Tasks;
using namespace boost;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::OrderTasks;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;

  BookViewWindow* FindNextWindow(const BookViewWindow& origin) {
    auto widgets = QApplication::topLevelWidgets();
    auto rightClosestPosition = std::numeric_limits<int>::max();
    BookViewWindow* rightClosestWindow = nullptr;
    auto bottomLeftPosition =
      QPoint{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
    BookViewWindow* bottomLeftWindow = nullptr;
    auto topLeftPosition =
      QPoint{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
    BookViewWindow* topLeftWindow = nullptr;
    for(auto& widget : widgets) {
      auto candidate = dynamic_cast<BookViewWindow*>(widget);
      if(widget != &origin && candidate != nullptr) {
        auto position = widget->pos();
        if(position.y() >= origin.pos().y() - 20 &&
            position.y() <= origin.pos().y() + 20 &&
            position.x() >= origin.pos().x() &&
            position.x() <= rightClosestPosition) {
          rightClosestWindow = candidate;
          rightClosestPosition = rightClosestWindow->pos().x();
        }
        if(position.y() > origin.pos().y() &&
            position.x() <= bottomLeftPosition.x() &&
            position.y() <= bottomLeftPosition.y()) {
          bottomLeftWindow = candidate;
          bottomLeftPosition = bottomLeftWindow->pos();
        }
        if(std::make_tuple(position.y(), position.x()) <=
            std::make_tuple(topLeftPosition.y(), topLeftPosition.x())) {
          topLeftWindow = candidate;
          topLeftPosition = topLeftWindow->pos();
        }
      }
    }
    if(rightClosestWindow != nullptr) {
      return rightClosestWindow;
    }
    if(bottomLeftWindow != nullptr) {
      return bottomLeftWindow;
    }
    if(topLeftWindow != nullptr) {
      return topLeftWindow;
    }
    return nullptr;
  }

  BookViewWindow* FindPreviousWindow(const BookViewWindow& origin) {
    BookViewWindow* previous = nullptr;
    auto next = FindNextWindow(origin);
    while(next != nullptr && next != &origin) {
      previous = next;
      next = FindNextWindow(*next);
    }
    return previous;
  }
}

BookViewWindow::BookViewWindow(RefType<UserProfile> userProfile,
    const BookViewProperties& properties, const string& identifier,
    QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent, flags),
      SecurityContext(identifier),
      m_ui(std::make_unique<Ui_BookViewWindow>()),
      m_userProfile(userProfile.Get()),
      m_properties(properties),
      m_boardLot(1),
      m_taskEntryWidget(nullptr),
      m_isTaskEntryWidgetForInteractionsProperties(false),
      m_bidPanelGuard(false),
      m_askPanelGuard(false) {
  m_ui->setupUi(this);
  m_ui->m_highValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_lowValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_defaultQuantityValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_volumeValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_openValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_closeValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_bidPanel->Initialize(Ref(*m_userProfile), m_properties, Side::BID);
  m_ui->m_askPanel->Initialize(Ref(*m_userProfile), m_properties, Side::ASK);
  m_ui->m_askPanel->GetQuoteList().setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ui->m_askPanel->GetQuoteList(),
    &QTableView::customContextMenuRequested, this,
    &BookViewWindow::OnContextMenu);
  m_ui->m_bidPanel->GetQuoteList().setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ui->m_bidPanel->GetQuoteList(),
    &QTableView::customContextMenuRequested, this,
    &BookViewWindow::OnContextMenu);
  connect(&m_updateTimer, &QTimer::timeout, this,
    &BookViewWindow::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  SetupSecurityTechnicalsModel();
}

const BookViewProperties& BookViewWindow::GetBookViewProperties() const {
  return m_properties;
}

void BookViewWindow::SetProperties(const BookViewProperties& properties) {
  m_properties = properties;
  m_ui->m_askPanel->SetProperties(m_properties);
  m_ui->m_bidPanel->SetProperties(m_properties);
}

void BookViewWindow::DisplaySecurity(const Security& security) {
  m_security = security;
  m_boardLot = m_userProfile->GetMarketDatabase().FromCode(
    security.GetMarket()).m_boardLot;
  setWindowTitle(QString::fromStdString(
    ToString(security, m_userProfile->GetMarketDatabase())) +
    tr(" - Book View"));
  m_ui->m_askPanel->DisplaySecurity(m_security);
  m_ui->m_bidPanel->DisplaySecurity(m_security);
  SetupSecurityTechnicalsModel();
  SetDisplayedSecurity(m_security);
}

unique_ptr<WindowSettings> BookViewWindow::GetWindowSettings() const {
  return std::make_unique<BookViewWindowSettings>(*this, Ref(*m_userProfile));
}

void BookViewWindow::showEvent(QShowEvent* event) {
  auto context = SecurityContext::FindSecurityContext(m_linkIdentifier);
  if(context.is_initialized()) {
    Link(*context);
  } else {
    m_linkConnection.disconnect();
    m_linkIdentifier.clear();
  }
  QFrame::showEvent(event);
  m_ui->m_askPanel->GetQuoteList().horizontalHeader()->setStretchLastSection(
    true);
  m_ui->m_bidPanel->GetQuoteList().horizontalHeader()->setStretchLastSection(
    true);
}

void BookViewWindow::closeEvent(QCloseEvent* event) {
  if(m_security != Security()) {
    auto settings = std::make_unique<BookViewWindowSettings>(
      *this, Ref(*m_userProfile));
    m_userProfile->AddRecentlyClosedWindow(std::move(settings));
  }
  QFrame::closeEvent(event);
}

void BookViewWindow::keyPressEvent(QKeyEvent* event) {
  if(m_taskEntryWidget != nullptr) {
    HandleTaskInputEvent(event);
  } else if(event->key() == Qt::Key_Tab) {
    auto window = FindNextWindow(*this);
    if(window != nullptr) {
      window->activateWindow();
    }
    return;
  } else if(event->key() == Qt::Key_Backtab) {
    auto window = FindPreviousWindow(*this);
    if(window != nullptr) {
      window->activateWindow();
    }
    return;
  } else if(m_security != Security()) {
    auto key = QKeySequence{
      static_cast<int>(event->modifiers() + event->key())};
    if(key.matches(Qt::Key_QuoteLeft) == QKeySequence::ExactMatch) {
      HandleInteractionsPropertiesEvent();
    } else {
      auto taskBinding = m_userProfile->GetKeyBindings().GetTaskFromBinding(
        m_security.GetMarket(), key);
      if(taskBinding.is_initialized()) {
        HandleKeyBindingEvent(*taskBinding);
        return;
      }
      auto cancelBinding = m_userProfile->GetKeyBindings().GetCancelFromBinding(
        key);
      if(cancelBinding.is_initialized()) {
        HandleCancelBindingEvent(*cancelBinding);
        return;
      }
    }
  }
  HandleSecurityInputEvent(event);
}

void BookViewWindow::HandleLink(SecurityContext& context) {
  m_linkIdentifier = context.GetIdentifier();
  m_linkConnection = context.ConnectSecurityDisplaySignal(
    std::bind(&BookViewWindow::DisplaySecurity, this, std::placeholders::_1));
  DisplaySecurity(context.GetDisplayedSecurity());
}

void BookViewWindow::HandleUnlink() {
  m_linkConnection.disconnect();
  m_linkIdentifier.clear();
}

void BookViewWindow::SetupSecurityTechnicalsModel() {
  m_securityTechnicalsConnections.DisconnectAll();
  m_ui->m_openValue->Reset();
  m_ui->m_closeValue->Reset();
  m_ui->m_highValue->Reset();
  m_ui->m_lowValue->Reset();
  m_ui->m_volumeValue->Reset();
  m_securityTechnicalsModel = SecurityTechnicalsModel::GetModel(
    Ref(*m_userProfile), m_security);
  m_securityTechnicalsConnections.AddConnection(
    m_securityTechnicalsModel->ConnectOpenSignal(std::bind(
    &BookViewWindow::OnOpenUpdate, this, std::placeholders::_1)));
  m_securityTechnicalsConnections.AddConnection(
    m_securityTechnicalsModel->ConnectCloseSignal(std::bind(
    &BookViewWindow::OnCloseUpdate, this, std::placeholders::_1)));
  m_securityTechnicalsConnections.AddConnection(
    m_securityTechnicalsModel->ConnectHighSignal(std::bind(
    &BookViewWindow::OnHighUpdate, this, std::placeholders::_1)));
  m_securityTechnicalsConnections.AddConnection(
    m_securityTechnicalsModel->ConnectLowSignal(std::bind(
    &BookViewWindow::OnLowUpdate, this, std::placeholders::_1)));
  m_securityTechnicalsConnections.AddConnection(
    m_securityTechnicalsModel->ConnectVolumeSignal(std::bind(
    &BookViewWindow::OnVolumeUpdate, this, std::placeholders::_1)));
}

unique_ptr<CanvasNode> BookViewWindow::PrepareTaskNode(const CanvasNode& node) {
  auto taskNode = CanvasNode::Clone(node);
  auto securityNode = taskNode->FindNode(BaseSingleOrderTaskFactory::SECURITY);
  if(securityNode.is_initialized() && !securityNode->IsReadOnly()) {
    if(auto securityValueNode =
        dynamic_cast<const SecurityNode*>(&*securityNode)) {
      CanvasNodeBuilder builder(*taskNode);
      builder.Replace(*securityNode, securityValueNode->SetValue(m_security,
        m_userProfile->GetMarketDatabase()));
      builder.SetReadOnly(*securityNode, true);
      Money price;
      auto sideNode = taskNode->FindNode(BaseSingleOrderTaskFactory::SIDE);
      if(sideNode.is_initialized()) {
        if(auto sideValueNode = dynamic_cast<const SideNode*>(&*sideNode)) {
          if(sideValueNode->GetValue() == Side::BID) {
            price = m_ui->m_bidPanel->GetBestQuote().m_price;
          } else {
            price = m_ui->m_askPanel->GetBestQuote().m_price;
          }
        } else {
          price = m_ui->m_bidPanel->GetBestQuote().m_price;
        }
      } else {
        price = m_ui->m_bidPanel->GetBestQuote().m_price;
      }
      auto priceNode = taskNode->FindNode(BaseSingleOrderTaskFactory::PRICE);
      if(priceNode.is_initialized() && !priceNode->IsReadOnly()) {
        if(auto moneyNode = dynamic_cast<const MoneyNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetValue(price));
        } else if(auto moneyNode =
            dynamic_cast<const OptionalPriceNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetReferencePrice(price));
        }
      }
      auto quantityNode = taskNode->FindNode(
        BaseSingleOrderTaskFactory::QUANTITY);
      if(quantityNode.is_initialized() && !quantityNode->IsReadOnly()) {
        Quantity quantity;
        if(auto quantityValueNode = dynamic_cast<const IntegerNode*>(
            &*quantityNode)) {
          auto sideNode = taskNode->FindNode(BaseSingleOrderTaskFactory::SIDE);
          if(sideNode.is_initialized()) {
            if(auto sideValueNode = dynamic_cast<const SideNode*>(&*sideNode)) {
              quantity = m_userProfile->GetDefaultQuantity(m_security,
                sideValueNode->GetValue());
            } else {
              quantity = m_userProfile->GetInteractionProperties().Get(
                m_security).m_defaultQuantity;
            }
          } else {
            quantity = m_userProfile->GetInteractionProperties().Get(
              m_security).m_defaultQuantity;
          }
          builder.Replace(*quantityNode, quantityValueNode->SetValue(quantity));
        }
      }
      taskNode = builder.Build();
    }
  }
  return taskNode;
}

void BookViewWindow::RemoveTaskEntry() {
  setUpdatesEnabled(false);
  m_ui->verticalLayout->removeWidget(m_taskEntryWidget);
  m_taskEntryWidget->deleteLater();
  m_taskEntryWidget = nullptr;
  m_isTaskEntryWidgetForInteractionsProperties = false;
  setUpdatesEnabled(true);
}

void BookViewWindow::ExecuteTask(const CanvasNode& node) {
  auto errors = Validate(node);
  if(!errors.empty()) {
    QMessageBox::warning(this, QObject::tr("Error"),
      QString::fromStdString(errors.front().GetErrorMessage()));
    return;
  }
  auto& activeBlotter = m_userProfile->GetBlotterSettings().GetActiveBlotter();
  auto& blotterWindow = BlotterWindow::GetBlotterWindow(Ref(*m_userProfile),
    Ref(activeBlotter));
  if(!blotterWindow.isVisible()) {
    blotterWindow.setAttribute(Qt::WA_ShowWithoutActivating);
    blotterWindow.show();
    raise();
  }
  auto& entry = activeBlotter.GetTasksModel().Add(node);
  m_tasksExecuted[m_security].push_back(entry.m_context);
  entry.m_context->m_task->GetPublisher().Monitor(
    m_slotHandler.GetSlot<Task::StateEntry>(std::bind(
    &BookViewWindow::OnTaskState, this,
    std::weak_ptr<BlotterTasksModel::TaskContext>(entry.m_context),
    std::placeholders::_1)));
  entry.m_context->m_task->Execute();
}

void BookViewWindow::HandleSecurityInputEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_PageUp) {
    m_securityViewStack.PushUp(m_security,
      [&] (const Security& security) {
        this->DisplaySecurity(security);
      });
    return;
  } else if(key == Qt::Key_PageDown) {
    m_securityViewStack.PushDown(m_security,
      [&] (const Security& security) {
        this->DisplaySecurity(security);
      });
    return;
  }
  auto text = event->text();
  if(text.isEmpty() || !text[0].isLetterOrNumber()) {
    return;
  }
  SecurityInputDialog dialog(Ref(*m_userProfile), text.toStdString(), this);
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto security = dialog.GetSecurity();
  if(security == Security() || security == m_security) {
    return;
  }
  m_securityViewStack.Push(m_security);
  DisplaySecurity(security);
}

void BookViewWindow::HandleKeyBindingEvent(
    const KeyBindings::TaskBinding& keyBinding) {
  auto taskNode = PrepareTaskNode(*keyBinding.m_node);
  m_taskEntryWidget = new CondensedCanvasWidget(keyBinding.m_name,
    Ref(*m_userProfile), this);
  auto coordinate = CanvasNodeModel::Coordinate{0, 0};
  bool isVisible;
  try {
    m_taskEntryWidget->Add(coordinate, *taskNode);
    isVisible = true;
  } catch(const CanvasNodeNotVisibleException&) {
    isVisible = false;
  }
  if(isVisible) {
    m_taskEntryWidget->setSizePolicy(QSizePolicy::Preferred,
      QSizePolicy::Fixed);
    m_ui->verticalLayout->insertWidget(2, m_taskEntryWidget);
    m_taskEntryWidget->Focus();
  } else {
    m_taskEntryWidget->deleteLater();
    m_taskEntryWidget = nullptr;
    ExecuteTask(*taskNode);
  }
}

void BookViewWindow::HandleInteractionsPropertiesEvent() {
  auto& interactionsProperties =
    m_userProfile->GetInteractionProperties().Get(m_security);
  auto interactionsNode = std::make_unique<InteractionsNode>(m_security,
    m_userProfile->GetMarketDatabase(), interactionsProperties);
  m_taskEntryWidget = new CondensedCanvasWidget("Interactions",
    Ref(*m_userProfile), this);
  m_isTaskEntryWidgetForInteractionsProperties = true;
  auto coordinate = CanvasNodeModel::Coordinate{0, 0};
  m_taskEntryWidget->Add(coordinate, *interactionsNode);
  m_taskEntryWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_ui->verticalLayout->insertWidget(2, m_taskEntryWidget);
  m_taskEntryWidget->Focus();
}

void BookViewWindow::HandleCancelBindingEvent(
    const KeyBindings::CancelBinding& cancelBinding) {
  KeyBindings::CancelBinding::HandleCancel(cancelBinding,
    Store(m_tasksExecuted[m_security]));
}

void BookViewWindow::HandleTaskInputEvent(QKeyEvent* event) {
  auto baseKey = event->key();
  if(baseKey == Qt::Key_Escape) {
    RemoveTaskEntry();
  } else if(baseKey == Qt::Key_Enter || baseKey == Qt::Key_Return) {
    if(m_isTaskEntryWidgetForInteractionsProperties) {
      auto& interactionsNode = static_cast<const InteractionsNode&>(
        *m_taskEntryWidget->GetRoots().front());
      m_userProfile->GetInteractionProperties().Set(m_security,
        interactionsNode.GetProperties());
      RemoveTaskEntry();
    } else {
      auto taskNode = CanvasNode::Clone(*m_taskEntryWidget->GetRoots().front());
      RemoveTaskEntry();
      ExecuteTask(*taskNode);
    }
  } else if(baseKey == Qt::Key_QuoteLeft) {
    RemoveTaskEntry();
    HandleInteractionsPropertiesEvent();
  } else {
    auto key = QKeySequence{
      static_cast<int>(event->modifiers() + event->key())};
    auto taskBinding = m_userProfile->GetKeyBindings().GetTaskFromBinding(
      m_security.GetMarket(), key);
    if(taskBinding.is_initialized()) {
      RemoveTaskEntry();
      HandleKeyBindingEvent(*taskBinding);
    }
  }
}

void BookViewWindow::UpdateDefaultQuantity() {
  auto defaultBidQuantity =
    m_userProfile->GetDefaultQuantity(m_security, Side::BID);
  auto defaultAskQuantity =
    m_userProfile->GetDefaultQuantity(m_security, Side::ASK);
  m_ui->m_defaultQuantityValue->SetValue(QString::fromStdString(
    lexical_cast<string>(defaultBidQuantity) + "x" +
    lexical_cast<string>(defaultAskQuantity)));
}

void BookViewWindow::OnOpenUpdate(Money open) {
  m_ui->m_openValue->SetValue(QVariant::fromValue(open));
}

void BookViewWindow::OnCloseUpdate(Money close) {
  m_ui->m_closeValue->SetValue(QVariant::fromValue(close));
}

void BookViewWindow::OnHighUpdate(Money high) {
  m_ui->m_highValue->SetValue(QVariant::fromValue(high));
}

void BookViewWindow::OnLowUpdate(Money low) {
  m_ui->m_lowValue->SetValue(QVariant::fromValue(low));
}

void BookViewWindow::OnVolumeUpdate(Quantity volume) {
  m_ui->m_volumeValue->SetValue(QVariant::fromValue(volume));
}

void BookViewWindow::OnTaskState(
    std::weak_ptr<BlotterTasksModel::TaskContext> weakTask,
    const Task::StateEntry& update) {
  auto task = weakTask.lock();
  if(task == nullptr) {
    return;
  }
  if(IsTerminal(update.m_state)) {
    RemoveFirst(m_tasksExecuted[m_security], task);
  }
}

void BookViewWindow::OnContextMenu(const QPoint& position) {
  QMenu contextMenu;
  QAction propertiesAction(&contextMenu);
  propertiesAction.setText(tr("Properties"));
  propertiesAction.setToolTip(tr("Opens the Book View properties."));
  contextMenu.addAction(&propertiesAction);
  QMenu linkMenu("Links");
  auto linkActions = LinkSecurityContextAction::BuildActions(this,
    m_linkIdentifier, &linkMenu, *m_userProfile);
  for(auto& action : linkActions) {
    linkMenu.addAction(action.get());
  }
  if(!linkMenu.isEmpty()) {
    contextMenu.addMenu(&linkMenu);
  }
  auto selectedAction = contextMenu.exec(
    static_cast<QWidget*>(sender())->mapToGlobal(position));
  if(selectedAction == &propertiesAction) {
    BookViewPropertiesDialog propertiesWidget(Ref(*m_userProfile), m_security,
      m_properties, this);
    if(propertiesWidget.exec() == QDialog::Rejected) {
      return;
    }
    SetProperties(propertiesWidget.GetProperties());
  } else if(LinkSecurityContextAction* linkAction =
      dynamic_cast<LinkSecurityContextAction*>(selectedAction)) {
    linkAction->Execute(Store(*this));
  }
}

void BookViewWindow::OnUpdateTimer() {
  HandleTasks(m_slotHandler);
  UpdateDefaultQuantity();
}
