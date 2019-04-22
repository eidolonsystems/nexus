#include "spire/book_view/book_view_properties_dialog.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "spire/book_view/book_view_highlight_properties_widget.hpp"
#include "spire/book_view/book_view_level_properties_widget.hpp"
#include "spire/key_bindings/interactions_properties_widget.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/properties_window_buttons_widget.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

BookViewPropertiesDialog::BookViewPropertiesDialog(
    const BookViewProperties& properties, const Security& security,
    QWidget* parent)
    : Dialog(parent),
        m_last_focus_was_key(false) {
  setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint
    & ~Qt::WindowMaximizeButtonHint);
  setWindowModality(Qt::WindowModal);
  setFixedSize(scale(492, 394));
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto body = new QWidget(this);
  body->setStyleSheet("background-color: #F5F5F5;");
  setWindowTitle(tr("Properties"));
  set_svg_icon(":/icons/bookview-black.svg",
    ":/icons/bookview-grey.svg");
  auto layout = new QVBoxLayout(body);
  layout->setContentsMargins(scale_width(8), scale_height(10), scale_width(8),
    0);
  layout->setSpacing(0);
  m_tab_widget = new QTabWidget(this);
  m_tab_widget->tabBar()->setFixedHeight(scale_height(40));
  m_tab_widget->setStyleSheet(QString(R"(
    QWidget {
      outline: none;
    }

    QTabWidget::pane {
      border: none;
    }

    QTabBar::tab {
      background-color: #EBEBEB;
      font-family: Roboto;
      font-size: %1px;
      height: %2px;
      margin: %3px %4px %3px 0px;
      width: %5px;
    }

    QTabBar::tab:focus {
      border: %6px solid #4B23A0;
      padding: -%6px 0px 0px -%6px;
    }

    QTabBar::tab:hover {
      color: #4B23A0;
    }

    QTabBar::tab:selected {
      background-color: #F5F5F5;
      color: #4B23A0;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1)));
  m_levels_tab_widget = new BookViewLevelPropertiesWidget(properties,
    m_tab_widget);
  m_tab_widget->addTab(m_levels_tab_widget, tr("Price Levels"));
  m_highlights_tab_widget = new BookViewHighlightPropertiesWidget(
    properties, m_tab_widget);
  m_tab_widget->addTab(m_highlights_tab_widget, tr("Highlights"));
  if(security != Security()) {
    auto interactions_tab_widget = new InteractionsPropertiesWidget(
      m_tab_widget);
    m_tab_widget->addTab(interactions_tab_widget, tr("Interactions"));
  }
  layout->addWidget(m_tab_widget);
  connect(m_tab_widget, &QTabWidget::currentChanged, this,
    &BookViewPropertiesDialog::on_tab_changed);
  m_tab_widget->tabBar()->installEventFilter(this);
  connect(m_tab_widget->tabBar(), &QTabBar::tabBarClicked, this,
    &BookViewPropertiesDialog::on_tab_bar_clicked);
  auto button_group_widget = new PropertiesWindowButtonsWidget(this);
  layout->addWidget(button_group_widget);
  button_group_widget->connect_apply_signal(
    [=] { m_apply_signal(get_properties()); });
  button_group_widget->connect_apply_to_all_signal(
    [=] { m_apply_all_signal(get_properties()); });
  button_group_widget->connect_cancel_signal([=] { reject(); });
  button_group_widget->connect_ok_signal([=] { accept(); });
  button_group_widget->connect_save_as_default_signal(
    [=] { m_save_default_signal(get_properties()); });
  Window::layout()->addWidget(body);
}

BookViewProperties BookViewPropertiesDialog::get_properties() const {
  BookViewProperties properties;
  m_levels_tab_widget->apply(properties);
  m_highlights_tab_widget->apply(properties);
  return properties;
}

connection BookViewPropertiesDialog::connect_apply_signal(
    const ApplySignal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection BookViewPropertiesDialog::connect_apply_all_signal(
    const ApplyAllSignal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection BookViewPropertiesDialog::connect_save_default_signal(
    const SaveDefaultSignal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}

bool BookViewPropertiesDialog::eventFilter(QObject* watched,
    QEvent* event) {
  if(watched == m_tab_widget->tabBar()) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Left || e->key() == Qt::Key_Right) {
        m_last_focus_was_key = true;
      }
    } else if(event->type() == QEvent::MouseButtonPress) {
      m_last_focus_was_key = false;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void BookViewPropertiesDialog::on_tab_bar_clicked(int index) {
  if(index > -1) {
    m_tab_widget->widget(index)->setFocus();
  }
}

void BookViewPropertiesDialog::on_tab_changed() {
  if(m_last_focus_was_key) {
    m_tab_widget->tabBar()->setFocus();
  }
}
