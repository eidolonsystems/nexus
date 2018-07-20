#include "spire/login/login_window.hpp"
#include <QHBoxLayout>
#include <QMovie>
#include <QVBoxLayout>
#include "spire/login/chroma_hash_widget.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"
#include "spire/ui/flat_button.hpp"
#include "spire/ui/icon_button.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

login_window::login_window(const std::string& version, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_is_dragging(false) {
  setFixedSize(scale(396, 358));
  m_shadow = std::make_unique<drop_shadow>(this);
  setObjectName("login_window");
  setStyleSheet(R"(
    #login_window {
      background-color: #4B23A0;
      border: 1px solid #321471;
    })");
  auto body_layout = new QVBoxLayout(this);
  body_layout->setContentsMargins({});
  body_layout->setSpacing(0);
  auto title_bar_layout = new QHBoxLayout();
  title_bar_layout->setContentsMargins({});
  title_bar_layout->setSpacing(0);
  title_bar_layout->addStretch(352);
  auto button_size = scale(32, 26);
  auto button_box = QRect(translate(11, 8), scale(10, 10));
  m_exit_button = new icon_button(
    imageFromSvg(":/icons/close-purple.svg", button_size, button_box),
    imageFromSvg(":/icons/close-red.svg", button_size, button_box), this);
  m_exit_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_exit_button->setFocusPolicy(Qt::NoFocus);
  m_exit_button->installEventFilter(this);
  m_exit_button->connect_clicked_signal([=] { window()->close(); });
  m_exit_button->set_hover_style("background-color: #401D8B;");
  title_bar_layout->addWidget(m_exit_button);
  title_bar_layout->setStretchFactor(m_exit_button, 32);
  body_layout->addLayout(title_bar_layout);
  body_layout->setStretchFactor(title_bar_layout, 2600);
  auto padding_layout = new QHBoxLayout();
  padding_layout->setContentsMargins({});
  padding_layout->setSpacing(0);
  padding_layout->addStretch(52);
  auto content_layout = new QVBoxLayout();
  content_layout->setContentsMargins({});
  content_layout->setSpacing(0);
  content_layout->addStretch(30);
  auto logo_layout = new QHBoxLayout();
  logo_layout->setContentsMargins({});
  logo_layout->setSpacing(0);
  logo_layout->addStretch(73);
  m_logo_widget = new QLabel(parent);
  m_logo_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto logo = new QMovie(":/icons/logo.gif", QByteArray(), this);
  logo->setScaledSize(scale(134, 50));
  m_logo_widget->setMovie(logo);
  logo->start();
  logo_layout->addWidget(m_logo_widget);
  logo_layout->setStretchFactor(m_logo_widget, 134);
  logo_layout->addStretch(73);
  content_layout->addLayout(logo_layout);
  content_layout->setStretchFactor(logo_layout, 50);
  content_layout->addStretch(23);
  m_status_label = new QLabel(this);
  m_status_label->setStyleSheet(QString(
    R"(color: #FAEB96;
       font-family: Roboto;
       font-size: %1px;
       qproperty-alignment: AlignCenter;)").arg(scale_height(12)));
  content_layout->addWidget(m_status_label);
  content_layout->setStretchFactor(m_status_label, 14);
  content_layout->addStretch(20);
  m_username_line_edit = new QLineEdit(this);
  connect(m_username_line_edit, &QLineEdit::textEdited,
    [=] {on_input_updated();});
  m_username_line_edit->installEventFilter(this);
  m_username_line_edit->setPlaceholderText(tr("Username"));
  m_username_line_edit->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_username_line_edit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %2px;
       padding-left: %1px;)")
    .arg(scale_width(10)).arg(scale_height(14)));
  content_layout->addWidget(m_username_line_edit);
  content_layout->setStretchFactor(m_username_line_edit, 30);
  content_layout->addStretch(15);
  auto password_layout = new QHBoxLayout();
  password_layout->setContentsMargins({});
  password_layout->setSpacing(0);
  m_password_line_edit = new QLineEdit(this);
  connect(m_password_line_edit, &QLineEdit::textEdited,
    [=] {on_input_updated();});
  connect(m_password_line_edit, &QLineEdit::textEdited,
    [=] {on_password_updated();});
  m_password_line_edit->installEventFilter(this);
  m_password_line_edit->setEchoMode(QLineEdit::Password);
  m_password_line_edit->setPlaceholderText(tr("Password"));
  m_password_line_edit->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_password_line_edit->setStyleSheet(QString(
    R"(background-color: white;
       border: 0px;
       font-family: Roboto;
       font-size: %2px;
       padding-left: %1px;)")
    .arg(scale_width(10)).arg(scale_height(14)));
  password_layout->addWidget(m_password_line_edit);
  password_layout->setStretchFactor(m_password_line_edit, 246);
  auto ch_outer_widget = new QWidget(this);
  ch_outer_widget->setContentsMargins(scale_width(2), scale_height(2),
    scale_width(2), scale_height(2));
  ch_outer_widget->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  ch_outer_widget->setStyleSheet("background-color: white;");
  auto ch_layout = new QHBoxLayout(ch_outer_widget);
  ch_layout->setContentsMargins({});
  ch_layout->setSpacing(0);
  m_chroma_hash_widget = new chroma_hash_widget(this);
  ch_layout->addWidget(m_chroma_hash_widget);
  password_layout->addWidget(ch_outer_widget);
  password_layout->setStretchFactor(ch_outer_widget, 34);
  password_layout->addStretch(1);
  content_layout->addLayout(password_layout);
  content_layout->setStretchFactor(password_layout, 30);
  content_layout->addStretch(30);
  auto button_layout = new QHBoxLayout();
  button_layout->setContentsMargins({});
  button_layout->setSpacing(0);
  auto build_label = new QLabel(QString(tr("Build ")) + version.c_str(), this);
  build_label->setStyleSheet(QString(
    R"(color: white;
       font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)));
  build_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  button_layout->addWidget(build_label);
  button_layout->setStretchFactor(build_label, 57);
  button_layout->addStretch(103);
  m_sign_in_button = new flat_button(tr("Sign In"), this);
  m_sign_in_button->connect_clicked_signal([=] {try_login();});
  m_sign_in_button->installEventFilter(this);
  m_sign_in_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  QFont button_default_font;
  button_default_font.setFamily("Roboto");
  button_default_font.setPixelSize(scale_height(14));
  button_default_font.setWeight(QFont::Bold);
  m_sign_in_button->setFont(button_default_font);
  auto button_default_style = m_sign_in_button->get_style();
  button_default_style.m_background_color = QColor("#684BC7");
  button_default_style.m_text_color = QColor("#E2E0FF");
  m_sign_in_button->set_style(button_default_style);
  auto button_hover_style = m_sign_in_button->get_hover_style();
  button_hover_style.m_background_color = QColor("#8D78EC");
  button_hover_style.m_text_color = QColor("#E2E0FF");
  m_sign_in_button->set_hover_style(button_hover_style);
  auto button_focused_style = m_sign_in_button->get_focus_style();
  button_focused_style.m_background_color = QColor("#684BC7");
  button_focused_style.m_border_color = QColor("#8D78EC");
  button_focused_style.m_text_color = QColor("#E2E0FF");
  m_sign_in_button->set_focus_style(button_focused_style);
  auto button_disabled_style = m_sign_in_button->get_disabled_style();
  button_disabled_style.m_background_color = QColor("#4B23A0");
  button_disabled_style.m_border_color = QColor("#684BC7");
  button_disabled_style.m_text_color = QColor("#8D78EC");
  m_sign_in_button->set_disabled_style(button_disabled_style);
  m_sign_in_button->setDisabled(true);
  button_layout->addWidget(m_sign_in_button);
  button_layout->setStretchFactor(m_sign_in_button, 120);
  content_layout->addLayout(button_layout);
  content_layout->setStretchFactor(button_layout, 30);
  content_layout->addStretch(48);
  padding_layout->addLayout(content_layout);
  padding_layout->setStretchFactor(content_layout, 280);
  padding_layout->addStretch(52);
  body_layout->addLayout(padding_layout);
  body_layout->setStretchFactor(padding_layout, 320);
  setTabOrder(m_username_line_edit, m_password_line_edit);
  setTabOrder(m_password_line_edit, m_sign_in_button);
  set_state(state::NONE);
}

