#include "spire/time_and_sales/time_and_sales_properties_dialog.hpp"
#include <QColorDialog>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/time_and_sales/time_and_sales_model.hpp"
#include "spire/ui/check_box.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/properties_window_buttons_widget.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;
using PriceRange = TimeAndSalesProperties::PriceRange;
using Columns = TimeAndSalesProperties::Columns;

TimeAndSalesPropertiesDialog::TimeAndSalesPropertiesDialog(
    const TimeAndSalesProperties& properties, QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags | Qt::FramelessWindowHint |
        Qt::WindowCloseButtonHint) {
  m_body = new QWidget(this);
  m_body->setFixedSize(scale(462, 272));
  m_body->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto window_layout = new QHBoxLayout(this);
  window_layout->setContentsMargins({});
  auto window = new spire::window(m_body, this);
  window->set_svg_icon(":/icons/time-sale-black.svg",
    ":/icons/time-sale-grey.svg");
  this->window()->setWindowTitle(tr("Properties"));
  window_layout->addWidget(window);
  m_body->setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QVBoxLayout(m_body);
  layout->setContentsMargins(scale_width(8), 0, scale_width(8), 0);
  layout->setSpacing(0);
  layout->addStretch(10);
  auto section_label_style = QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(12));
  auto band_appearance_label = new QLabel(tr("Band Appearance"), this);
  band_appearance_label->setStyleSheet(section_label_style);
  layout->addWidget(band_appearance_label);
  layout->setStretchFactor(band_appearance_label, 14);
  layout->addStretch(10);
  auto style_layout = new QHBoxLayout();
  style_layout->setContentsMargins({});
  style_layout->setSpacing(0);
  auto band_list_layout = new QVBoxLayout();
  band_list_layout->setContentsMargins({});
  band_list_layout->setSpacing(0);
  m_band_list = new QListWidget(this);
  m_band_list->setSelectionMode(
    QAbstractItemView::SelectionMode::SingleSelection);
  m_band_list->setSelectionBehavior(
    QAbstractItemView::SelectionBehavior::SelectRows);
  connect(m_band_list, &QListWidget::currentRowChanged,
    [=] (auto index) {update_band_list_stylesheet(index);});
  auto band_unknown_item = new QListWidgetItem(tr("Bid/Ask Unknown"),
    m_band_list);
  band_unknown_item->setTextAlignment(Qt::AlignCenter);
  auto above_ask_item = new QListWidgetItem(tr("Trade Above Ask"),
    m_band_list);
  above_ask_item->setTextAlignment(Qt::AlignCenter);
  auto at_ask_item = new QListWidgetItem(tr("Trade At Ask"),
    m_band_list);
  at_ask_item->setTextAlignment(Qt::AlignCenter);
  auto inside_item = new QListWidgetItem(tr("Trade Inside"),
    m_band_list);
  inside_item->setTextAlignment(Qt::AlignCenter);
  auto at_bid_item = new QListWidgetItem(tr("Trade At Bid"),
    m_band_list);
  at_bid_item->setTextAlignment(Qt::AlignCenter);
  auto below_bid_item = new QListWidgetItem(tr("Trade Below Bid"),
    m_band_list);
  below_bid_item->setTextAlignment(Qt::AlignCenter);
  m_band_list->setFont(properties.m_font);
  m_band_list->setItemSelected(band_unknown_item, true);
  band_list_layout->addWidget(m_band_list);
  band_list_layout->setStretchFactor(m_band_list, 140);
  style_layout->addLayout(band_list_layout);
  style_layout->setStretchFactor(band_list_layout, 190);
  style_layout->addStretch(18);
  auto color_settings_layout = new QVBoxLayout();
  color_settings_layout->setContentsMargins({});
  color_settings_layout->setSpacing(0);
  auto text_color_label = new QLabel(tr("Text Color"), this);
  auto generic_text_style = QString(R"(
    color: black;
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(12));
  text_color_label->setStyleSheet(generic_text_style);
  color_settings_layout->addWidget(text_color_label);
  color_settings_layout->setStretchFactor(text_color_label, 14);
  color_settings_layout->addStretch(4);
  m_text_color_button = new flat_button(this);
  m_text_color_button->connect_clicked_signal(
    [=] { set_text_color(); });
  set_color_button_stylesheet(m_text_color_button,
    properties.get_text_color(
    PriceRange::UNKNOWN));
  color_settings_layout->addWidget(m_text_color_button);
  color_settings_layout->setStretchFactor(m_text_color_button, 26);
  color_settings_layout->addStretch(10);
  auto band_color_label = new QLabel(tr("Band Color"), this);
  band_color_label->setStyleSheet(generic_text_style);
  color_settings_layout->addWidget(band_color_label);
  color_settings_layout->setStretchFactor(band_color_label, 14);
  color_settings_layout->addStretch(4);
  m_band_color_button = new flat_button(this);
  m_band_color_button->connect_clicked_signal(
    [=] { set_band_color(); });
  set_color_button_stylesheet(m_band_color_button, properties.get_band_color(
    PriceRange::UNKNOWN));
  color_settings_layout->addWidget(m_band_color_button);
  color_settings_layout->setStretchFactor(m_band_color_button, 26);
  color_settings_layout->addStretch(42);
  style_layout->addLayout(color_settings_layout);
  style_layout->setStretchFactor(color_settings_layout, 100);
  style_layout->addStretch(18);
  auto font_layout = new QVBoxLayout();
  font_layout->setContentsMargins({});
  font_layout->setSpacing(0);
  font_layout->addStretch(18);
  auto edit_font_button = new flat_button(tr("Change Font"), this);
  edit_font_button->connect_clicked_signal([=] { set_font(); });
  QFont generic_button_font;
  generic_button_font.setFamily("Roboto");
  generic_button_font.setPixelSize(scale_height(12));
  edit_font_button->setFont(generic_button_font);
  auto  generic_button_default_style = edit_font_button->get_style();
  generic_button_default_style.m_background_color = QColor("#EBEBEB");
  generic_button_default_style.m_text_color = Qt::black;
  auto generic_button_hover_style = edit_font_button->get_hover_style();
  generic_button_hover_style.m_background_color = QColor("#4B23A0");
  generic_button_hover_style.m_text_color = Qt::white;
  auto generic_button_focused_style = edit_font_button->get_focus_style();
  generic_button_focused_style.m_background_color = QColor("#EBEBEB");
  generic_button_focused_style.m_border_color = QColor("#4B23A0");
  edit_font_button->set_style(generic_button_default_style);
  edit_font_button->set_hover_style(generic_button_hover_style);
  edit_font_button->set_focus_style(generic_button_focused_style);
  font_layout->addWidget(edit_font_button);
  font_layout->setStretchFactor(edit_font_button, 26);
  font_layout->addStretch(33);
  m_show_grid_check_box = new check_box(tr("Show Grid"), this);
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
  m_show_grid_check_box->set_stylesheet(check_box_text_style,
    check_box_indicator_style, check_box_checked_style,
    check_box_hover_style, check_box_focused_style);
  font_layout->addWidget(m_show_grid_check_box);
  font_layout->setStretchFactor(m_show_grid_check_box, 16);
  font_layout->addStretch(47);
  style_layout->addLayout(font_layout);
  style_layout->setStretchFactor(font_layout, 100);
  style_layout->addStretch(20);
  layout->addLayout(style_layout);
  layout->setStretchFactor(style_layout, 140);
  layout->addStretch(20);
  auto buttons_layout = new QHBoxLayout();
  buttons_layout->setContentsMargins({});
  buttons_layout->setSpacing(0);
  auto buttons_widget = new properties_window_buttons_widget(this);
  buttons_widget->connect_save_as_default_signal(
    [=] { m_save_default_signal(get_properties()); });
  buttons_widget->connect_apply_to_all_signal(
    [=] { m_apply_all_signal(get_properties()); });
  buttons_widget->connect_apply_signal(
    [=] { m_apply_signal(get_properties()); });
  buttons_widget->connect_ok_signal([=] { accept(); });
  buttons_widget->connect_cancel_signal([=] { reject(); });
  buttons_layout->addWidget(buttons_widget);
  layout->addLayout(buttons_layout);
  layout->setStretchFactor(buttons_layout, 60);
  set_properties(properties);
}

