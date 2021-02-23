#include "Spire/Ui/Button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  void update_text_padding(const Box::Style& style, TextBox::Style& text_style) {
    if(style.m_paddings) {
      if(style.m_borders) {
        text_style.m_paddings = *style.m_paddings + *style.m_borders;
      } else {
        text_style.m_paddings = *style.m_paddings;
      }
    }
  }
}

Button::Button(TextBox* text_box, QWidget* parent)
    : Box(parent),
      m_text_box(text_box) {
  m_text_box->setParent(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_text_box);
  setLayout(layout);
  m_text_box->installEventFilter(this);
  setFocusProxy(m_text_box);
  connect(m_text_box, &QLineEdit::selectionChanged, [=] {
    m_text_box->deselect();
  });
}

const QString& Button::get_text() const {
  return m_text_box->get_text();
}

void Button::set_text(const QString& label) {
  m_text_box->set_text(label);
}

const TextBox::Style& Button::get_text_style() const {
  return m_text_box->get_styles().m_read_only_style;
}

void Button::set_text_style(const TextBox::Style& style) {
  auto styles = m_text_box->get_styles();
  styles.m_read_only_style = style;
  m_text_box->set_styles(styles);
}

const TextBox::Style& Button::get_text_hover_style() const {
  return m_text_box->get_styles().m_read_only_hover_style;
}

void Button::set_text_hover_style(const TextBox::Style& style) {
  auto styles = m_text_box->get_styles();
  styles.m_read_only_hover_style = style;
  m_text_box->set_styles(styles);
}

const TextBox::Style& Button::get_text_focus_style() const {
  return m_text_box->get_styles().m_read_only_focus_style;
}

void Button::set_text_focus_style(const TextBox::Style& style) {
  auto styles = m_text_box->get_styles();
  styles.m_read_only_focus_style = style;
  m_text_box->set_styles(styles);
}

const TextBox::Style& Button::get_text_disabled_style() const {
  return m_text_box->get_styles().m_read_only_disabled_style;
}

void Button::set_text_disabled_style(const TextBox::Style& style) {
  auto styles = m_text_box->get_styles();
  styles.m_read_only_disabled_style = style;
  m_text_box->set_styles(styles);
}

connection Button::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

bool Button::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_text_box) {
    switch(event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
      event->ignore();
      return true;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
      Box::event(event);
      break;
    }
  }
  return Box::eventFilter(watched, event);
}

void Button::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
      if(!event->isAutoRepeat()) {
        m_clicked_signal();
      }
      break;
    default:
      Box::keyPressEvent(event);
  }
}

void Button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
    m_clicked_signal();
  }
  Box::mouseReleaseEvent(event);
}

void Button::process_style(const Style& style) {
  auto text_style = get_text_style();
  update_text_padding(style, text_style);
  set_text_style(text_style);
}

void Button::process_hover_style(const Style& hover_style) {
  auto text_style = get_text_hover_style();
  update_text_padding(hover_style, text_style);
  set_text_hover_style(text_style);
}

void Button::process_focus_style(const Style& focus_style) {
  auto text_style = get_text_focus_style();
  update_text_padding(focus_style, text_style);
  set_text_focus_style(text_style);
}

void Button::process_disabled_style(const Style& disabled_style) {
  auto text_style = get_text_disabled_style();
  update_text_padding(disabled_style, text_style);
  set_text_disabled_style(text_style);
}

Button* Spire::make_button(const QString& label, QWidget* parent) {
  auto text_box = new TextBox(label);
  text_box->setReadOnly(true);
  text_box->setAlignment(Qt::AlignCenter);
  text_box->setFocusPolicy(Qt::NoFocus);
  text_box->setContextMenuPolicy(Qt::NoContextMenu);
  text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto button = new Button(text_box, parent);
  button->setFocusPolicy(Qt::StrongFocus);
  auto text_style = button->get_text_style();
  text_style.m_background_color = QColor("#00000000");
  text_style.m_text_color = QColor("#000000");
  button->set_text_style(text_style);
  auto text_hover_style = button->get_text_hover_style();
  text_hover_style.m_background_color = QColor("#00000000");
  text_hover_style.m_text_color = QColor("#FFFFFF");
  button->set_text_hover_style(text_hover_style);
  auto text_focus_style = button->get_text_focus_style();
  text_focus_style.m_background_color = QColor("#00000000");
  text_focus_style.m_text_color = QColor("#000000");
  button->set_text_focus_style(text_focus_style);
  auto text_disabled_style = button->get_text_disabled_style();
  text_disabled_style.m_background_color = QColor("#00000000");
  text_disabled_style.m_text_color = QColor("#C8C8C8");
  button->set_text_disabled_style(text_disabled_style);
  auto style = button->get_style();
  style.m_background_color = QColor("#EBEBEB");
  style.m_borders = {{0, 0, 0, 0}};
  style.m_size = QSize(scale(180, 26));
  button->set_style(style);
  auto hover_style = button->get_hover_style();
  hover_style.m_background_color = QColor("#4B23A0");
  hover_style.m_borders = {{0, 0, 0, 0}};
  button->set_hover_style(hover_style);
  auto focus_style = button->get_focus_style();
  focus_style.m_background_color = QColor("#EBEBEB");
  focus_style.m_border_color = QColor("#4B23A0");
  focus_style.m_borders = {{scale_width(1), scale_height(1), scale_width(1),
    scale_height(1)}};
  button->set_focus_style(focus_style);
  auto disabled_style = button->get_disabled_style();
  disabled_style.m_background_color = QColor("#EBEBEB");
  disabled_style.m_borders = {{0, 0, 0, 0}};
  button->set_disabled_style(disabled_style);
  return button;
}
