#include "spire/ui/properties_window_buttons_widget.hpp"
#include <QGridLayout>
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/flat_button.hpp"

using namespace boost::signals2;
using namespace spire;

properties_window_buttons_widget::properties_window_buttons_widget(
    QWidget* parent)
    : QWidget(parent) {
  setFixedHeight(scale_height(78));
  auto main_layout = new QHBoxLayout(this);
  main_layout->setContentsMargins(0, scale_height(10), 0, scale_height(10));
  main_layout->setSpacing(0);
  auto left_widget = new QWidget(this);
  left_widget->setFixedSize(scale(208, 60));
  auto left_layout = new QGridLayout(left_widget);
  left_layout->setContentsMargins({});
  left_layout->setHorizontalSpacing(scale_width(8));
  left_layout->setVerticalSpacing(scale_height(8));
  auto generic_button_default_style = QString(R"(
    background-color: #EBEBEB;
    color: black;
    font-family: Roboto;
    font-size: %1px;
    qproperty-alignment: AlignCenter;)").arg(scale_height(12));
  auto generic_button_hover_style = QString(R"(
    background-color: #4B23A0;
    color: white;)");
  auto generic_button_focused_style = QString(R"(
    border: %1px solid #4B23A0 %2px solid #4B23A0;)")
    .arg(scale_height(1)).arg(scale_width(1));
  auto save_as_default_button = new flat_button(tr("Save As Default"));
  save_as_default_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  save_as_default_button->connect_clicked_signal(
    [=] { m_save_as_default_signal(); });
  left_layout->addWidget(save_as_default_button, 0, 0);
  auto load_default_button = new flat_button(tr("Load Default"));
  load_default_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  load_default_button->connect_clicked_signal(
    [=] { m_load_default_signal(); });
  left_layout->addWidget(load_default_button, 1, 0);
  auto reset_default_button = new flat_button(tr("Reset Default"));
  reset_default_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  reset_default_button->connect_clicked_signal(
    [=] { m_reset_default_signal(); });
  left_layout->addWidget(reset_default_button, 1, 1);
  main_layout->addWidget(left_widget);
  main_layout->addStretch(1);
  auto right_widget = new QWidget(this);
  right_widget->setFixedSize(scale(208, 60));
  auto right_layout = new QGridLayout(right_widget);
  right_layout->setContentsMargins({});
  right_layout->setHorizontalSpacing(scale_width(8));
  right_layout->setVerticalSpacing(scale_height(8));
  auto apply_to_all_button = new flat_button(tr("Apply To All"));
  apply_to_all_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  apply_to_all_button->connect_clicked_signal(
    [=] { m_apply_to_all_signal(); });
  right_layout->addWidget(apply_to_all_button, 0, 0);
  auto cancel_button = new flat_button(tr("Cancel"));
  cancel_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  cancel_button->connect_clicked_signal([=] { m_cancel_signal(); });
  right_layout->addWidget(cancel_button, 1, 0);
  auto apply_button = new flat_button(tr("Apply"));
  apply_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  apply_button->connect_clicked_signal([=] { m_apply_signal(); });
  right_layout->addWidget(apply_button, 0, 1);
  auto ok_button = new flat_button(tr("OK"));
  ok_button->set_stylesheet(generic_button_default_style,
    generic_button_hover_style, generic_button_focused_style, "");
  ok_button->connect_clicked_signal([=] { m_ok_signal(); });
  right_layout->addWidget(ok_button, 1, 1);
  main_layout->addWidget(right_widget);
}

connection properties_window_buttons_widget::connect_save_as_default_signal(
    const clicked_signal::slot_type& slot) const {
  return m_save_as_default_signal.connect(slot);
}

connection properties_window_buttons_widget::connect_load_default_signal(
    const clicked_signal::slot_type& slot) const {
  return m_load_default_signal.connect(slot);
}

connection properties_window_buttons_widget::connect_reset_default_signal(
    const clicked_signal::slot_type& slot) const {
  return m_reset_default_signal.connect(slot);
}

connection properties_window_buttons_widget::connect_apply_to_all_signal(
    const clicked_signal::slot_type& slot) const {
  return m_apply_to_all_signal.connect(slot);
}

connection properties_window_buttons_widget::connect_cancel_signal(
    const clicked_signal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

connection properties_window_buttons_widget::connect_apply_signal(
    const clicked_signal::slot_type& slot) const {
  return m_apply_signal.connect(slot);
}

connection properties_window_buttons_widget::connect_ok_signal(
    const clicked_signal::slot_type& slot) const {
  return m_ok_signal.connect(slot);
}
