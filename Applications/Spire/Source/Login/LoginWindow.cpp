#include "Spire/Login/LoginWindow.hpp"
#include <QHBoxLayout>
#include <QMovie>
#include <QVBoxLayout>
#include "Spire/Login/ChromaHashWidget.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto BUTTON_SIZE() {
    static auto size = scale(32, 26);
    return size;
  }

  auto CLOSE_BUTTON_STYLE() {
    auto style = StyleSheet();
    style.get(Any() > Button::Body()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(Fill(QColor::fromRgb(0xE2, 0xE0, 0xFF)));
    style.get(!Active() > Button::Body()).
      set(Fill(QColor::fromRgb(0xBA, 0xB3, 0xD9)));
    style.get(Hover() > Button::Body()).
      set(BackgroundColor(QColor::fromRgb(0x32, 0x14, 0x71))).
      set(Fill(QColor::fromRgb(0xE6, 0x3F, 0x45)));
    return style;
  }

  auto INPUT_STYLE(StyleSheet style) {
    style.get(Any()).set(border(scale_width(0), QColor::fromRgb(0, 0, 0, 0)));
    auto font_style = Styles::find<TextStyle>(style.find(Any())->get_block());
    if(font_style) {
      auto font = font_style->get<Font>().get_expression().as<QFont>();
      font.setPixelSize(scale_height(14));
      style.get(Any()).set(text_style(font,
        font_style->get<TextColor>().get_expression().as<QColor>()));
    }
    return style;
  }
}

