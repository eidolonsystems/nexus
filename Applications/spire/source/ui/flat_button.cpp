#include "spire/ui/flat_button.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointF>
#include "spire/spire/dimensions.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;
using style = flat_button::style;

flat_button::flat_button(QWidget* parent)
  : flat_button("", parent) {}

flat_button::flat_button(const QString& label, QWidget* parent)
    : QWidget(parent),
      m_clickable(true),
      m_last_focus_reason(Qt::MouseFocusReason) {
  m_label = new QLabel(label, this);
  m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_label);
  setFocusPolicy(Qt::StrongFocus);
  m_default_style.m_background_color = Qt::white;
  m_default_style.m_border_color = Qt::transparent;
  m_default_style.m_text_color = Qt::black;
  m_hover_style.m_background_color = Qt::white;
  m_hover_style.m_border_color = Qt::transparent;
  m_hover_style.m_text_color = Qt::black;
  m_focus_style.m_background_color = Qt::white;
  m_focus_style.m_border_color = Qt::transparent;
  m_focus_style.m_text_color = Qt::black;
  m_disabled_style.m_background_color = Qt::transparent;
  m_disabled_style.m_border_color = Qt::white;
  m_disabled_style.m_text_color = Qt::white;
  set_style(m_default_style);
  set_hover_style(m_hover_style);
  set_focus_style(m_focus_style);
  set_disabled_style(m_disabled_style);
}

void flat_button::set_text(const QString& text) {
  m_label->setText(text);
}

const style& flat_button::get_style() const {
  return m_default_style;
}

void flat_button::set_style(const style& default_style) {
  m_default_style = default_style;
  on_style_updated();
}

const style& flat_button::get_hover_style() const {
  return m_hover_style;
}

void flat_button::set_hover_style(const style& hover_style) {
  m_hover_style = hover_style;
  on_style_updated();
}

const style& flat_button::get_focus_style() const {
  return m_focus_style;
}

void flat_button::set_focus_style(const style& focus_style) {
  m_focus_style = focus_style;
  on_style_updated();
}

const style& flat_button::get_disabled_style() const {
  return m_disabled_style;
}

void flat_button::set_disabled_style(const style& disabled_style) {
  m_disabled_style = disabled_style;
  on_style_updated();
}

connection flat_button::connect_clicked_signal(
    const clicked_signal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

void flat_button::changeEvent(QEvent* event) {
  if(event->type() == QEvent::FontChange) {
    m_label->setFont(font());
  } else if(event->type() == QEvent::EnabledChange) {
    if(!isEnabled()) {
      disable_button();
    } else {
      enable_button();
    }
  }
}

void flat_button::focusInEvent(QFocusEvent* event) {
  if(event->reason() == Qt::ActiveWindowFocusReason) {
    if(m_last_focus_reason == Qt::MouseFocusReason) {
      set_hover_stylesheet();
    } else {
      set_focus_stylesheet();
    }
    return;
  } else if(event->reason() != Qt::MouseFocusReason) {
    set_focus_stylesheet();
  } else {
    set_hover_stylesheet();
  }
  m_last_focus_reason = event->reason();
}

void flat_button::focusOutEvent(QFocusEvent* event) {
  set_hover_stylesheet();
}

void flat_button::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_clickable) {
      m_clicked_signal();
    }
  }
}

void flat_button::mousePressEvent(QMouseEvent* event) {
  event->accept();
}

void flat_button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && m_clickable) {
    set_hover_stylesheet();
    m_last_focus_reason = Qt::MouseFocusReason;
    if(rect().contains(event->localPos().toPoint())) {
      m_clicked_signal();
    }
  }
}

void flat_button::disable_button() {
  m_clickable = false;
  setFocusPolicy(Qt::NoFocus);
  set_disabled_stylesheet();
}

void flat_button::enable_button() {
  m_clickable = true;
  setFocusPolicy(Qt::StrongFocus);
  set_hover_stylesheet();
}

QString flat_button::get_stylesheet_properties(const style& s) {
  return QString(R"(
      background-color: %1;
      border: %2px solid %3 %4px solid %3;
      color: %5;
      qproperty-alignment: AlignCenter;)")
        .arg(s.m_background_color.name(QColor::HexArgb)).arg(scale_height(1))
        .arg(s.m_border_color.name(QColor::HexArgb)).arg(scale_width(1))
        .arg(s.m_text_color.name(QColor::HexArgb));
}

void flat_button::set_disabled_stylesheet() {
  m_label->setStyleSheet(get_stylesheet_properties(m_disabled_style));
}

void flat_button::set_focus_stylesheet() {
  m_label->setStyleSheet(
    QString(R"(QLabel { %1 })").arg(
      get_stylesheet_properties(m_default_style)) +
    QString(R"(QLabel:hover { %1 })").arg(
      get_stylesheet_properties(m_hover_style)) +
    QString(R"(QLabel { %1 })").arg(
      get_stylesheet_properties(m_focus_style)));
}

void flat_button::set_hover_stylesheet() {
  m_label->setStyleSheet(
    QString(R"(QLabel { %1 })").arg(
      get_stylesheet_properties(m_default_style)) +
    QString(R"(QLabel:hover { %1 })").arg(
      get_stylesheet_properties(m_hover_style)));
}

void flat_button::on_style_updated() {
  if(!hasFocus() && isEnabled()) {
    set_hover_stylesheet();
  } else if(hasFocus()) {
    set_focus_stylesheet();
  } else {
    set_disabled_stylesheet();
  }
}
