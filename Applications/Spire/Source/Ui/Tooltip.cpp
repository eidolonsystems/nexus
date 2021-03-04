#include "Spire/Ui/Tooltip.hpp"
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  const auto DEFAULT_SHOW_DELAY_MS = 500;
  const auto DROP_SHADOW_SIZE = 4;
  const auto DROP_SHADOW_COLOR = QColor(0, 0, 0, 89);
  const auto TOOLTIP_COLOR = QColor("#333333");

  auto ARROW_X_POSITION() {
    static auto x = scale_width(6);
    return x;
  }

  auto ARROW_SIZE() {
    static auto size = scale(14, 7);
    return size;
  }

  auto DROP_SHADOW_HEIGHT() {
    static auto height = scale_height(DROP_SHADOW_SIZE);
    return height;
  }

  auto DROP_SHADOW_WIDTH() {
    static auto width = scale_width(DROP_SHADOW_SIZE);
    return width;
  }

  auto Y_OFFSET() {
    static auto offset = scale_height(3);
    return offset;
  }

  const auto& QLABEL_TOOLTIP_STYLE() {
    static auto style = QString(R"(
      QLabel {
        background-color: transparent;
        color: #FFFFFF;
        font-family: Roboto;
        font-size: %1px;
        font-weight: 600;
        padding: %2px %3px;
      }
    )").arg(scale_height(10)).arg(scale_height(8)).arg(scale_width(8));
    return style;
  }
}

Tooltip::Tooltip(QWidget* body, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
        Qt::NoDropShadowWindowHint | Qt::WindowDoesNotAcceptFocus |
        Qt::MSWindowsFixedSizeDialogHint),
      m_body(body) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_TranslucentBackground);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(get_margins());
  layout->addWidget(body);
  m_show_timer.setInterval(DEFAULT_SHOW_DELAY_MS);
  m_show_timer.setSingleShot(true);
  connect(&m_show_timer, &QTimer::timeout, this, &Tooltip::on_show_timeout);
  m_body->installEventFilter(this);
  parent->installEventFilter(this);
}

bool Tooltip::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::Resize) {
      resize(m_body->size().grownBy(get_margins()));
    }
  } else if(watched == parentWidget()) {
    switch(event->type()) {
      case QEvent::HoverEnter:
        if(parentWidget()->isEnabled()) {
          m_show_timer.start();
        }
        break;
      case QEvent::MouseMove:
        if(!parentWidget()->rect().contains(
            static_cast<QMouseEvent*>(event)->pos())) {
          hide();
        }
        break;
      case QEvent::HoverLeave:
      case QEvent::WindowDeactivate:
        hide();
        break;
      case QEvent::ToolTip:
        return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void Tooltip::hideEvent(QHideEvent* event) {
  m_show_timer.stop();
  QWidget::hideEvent(event);
}

void Tooltip::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.fillRect(rect().marginsRemoved(get_margins()), TOOLTIP_COLOR);
  painter.fillPath(get_arrow_path(), TOOLTIP_COLOR);
}

QPainterPath Tooltip::get_arrow_path() const {
  auto path = QPainterPath();
  auto polygon = [&] () -> QPolygonF {
    auto margins = get_margins();
    auto left_x = ARROW_X_POSITION() + DROP_SHADOW_WIDTH();
    auto tip_x = left_x + (ARROW_SIZE().width() / 2);
    auto right_x = left_x + ARROW_SIZE().width();
    auto orientation = get_orientation();
    if(orientation == Orientation::TOP_LEFT ||
        orientation == Orientation::TOP_RIGHT) {
      return QVector<QPoint>({{left_x, height() - margins.bottom()},
        {tip_x, height() - Y_OFFSET()},
        {right_x, height() - margins.bottom()}});
    }
    return QVector<QPoint>({{left_x, margins.top()}, {tip_x, Y_OFFSET()},
      {right_x, margins.top()}});
  }();
  if(get_body_orientation() == BodyOrientation::LEFT) {
    polygon.translate(width() - (2 * (ARROW_X_POSITION() +
      DROP_SHADOW_WIDTH())) - ARROW_SIZE().width(), 0);
  }
  path.addPolygon(polygon);
  return path;
}

Tooltip::BodyOrientation Tooltip::get_body_orientation() const {
  auto parent_position = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto screen_geometry =
    get_current_screen(parent_position)->availableGeometry();
  if(parent_position.x() + width() >
      screen_geometry.x() + screen_geometry.width()) {
    return BodyOrientation::LEFT;
  }
  return BodyOrientation::RIGHT;
}

QScreen* Tooltip::get_current_screen(const QPoint& point) const {
  if(auto screen = QGuiApplication::screenAt(point)) {
    return screen;
  }
  return parentWidget()->screen();
}

QMargins Tooltip::get_margins() const {
  auto orientation = get_orientation();
  if(orientation == Orientation::TOP_LEFT ||
      orientation == Orientation::TOP_RIGHT) {
    return {DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT(), DROP_SHADOW_WIDTH(),
      Y_OFFSET() + ARROW_SIZE().height()};
  }
  return {DROP_SHADOW_WIDTH(), Y_OFFSET() + ARROW_SIZE().height(),
    DROP_SHADOW_WIDTH(), DROP_SHADOW_HEIGHT()};
}

Tooltip::Orientation Tooltip::get_orientation() const {
  auto parent_position = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto screen_geometry =
    get_current_screen(parent_position)->availableGeometry();
  if((parent_position.y() + height()) >
      screen_geometry.y() + screen_geometry.height()) {
    if(parent_position.x() < screen_geometry.x()) {
      return Orientation::TOP_RIGHT;
    }
    return Orientation::TOP_LEFT;
  } else if(parent_position.x() < screen_geometry.x()) {
    return Orientation::BOTTOM_RIGHT;
  }
  return Orientation::BOTTOM_LEFT;
}

QPoint Tooltip::get_position() const {
  auto parent_pos = parentWidget()->mapToGlobal(
    parentWidget()->rect().bottomLeft());
  auto orientation = get_orientation();
  auto x = [&] {
    if(orientation == Orientation::BOTTOM_LEFT ||
        orientation == Orientation::TOP_LEFT) {
      return parent_pos.x() - DROP_SHADOW_WIDTH();
    }
    return parent_pos.x() + (parentWidget()->width() -
      (2 * ARROW_X_POSITION()) - ARROW_SIZE().width() - DROP_SHADOW_WIDTH());
  }();
  auto y = [&] {
    if(orientation == Orientation::TOP_LEFT ||
        orientation == Orientation::TOP_RIGHT) {
      return parent_pos.y() - parentWidget()->height() - height() -
        scale_height(1);
    }
    return parent_pos.y() + scale_height(1);
  }();
  if(get_body_orientation() == BodyOrientation::LEFT) {
    x -= width() - ((2 * DROP_SHADOW_WIDTH()) + ARROW_SIZE().width() +
      (2 * ARROW_X_POSITION()));
  }
  return {x, y};
}

void Tooltip::on_show_timeout() {
  if(parentWidget()->underMouse()) {
    layout()->setContentsMargins(get_margins());
    adjustSize();
    move(get_position());
    show();
  }
}

Tooltip* Spire::make_text_tooltip(const QString& label, QWidget* parent) {
  auto body = new QLabel(label, parent);
  body->setStyleSheet(QLABEL_TOOLTIP_STYLE());
  auto tooltip = new Tooltip(body, parent);
  return tooltip;
}