LoginWindow::LoginWindow(const std::string& version, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_is_dragging(false) {
  setWindowIcon(QIcon(":/Icons/taskbar_icons/spire.png"));
  setFixedSize(scale(396, 358));
  m_shadow = new DropShadow(this);
  setObjectName("LoginWindow");
  setStyleSheet(R"(
    #LoginWindow {
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
  m_close_button = make_icon_button(imageFromSvg(":/Icons/close.svg",
    BUTTON_SIZE()), this);
  set_style(*m_close_button, CLOSE_BUTTON_STYLE());
  m_close_button->setFixedSize(BUTTON_SIZE());
  m_close_button->setFocusPolicy(Qt::NoFocus);
  m_close_button->connect_clicked_signal([=] {
    window()->close();
  });
  title_bar_layout->addWidget(m_close_button);
  title_bar_layout->setStretchFactor(m_close_button, 32);
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
  auto logo = new QMovie(":/Icons/logo.gif", QByteArray(), this);
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
  m_status_label->setStyleSheet(QString(R"(
    QLabel {
      color: #FAEB96;
      font-family: Roboto;
      font-size: %1px;
      qproperty-alignment: AlignCenter;
    })").arg(scale_height(12)));
  content_layout->addWidget(m_status_label);
  content_layout->setStretchFactor(m_status_label, 14);
  content_layout->addStretch(20);
  m_username_text_box = new TextBox(this);
  m_username_text_box->get_model()->
    connect_current_signal([=] (const auto& current) {
      on_input_updated();
    });
  m_username_text_box->set_placeholder(tr("Username"));
  m_username_text_box->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  set_style(*m_username_text_box,
    INPUT_STYLE(get_style(*m_username_text_box)));
  content_layout->addWidget(m_username_text_box);
  content_layout->setStretchFactor(m_username_text_box, 30);
  content_layout->addStretch(15);
  auto password_layout = new QHBoxLayout();
  password_layout->setContentsMargins({});
  password_layout->setSpacing(0);
  m_password_line_edit = new QLineEdit(this);
  connect(m_password_line_edit, &QLineEdit::textEdited,
    [=] {on_input_updated();});
  connect(m_password_line_edit, &QLineEdit::textEdited,
    [=] {on_password_updated();});
  m_password_line_edit->setEchoMode(QLineEdit::Password);
  m_password_line_edit->setPlaceholderText(tr("Password"));
  m_password_line_edit->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_password_line_edit->setStyleSheet(QString(R"(
    QLineEdit {
      background-color: white;
      border: 0px;
      font-family: Roboto;
      font-size: %2px;
      padding-left: %1px;
    })").arg(scale_width(6)).arg(scale_height(14)));
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
  m_chroma_hash_widget = new ChromaHashWidget(this);
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
  auto build_label = new QLabel(QString(tr("Build ")) +
    QString::fromStdString(version), this);
  build_label->setStyleSheet(QString(R"(
    QLabel {
      color: white;
      font-family: Roboto;
      font-size: %1px;
    })").arg(scale_height(12)));
  build_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  button_layout->addWidget(build_label);
  button_layout->setStretchFactor(build_label, 57);
  button_layout->addStretch(103);
  m_sign_in_button = new FlatButton(tr("Sign In"), this);
  connect(m_sign_in_button, &FlatButton::clicked,
    this, &LoginWindow::try_login);
  m_sign_in_button->setSizePolicy(QSizePolicy::Expanding,
    QSizePolicy::Expanding);
  m_sign_in_button->set_font_properties(scale_height(14), QFont::Bold);
  m_sign_in_button->set_style(
    {QColor("#684BC7"), QColor("#684BC7"), QColor("#E2E0FF")},
    {QColor("#8D78EC"), QColor("#8D78EC"), QColor("#E2E0FF")},
    {QColor("#684BC7"), QColor("#8D78EC"), QColor("#E2E0FF")},
    {QColor("#4B23A0"), QColor("#684BC7"), QColor("#8D78EC")});
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
  setTabOrder(m_username_text_box, m_password_line_edit);
  setTabOrder(m_password_line_edit, m_sign_in_button);
  set_state(State::NONE);
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::set_state(State s) {
  switch(s) {
    case State::NONE: {
      reset_all();
      break;
    }
    case State::LOGGING_IN: {
      m_username_text_box->setEnabled(false);
      m_password_line_edit->setEnabled(false);
      m_status_label->setText("");
      m_sign_in_button->setText(tr("Cancel"));
      m_logo_widget->movie()->start();
      break;
    }
    case State::CANCELLING: {
      reset_all();
      s = State::NONE;
      break;
    }
    case State::INCORRECT_CREDENTIALS: {
      m_status_label->setText(tr("Incorrect username or password."));
      reset_visuals();
      break;
    }
    case State::SERVER_UNAVAILABLE: {
      m_status_label->setText(tr("Server is unavailable."));
      reset_visuals();
      break;
    }
  }
  m_state = s;
}

connection LoginWindow::connect_login_signal(
    const LoginSignal::slot_type& slot) const {
  return m_login_signal.connect(slot);
}

connection LoginWindow::connect_cancel_signal(
    const CancelSignal::slot_type& slot) const {
  return m_cancel_signal.connect(slot);
}

void LoginWindow::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    window()->close();
  } else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    if(m_password_line_edit->hasFocus()) {
      if(!m_username_text_box->get_model()->get_current().isEmpty()) {
        try_login();
      }
    }
  } else if(m_password_line_edit->hasFocus()) {
    return;
  } else if(!m_username_text_box->hasFocus() &&
      m_username_text_box->get_model()->get_current().isEmpty()) {
    m_username_text_box->get_model()->set_current(event->text());
    m_username_text_box->setFocus();
  }
}

void LoginWindow::mouseMoveEvent(QMouseEvent* event) {
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

void LoginWindow::mousePressEvent(QMouseEvent* event) {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_pos = event->globalPos();
}

void LoginWindow::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void LoginWindow::reset_all() {
  m_status_label->setText("");
  reset_visuals();
}

void LoginWindow::reset_visuals() {
  m_username_text_box->setEnabled(true);
  m_password_line_edit->setEnabled(true);
  m_sign_in_button->setFocus();
  m_sign_in_button->setText(tr("Sign In"));
  m_logo_widget->movie()->stop();
  m_logo_widget->movie()->jumpToFrame(0);
}

void LoginWindow::try_login() {
  if(m_state != State::LOGGING_IN) {
    if(m_username_text_box->get_model()->get_current().isEmpty()) {
      set_state(State::INCORRECT_CREDENTIALS);
    } else {
      m_login_signal(
        m_username_text_box->get_model()->get_current().toStdString(),
        m_password_line_edit->text().toStdString());
      set_state(State::LOGGING_IN);
    }
  } else {
    m_cancel_signal();
    set_state(State::CANCELLING);
  }
}

void LoginWindow::on_input_updated() {
  if(!m_username_text_box->get_model()->get_current().isEmpty()) {
    m_sign_in_button->setEnabled(true);
  } else {
    m_sign_in_button->setEnabled(false);
  }
}

void LoginWindow::on_password_updated() {
  m_chroma_hash_widget->set_text(m_password_line_edit->text());
}
