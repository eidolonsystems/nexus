#include "Spire/Ui/ColorSelectorHexInputWidget.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QRegExpValidator>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto FONT_HEIGHT() {
    static auto height = scale_height(12);
    return height;
  }

  auto color_name(const QColor& color) {
    return color.name().toUpper().remove('#');
  }
}

ColorSelectorHexInputWidget::ColorSelectorHexInputWidget(
    const QColor& current_color, QWidget* parent)
    : QWidget(parent),
      m_color_name(color_name(current_color)) {
  setObjectName("hex_input_widget");
  setStyleSheet("#hex_input_widget { background-color: white; }");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto hex_label = new QLabel(tr("HEX"), this);
  auto label_style = QString(R"(
    QLabel {
      background-color: #FFFFFF;
      font-family: Roboto;
      font-size: %1px;
    })").arg(FONT_HEIGHT());
  hex_label->setStyleSheet(label_style);
  hex_label->setFixedSize(scale(29, 26));
  layout->addWidget(hex_label);
  auto hash_label = new QLabel("#", this);
  hash_label->setFixedSize(scale(14, 26));
  hash_label->setStyleSheet(label_style);
  layout->addWidget(hash_label);
  m_text_input = new TextInputWidget(m_color_name, this);
  m_text_input->setValidator(new QRegExpValidator(QRegExp("^[0-9a-fA-F]{1,6}$"),
    this));
  m_text_input->setFixedHeight(26);
  connect(m_text_input, &QLineEdit::textChanged, this,
    &ColorSelectorHexInputWidget::on_text_changed);
  m_text_input->installEventFilter(this);
  layout->addWidget(m_text_input);
  setFocusProxy(m_text_input);
}

void ColorSelectorHexInputWidget::set_color(const QColor& color) {
  if(color_name(color) != m_color_name) {
    m_color_name = color_name(color);
    m_text_input->setText(m_color_name);
    m_text_input->selectAll();
  }
}

connection ColorSelectorHexInputWidget::connect_changed_signal(
    const ColorSignal::slot_type& slot) const {
  return m_changed_signal.connect(slot);
}

connection ColorSelectorHexInputWidget::connect_selected_signal(
    const ColorSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool ColorSelectorHexInputWidget::eventFilter(QObject* watched,
    QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return ||
        e->key() == Qt::Key_Space) {
      auto text = m_text_input->text();
      if(text.length() == 6 || text.length() == 3) {
        auto color = QColor();
        color.setNamedColor(QString("#%1").arg(m_text_input->text()));
        m_color_name = color_name(color);
        if(text.length() == 3) {
          m_text_input->setText(m_color_name);
        }
        m_text_input->clearFocus();
        m_selected_signal(color);
      } else {
        m_text_input->setText(m_color_name);
      }
      return true;
    }
  } else if(event->type() == QEvent::FocusOut) {
    auto color = QColor(QString("#%1").arg(m_text_input->text()));
    if(!color.isValid()) {
      m_text_input->setText(m_color_name);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void ColorSelectorHexInputWidget::on_text_changed(const QString& text) {
  auto color = QColor(QString("#%1").arg(m_text_input->text()));
  m_text_input->setText(text.toUpper());
  if(color.isValid()) {
    m_changed_signal(color);
  }
}