login_window::~login_window() = default;

void login_window::set_state(state s) {
  switch(s) {
    case state::NONE: {
      reset_all();
      break;
    }
    case state::LOGGING_IN: {
      m_username_line_edit->setEnabled(false);
      m_password_line_edit->setEnabled(false);
      m_status_label->setText("");
      m_sign_in_button->set_text(tr("Cancel"));
      m_logo_widget->movie()->start();
      break;
    }
    case state::CANCELLING: {
      reset_all();
      s = state::NONE;
      break;
    }
    case state::INCORRECT_CREDENTIALS: {
      m_status_label->setText(tr("Incorrect username or password."));
      reset_visuals();
      break;
    }
    case state::SERVER_UNAVAILABLE: {
      m_status_label->setText(tr("Server is unavailable."));
      reset_visuals();
      break;
    }
  }
  m_state = s;
}

connection login_window::connect_login_signal(
    const login_signal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection login_window::connect_cancel_signal(
    const cancel_signal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void login_window::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    window()->close();
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_password_line_edit->hasFocus()) {
      if(!m_username_line_edit->text().isEmpty()) {
        try_login();
      }
    }
  } else if(m_password_line_edit->hasFocus()) {
    return;
  } else if(!m_username_line_edit->hasFocus() &&
      m_username_line_edit->text().isEmpty()) {
    m_username_line_edit->setText(event->text());
    m_username_line_edit->setFocus();
  }
}

void login_window::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_pos = event->globalPos();
  window()->move(window_pos);
}

void login_window::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
}

void login_window::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void login_window::reset_all() {
  m_status_label->setText("");
  reset_visuals();
}

void login_window::reset_visuals() {
  m_username_line_edit->setEnabled(true);
  m_password_line_edit->setEnabled(true);
  m_sign_in_button->setFocus();
  m_sign_in_button->set_text(tr("Sign In"));
  m_logo_widget->movie()->stop();
  m_logo_widget->movie()->jumpToFrame(0);
}

void login_window::try_login() {
  if(m_state != state::LOGGING_IN) {
    if(m_username_line_edit->text().isEmpty()) {
      set_state(state::INCORRECT_CREDENTIALS);
    } else {
      m_login_signal(m_username_line_edit->text().toStdString(),
        m_password_line_edit->text().toStdString());
      set_state(state::LOGGING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(state::CANCELLING);
  }
}

void login_window::on_input_updated() {
  if(!m_username_line_edit->text().isEmpty()) {
    m_sign_in_button->setEnabled(true);
  } else {
    m_sign_in_button->setEnabled(false);
  }
}

void login_window::on_password_updated() {
  m_chroma_hash_widget->set_text(m_password_line_edit->text());
}
