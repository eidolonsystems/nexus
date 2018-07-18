#include "spire/book_view/book_view_highlight_properties_widget.hpp"
#include <QButtonGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "spire/book_view/book_view_properties.hpp"
#include "spire/book_view/market_list_item.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/check_box.hpp"

using namespace Nexus;
using namespace spire;

book_view_highlight_properties_widget::book_view_highlight_properties_widget(
    const book_view_properties& properties, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, scale_height(8), 0, scale_height(20));
  layout->setSpacing(0);
  auto markets_layout = new QVBoxLayout();
  markets_layout->setContentsMargins({});
  markets_layout->setSpacing(0);
  auto markets_label = new QLabel(tr("Markets"), this);
  auto generic_header_label_stylesheet = QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  markets_label->setStyleSheet(generic_header_label_stylesheet);
  markets_layout->addWidget(markets_label, 14);
  markets_layout->addStretch(10);
  m_markets_list_widget = new QListWidget(this);
  auto market_database = GetDefaultMarketDatabase().GetEntries();
  for(auto i = 0; i < static_cast<int>(market_database.size()); ++i) {
    auto item = new market_list_item(market_database[i], m_markets_list_widget);
    item->setTextAlignment(Qt::AlignCenter);
    auto highlight = properties.get_market_highlight(
      market_database[i].m_code);
    if(highlight.is_initialized()) {
      item->setBackgroundColor(highlight->m_color);
    } else {
      item->setBackgroundColor(Qt::white);
    }
  }
  m_markets_list_widget->item(0)->setSelected(true);
  m_markets_list_widget->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
  m_markets_list_widget->setSelectionBehavior(
    QAbstractItemView::SelectionBehavior::SelectRows);
  connect(m_markets_list_widget, &QListWidget::currentRowChanged,
    [=] (auto index) { update_market_widgets(index); });
  m_markets_list_widget->setFixedWidth(scale_width(140));
  markets_layout->addWidget(m_markets_list_widget, 222);
  layout->addLayout(markets_layout, 140);
  layout->addStretch(18);
  auto market_highlight_layout = new QVBoxLayout();
  market_highlight_layout->setContentsMargins({});
  market_highlight_layout->setSpacing(0);
  market_highlight_layout->addStretch(26);
  m_highlight_none_check_box = new check_box(tr("Highlight None"), this);
  auto check_box_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;
    outline: none;
    spacing: %2px;)")
    .arg(scale_height(12)).arg(scale_width(4));
  auto check_box_indicator_style = QString(R"(
    background-color: white;
    border: %1px solid #C8C8C8 %2px solid #C8C8C8;
    height: %3px;
    width: %4px;)").arg(scale_height(1))
    .arg(scale_width(1)).arg(scale_height(15)).arg(scale_width(15));
  auto check_box_checked_style = QString(R"(
    image: url(:/icons/check-with-box.svg);)");
  auto check_box_hover_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto check_box_focused_style = QString(R"(border-color: #4B23A0;)");
  m_highlight_none_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  market_highlight_layout->addWidget(m_highlight_none_check_box, 16);
  market_highlight_layout->addStretch(10);
  m_highlight_top_level_check_box = new check_box(tr("Highlight Top Level"),
    this);
  m_highlight_top_level_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  market_highlight_layout->addWidget(m_highlight_top_level_check_box, 16);
  market_highlight_layout->addStretch(10);
  m_highlight_all_levels_check_box = new check_box(
    tr("Highlight All Levels"), this);
  m_highlight_all_levels_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  market_highlight_layout->addWidget(m_highlight_all_levels_check_box, 16);
  auto market_check_box_button_group = new QButtonGroup(this);
  market_check_box_button_group->addButton(m_highlight_none_check_box);
  market_check_box_button_group->addButton(m_highlight_top_level_check_box);
  market_check_box_button_group->addButton(m_highlight_all_levels_check_box);
  market_highlight_layout->addStretch(18);
  auto market_highlight_color_label = new QLabel(tr("Highlight Color"), this);
  auto generic_label_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  market_highlight_color_label->setStyleSheet(generic_label_text_style);
  market_highlight_layout->addWidget(market_highlight_color_label, 14);
  market_highlight_layout->addStretch(4);
  m_market_highlight_color_button = new flat_button(this);
  m_market_highlight_color_button->setFixedWidth(scale_width(100));
  update_color_button_stylesheet(m_market_highlight_color_button,
    Qt::yellow);
  m_market_highlight_color_button->connect_clicked_signal(
    [=] { on_market_highlight_color_button_clicked(); });
  market_highlight_layout->addWidget(m_market_highlight_color_button, 26);
  market_highlight_layout->addStretch(92);
  layout->addLayout(market_highlight_layout, 130);
  layout->addStretch(18);
  auto vertical_rule = new QWidget(this);
  vertical_rule->setStyleSheet("background-color: #C8C8C8;");
  layout->addWidget(vertical_rule, 1);
  layout->addStretch(18);
  auto orders_layout = new QVBoxLayout();
  orders_layout->setContentsMargins({});
  orders_layout->setSpacing(0);
  auto orders_label = new QLabel(tr("Orders"), this);
  orders_label->setStyleSheet(generic_header_label_stylesheet);
  orders_layout->addWidget(orders_label, 14);
  orders_layout->addStretch(10);
  auto hide_orders_check_box = new check_box(tr("Hide Orders"), this);
  hide_orders_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  orders_layout->addWidget(hide_orders_check_box, 16);
  orders_layout->addStretch(10);
  auto display_orders_check_box = new check_box(tr("Display Orders"), this);
  display_orders_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  orders_layout->addWidget(display_orders_check_box, 16);
  orders_layout->addStretch(10);
  auto highlight_orders_check_box = new check_box(tr("Highlight Orders"),
    this);
  highlight_orders_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style, check_box_hover_style,
    check_box_focused_style);
  orders_layout->addWidget(highlight_orders_check_box, 16);
  orders_layout->addStretch(18);
  auto order_highlight_color_label = new QLabel(tr("Highlight Color"), this);
  order_highlight_color_label->setStyleSheet(generic_label_text_style);
  orders_layout->addWidget(order_highlight_color_label, 14);
  orders_layout->addStretch(4);
  auto order_highlight_color_button = new flat_button(this);
  order_highlight_color_button->setFixedWidth(scale_width(100));
  update_color_button_stylesheet(order_highlight_color_button, Qt::yellow);
  orders_layout->addWidget(order_highlight_color_button, 26);
  orders_layout->addStretch(92);
  layout->addLayout(orders_layout, 151);
  layout->addStretch(27);
}