TimeAndSalesProperties
    TimeAndSalesPropertiesDialog::get_properties() const {
  TimeAndSalesProperties properties;
  auto unknown_item = m_band_list->item(0);
  properties.set_band_color(PriceRange::UNKNOWN,
    unknown_item->backgroundColor());
  properties.set_text_color(PriceRange::UNKNOWN,
    unknown_item->textColor());
  auto above_ask_item = m_band_list->item(1);
  properties.set_band_color(PriceRange::ABOVE_ASK,
    above_ask_item->backgroundColor());
  properties.set_text_color(PriceRange::ABOVE_ASK,
    above_ask_item->textColor());
  auto at_ask_item = m_band_list->item(2);
  properties.set_band_color(PriceRange::AT_ASK,
    at_ask_item->backgroundColor());
  properties.set_text_color(PriceRange::AT_ASK,
    at_ask_item->textColor());
  auto inside_item = m_band_list->item(3);
  properties.set_band_color(PriceRange::INSIDE,
    inside_item->backgroundColor());
  properties.set_text_color(PriceRange::INSIDE,
    inside_item->textColor());
  auto at_bid_item = m_band_list->item(4);
  properties.set_band_color(PriceRange::AT_BID,
    at_bid_item->backgroundColor());
  properties.set_text_color(PriceRange::AT_BID,
    at_bid_item->textColor());
  auto below_bid_item = m_band_list->item(5);
  properties.set_band_color(PriceRange::BELOW_BID,
    below_bid_item->backgroundColor());
  properties.set_text_color(PriceRange::BELOW_BID,
    below_bid_item->textColor());
  properties.m_show_grid = m_show_grid_check_box->isChecked();
  properties.m_font = m_band_list->font();
  return properties;
}

