#include "Spire/Ui/Checkbox.hpp"
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Ui.hpp"

using namespace Spire;

namespace {
  auto BOX_SIZE() {
    static auto size = scale(16, 16);
    return size;
  }

  auto PADDING() {
    static auto padding = scale_width(8);
    return padding;
  }
}

Checkbox::Checkbox(QWidget* parent)
  : QCheckBox("", parent) {}

Checkbox::Checkbox(const QString& label, QWidget* parent)
    : QCheckBox(label, parent),
      m_is_read_only(false),
      m_check_icon(imageFromSvg(":/Icons/check.svg", scale(10, 8))) {
  setAttribute(Qt::WA_Hover);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  setFont(font);
}

bool Checkbox::event(QEvent* event) {
  if(event->type() == QEvent::LayoutDirectionChange) {
    // TODO: add this as a test paramter to UiViewer.
    update();
  }
  return QCheckBox::event(event);
}

void Checkbox::keyPressEvent(QKeyEvent* event) {
  // TODO: add read-only as test parameter
  if(!m_is_read_only) {
    QCheckBox::keyPressEvent(event);
  }
}

void Checkbox::mousePressEvent(QMouseEvent* event) {
  if(!m_is_read_only) {
    QCheckBox::mousePressEvent(event);
  }
}

void Checkbox::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(!isEnabled() || m_is_read_only) {
    painter.fillRect(QRect(get_box_position(), BOX_SIZE()), "#F5F5F5");
  } else {
    painter.fillRect(QRect(get_box_position(), BOX_SIZE()), "#FFFFFF");
  }
  painter.setPen(get_box_color());
  painter.drawRect(QRect(get_box_position(), BOX_SIZE()));
  if(isChecked()) {
    auto icon = QPixmap::fromImage(m_check_icon);
    auto image_painter = QPainter(&icon);
    image_painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    image_painter.fillRect(icon.rect(), get_check_color());
    painter.drawPixmap(get_check_position(), icon);
  }
  painter.setPen("#000000");
  if(!text().isEmpty()) {
    painter.drawText(QRect(get_text_position(), size()), 0, text());
  }
}

void Checkbox::set_read_only(bool is_read_only) {
  if(is_read_only != m_is_read_only) {
    m_is_read_only = is_read_only;
    update();
  }
}

QPoint Checkbox::get_box_position() const {
  auto box_y = (height() - BOX_SIZE().height()) / 2;
  if(layoutDirection() == Qt::LeftToRight) {
    return {0, box_y};
  }
  return {width() - BOX_SIZE().width(), box_y};
}

QColor Checkbox::get_check_color() const {
  if(!isEnabled() || m_is_read_only) {
    return "#C8C8C8";
  }
  return "#4B23A0";
}

QPoint Checkbox::get_check_position() const {
  auto box_pos = get_box_position();
  return {box_pos.x() + scale_width(3), box_pos.y() + scale_height(4)};
}

QColor Checkbox::get_box_color() const {
  if(!isEnabled() || m_is_read_only || (!hasFocus() && !underMouse())) {
    return "#C8C8C8";
  }
  return "#4B23A0";
}

QPoint Checkbox::get_text_position() const {
  auto metrics = QFontMetrics(font());
  auto text_height = metrics.tightBoundingRect(text()).height();
  auto text_y = (height() - text_height) / 2;
  if(layoutDirection() == Qt::LeftToRight) {
    return {BOX_SIZE().width() + PADDING(), text_y};
  }
  return {0, text_y};
}