void book_view_highlight_properties_widget::apply(
    book_view_properties& properties) const {
}

void book_view_highlight_properties_widget::update_color_button_stylesheet(
    flat_button* button, const QColor& color) {
  auto s = button->get_style();
  s.m_background_color = color;
  s.m_border_color = QColor("#C8C8C8");
  button->set_style(s);
  s.m_border_color = QColor("#4B23A0");
  button->set_hover_style(s);
  button->set_focus_style(s);
}

void book_view_highlight_properties_widget::update_market_widgets(
    int selected_item_index) {
  auto selected_item = static_cast<market_list_item*>(
    m_markets_list_widget->item(selected_item_index));
  auto& market_highlight = selected_item->get_market_highlight();
  if(market_highlight.is_initialized()) {
    update_color_button_stylesheet(m_market_highlight_color_button,
      market_highlight->m_color);
    if(market_highlight->m_highlight_all_levels) {
      m_highlight_all_levels_check_box->setChecked(true);
    } else {
      m_highlight_top_level_check_box->setChecked(true);
    }
  } else {
    update_color_button_stylesheet(m_market_highlight_color_button, Qt::white);
    m_highlight_none_check_box->setChecked(true);
  }
  update_market_list_stylesheet(selected_item_index);
}

void book_view_highlight_properties_widget::update_market_list_stylesheet(
    int selected_item_index) {
  m_markets_list_widget->setStyleSheet(QString(R"(
    QListWidget {
      background-color: white;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      outline: none;
      padding: %3px %4px 0px %4px;
    }

    QListWidget::item:selected {
      background-color: %7;
      border: %5px solid #4B23A0 %6px solid #4B23A0;
      color: #000000;
    })").arg(scale_height(1)).arg(scale_width(1))
        .arg(scale_height(4)).arg(scale_width(4))
        .arg(scale_height(1)).arg(scale_width(1))
        .arg(m_markets_list_widget->item(
          selected_item_index)->background().color().name()));
}

void book_view_highlight_properties_widget::
    on_market_highlight_color_button_clicked() {
  auto item = static_cast<market_list_item*>(
    m_markets_list_widget->currentItem());
  auto color = QColorDialog::getColor(item->backgroundColor());
  if(color.isValid()) {
    item->set_highlight_color(color);
    update_market_widgets(m_markets_list_widget->currentRow());
    item->setBackgroundColor(color);
    update_market_list_stylesheet(m_markets_list_widget->currentRow());
  }
}