connection TimeAndSalesPropertiesDialog::connect_apply_signal(
    const ApplySignal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection TimeAndSalesPropertiesDialog::connect_apply_all_signal(
    const ApplyAllSignal::slot_type& slot) const {
  return m_apply_all_signal.connect(slot);
}

connection TimeAndSalesPropertiesDialog::connect_save_default_signal(
    const SaveDefaultSignal::slot_type& slot) const {
  return m_save_default_signal.connect(slot);
}

void TimeAndSalesPropertiesDialog::mousePressEvent(QMouseEvent* event) {
  setFocus();
}

void TimeAndSalesPropertiesDialog::showEvent(QShowEvent* event) {
  m_band_list->setFocus();
  auto parent_geometry = static_cast<QWidget*>(parent())->geometry();
  move(parent_geometry.center().x() - (width() / 2),
    parent_geometry.center().y() - (height() / 2));
}

void TimeAndSalesPropertiesDialog::set_band_color() {
  auto index = m_band_list->currentIndex().row();
  auto band = static_cast<PriceRange>(index);
  auto current_color = m_properties.get_band_color(band);
  auto color = QColorDialog::getColor(current_color);
  if(color.isValid()) {
    m_properties.set_band_color(band, color);
    m_band_list->item(index)->setBackground(color);
    update_band_list_stylesheet(index);
  }
}

void TimeAndSalesPropertiesDialog::set_font() {
  auto ok = false;
  auto font = QFontDialog::getFont(&ok, m_band_list->font());
  if(ok) {
    m_properties.m_font = font;
    m_band_list->setFont(font);
  }
}

void TimeAndSalesPropertiesDialog::set_text_color() {
  auto index = m_band_list->currentIndex().row();
  auto band = static_cast<PriceRange>(index);
  auto current_color = m_properties.get_text_color(band);
  auto color = QColorDialog::getColor(current_color);
  if(color.isValid()) {
    m_properties.set_text_color(band, color);
    m_band_list->item(index)->setForeground(color);
    update_band_list_stylesheet(index);
  }
}

void TimeAndSalesPropertiesDialog::set_color_button_stylesheet(
    flat_button* button, const QColor& color) {
  auto s = button->get_style();
  s.m_background_color = color;
  s.m_border_color = QColor("#C8C8C8");
  button->set_style(s);
  s.m_border_color = QColor("#4B23A0");
  button->set_hover_style(s);
  button->set_focus_style(s);
}

void TimeAndSalesPropertiesDialog::set_color_settings_stylesheet(
    int band_index) {
  auto i = static_cast<PriceRange>(band_index);
  set_color_button_stylesheet(m_band_color_button,
    m_properties.get_band_color(i));
  set_color_button_stylesheet(m_text_color_button,
    m_properties.get_text_color(i));
}

void TimeAndSalesPropertiesDialog::set_properties(
    const TimeAndSalesProperties& properties) {
  m_properties = properties;
  auto unknown_item = m_band_list->item(0);
  unknown_item->setBackground(m_properties.get_band_color(
    PriceRange::UNKNOWN));
  unknown_item->setForeground(m_properties.get_text_color(
    PriceRange::UNKNOWN));
  auto above_ask_item = m_band_list->item(1);
  above_ask_item->setBackground(m_properties.get_band_color(
    PriceRange::ABOVE_ASK));
  above_ask_item->setForeground(m_properties.get_text_color(
    PriceRange::ABOVE_ASK));
  auto at_ask_item = m_band_list->item(2);
  at_ask_item->setBackground(m_properties.get_band_color(
    PriceRange::AT_ASK));
  at_ask_item->setForeground(m_properties.get_text_color(
    PriceRange::AT_ASK));
  auto inside_item = m_band_list->item(3);
  inside_item->setBackground(m_properties.get_band_color(
    PriceRange::INSIDE));
  inside_item->setForeground(m_properties.get_text_color(
    PriceRange::INSIDE));
  auto at_bid_item = m_band_list->item(4);
  at_bid_item->setBackground(m_properties.get_band_color(
    PriceRange::AT_BID));
  at_bid_item->setForeground(m_properties.get_text_color(
    PriceRange::AT_BID));
  auto below_bid_item = m_band_list->item(5);
  below_bid_item->setBackground(m_properties.get_band_color(
    PriceRange::BELOW_BID));
  below_bid_item->setForeground(m_properties.get_text_color(
    PriceRange::BELOW_BID));
  if(m_band_list->currentRow() > -1) {
    set_color_settings_stylesheet(m_band_list->currentRow());
  } else {
    set_color_settings_stylesheet(0);
  }
  m_show_grid_check_box->setChecked(m_properties.m_show_grid);
}

void TimeAndSalesPropertiesDialog::update_band_list_stylesheet(
    int highlighted_band_index) {
  set_color_settings_stylesheet(highlighted_band_index);
  auto i = static_cast<PriceRange>(highlighted_band_index);
  m_band_list->setStyleSheet(QString(R"(
    QListWidget {
      background-color: white;
      border: %1px solid #C8C8C8 %2px solid #C8C8C8;
      outline: none;
      padding: %3px %4px 0px %4px;
    }

    QListWidget::item:selected {
      background-color: %7;
      border: %5px solid #4B23A0 %6px solid #4B23A0;
      color: %8;
    })").arg(scale_height(1)).arg(scale_width(1))
        .arg(scale_height(4)).arg(scale_width(4))
        .arg(scale_height(1)).arg(scale_width(1))
        .arg(m_properties.get_band_color(i).name())
        .arg(m_properties.get_text_color(i).name()));
}
