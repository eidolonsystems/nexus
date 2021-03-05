#include "Spire/Ui/Button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

Button::Button(QWidget* component, QWidget* parent)
    : StyledWidget(parent),
      m_component(component),
      m_is_down(false) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_component);
  setFocusPolicy(Qt::StrongFocus);
  auto style = get_style();
  style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
    set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  style.get(Hover() || Focus()).set(
    border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
  style.get(Disabled()).
    set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
    set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  set_style(std::move(style));
}

connection Button::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void Button::focusOutEvent(QFocusEvent* event) {
  if(event->reason() != Qt::PopupFocusReason && m_is_down) {
    m_is_down = false;
  }
  QWidget::focusOutEvent(event);
}

void Button::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(!event->isAutoRepeat()) {
        m_clicked_signal();
      }
      break;
    case Qt::Key_Space:
      if(!event->isAutoRepeat()) {
        m_is_down = true;
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::keyReleaseEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Space:
      if(!event->isAutoRepeat() && m_is_down) {
        m_is_down = false;
        m_clicked_signal();
      }
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void Button::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
    m_is_down = true;
  }
  QWidget::mousePressEvent(event);
}

void Button::mouseReleaseEvent(QMouseEvent* event) {
  if(m_is_down && event->button() == Qt::LeftButton &&
      rect().contains(event->pos())) {
    m_is_down = false;
    m_clicked_signal();
  }
  QWidget::mouseReleaseEvent(event);
}

Button* Spire::make_icon_button(QImage icon, QWidget* parent) {
  auto button = new Button(new Icon(icon, parent), parent);
  auto style = StyleSheet();
  style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
    set(border_size(0));
  style.get(Any() < Hover()).
    set(BackgroundColor(QColor::fromRgb(0xE3, 0xE3, 0xE3)));
  style.get(Any() < Focus()).set(
    border(scale_width(1), QColor::fromRgb(0x4B, 0x23, 0xA0)));
  button->set_style(std::move(style));
  return button;
}

Button* Spire::make_label_button(const QString& label, QWidget* parent) {
  auto text_box = new TextBox(label);
  text_box->set_read_only(true);
  text_box->setDisabled(true);
  auto button = new Button(text_box, parent);
  auto style = StyleSheet();
  style.get(Any()).
    set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
    set(BackgroundColor(QColor::fromRgb(0xEB, 0xEB, 0xEB))).
    set(border_size(0));
  style.get(Disabled()).set(TextColor(QColor::fromRgb(0, 0, 0)));
  style.get(Any() < Hover()).
    set(BackgroundColor(QColor::fromRgb(0x4B, 0x23, 0xA0))).
    set(TextColor(QColor::fromRgb(0xFF, 0xFF, 0xFF)));
  style.get(Any() < Focus()).set(
    border(scale_width(1), QColor::fromRgb(0x4B, 0x23, 0xA0)));
  style.get(Any() < Disabled()).set(
    TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  button->set_style(std::move(style));
  return button;
}
