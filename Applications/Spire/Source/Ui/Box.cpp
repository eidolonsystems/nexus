#include "Spire/Ui/Box.hpp"
#include <array>
#include <QTimer>
#include <QTimeLine>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(horizontal_padding(scale_width(8)));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  const auto WARNING_FRAME_COUNT = 10;

  auto get_fade_out_step(int start, int end) {
    return (end - start) / WARNING_FRAME_COUNT;
  }

  auto get_color_step(const QColor& start_color, const QColor& end_color) {
    return std::array{get_fade_out_step(start_color.red(), end_color.red()),
      get_fade_out_step(start_color.green(), end_color.green()),
      get_fade_out_step(start_color.blue(), end_color.blue())};
  }

  auto get_fade_out_color(const QColor& color, const std::array<int, 3>& steps,
      int frame) {
    return QColor(color.red() + steps[0] * frame,
      color.green() + steps[1] * frame, color.blue() + steps[2] * frame);
  }
}

BorderSize Spire::Styles::border_size(Expression<int> size) {
  return BorderSize(size, size, size, size);
}

BorderColor Spire::Styles::border_color(Expression<QColor> color) {
  return BorderColor(color, color, color, color);
}

Border Spire::Styles::border(Expression<int> size, Expression<QColor> color) {
  return Border(border_size(size), border_color(color));
}

HorizontalPadding Spire::Styles::horizontal_padding(int size) {
  return HorizontalPadding(PaddingRight(size), PaddingLeft(size));
}

Box::Box(QWidget* body, QWidget* parent)
  : StyledWidget(parent),
    m_body(body) {
  setObjectName("Box");
  set_style(DEFAULT_STYLE());
  if(m_body) {
    m_body->setParent(this);
    setFocusProxy(m_body);
  }
}

void Box::enterEvent(QEvent* event) {
  update();
}

void Box::leaveEvent(QEvent* event) {
  update();
}

void Box::paintEvent(QPaintEvent* event) {
  auto computed_style = compute_style();
  auto style = QString("#Box {");
  style += "border-style: solid;";
  auto body_geometry = QRect(0, 0, width(), height());
  for(auto& property : computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        style += "background-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderTopSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-top-width: " + QString::number(computed_size) + "px;";
        body_geometry.setTop(body_geometry.top() + computed_size);
      },
      [&] (const BorderRightSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-right-width: " + QString::number(computed_size) +
          "px;";
        body_geometry.setRight(body_geometry.right() - computed_size);
      },
      [&] (const BorderBottomSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-bottom-width: " + QString::number(computed_size) +
          "px;";
        body_geometry.setBottom(body_geometry.bottom() - computed_size);
      },
      [&] (const BorderLeftSize& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "border-left-width: " + QString::number(computed_size) + "px;";
        body_geometry.setLeft(body_geometry.left() + computed_size);
      },
      [&] (const BorderTopColor& color) {
        style += "border-top-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderRightColor& color) {
        style += "border-right-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderBottomColor& color) {
        style += "border-bottom-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderLeftColor& color) {
        style += "border-left-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const PaddingTop& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-top: " + QString::number(computed_size) + "px;";
        body_geometry.setTop(body_geometry.top() + computed_size);
      },
      [&] (const PaddingRight& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-right: " + QString::number(computed_size) + "px;";
        body_geometry.setRight(body_geometry.right() - computed_size);
      },
      [&] (const PaddingBottom& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-bottom: " + QString::number(computed_size) + "px;";
        body_geometry.setBottom(body_geometry.bottom() - computed_size);
      },
      [&] (const PaddingLeft& size) {
        auto computed_size = size.get_expression().as<int>();
        style += "padding-left: " + QString::number(computed_size) + "px;";
        body_geometry.setLeft(body_geometry.left() + computed_size);
      },
      [] {});
  }
  style += "}";
  if(style != styleSheet()) {
    setStyleSheet(style);
    if(m_body) {
      m_body->setGeometry(body_geometry);
    }
  }
  StyledWidget::paintEvent(event);
}

void Spire::display_warning_indicator(StyledWidget& widget) {
  const auto WARNING_DURATION = 300;
  const auto WARNING_FADE_OUT_DELAY = 250;
  const auto WARNING_BACKGROUND_COLOR = QColor("#FFF1F1");
  const auto WARNING_BORDER_COLOR = QColor("#B71C1C");
  auto time_line = new QTimeLine(WARNING_DURATION, &widget);
  time_line->setFrameRange(0, WARNING_FRAME_COUNT);
  time_line->setEasingCurve(QEasingCurve::Linear);
  auto computed_style = widget.compute_style();
  auto computed_background_color = [&] {
    if(auto color = find<BackgroundColor>(computed_style)) {
      return color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0, 0, 0, 0);
  }();
  auto background_color_step =
    get_color_step(WARNING_BACKGROUND_COLOR, computed_background_color);
  auto computed_border_color = [&] {
    if(auto border_color = find<BorderTopColor>(computed_style)) {
      return border_color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0, 0, 0, 0);
  }();
  auto border_color_step =
    get_color_step(WARNING_BORDER_COLOR, computed_border_color);
  QObject::connect(time_line, &QTimeLine::frameChanged,
    [=, &widget] (auto frame) {
      auto frame_background_color = get_fade_out_color(WARNING_BACKGROUND_COLOR,
        background_color_step, frame);
      auto frame_border_color =
        get_fade_out_color(WARNING_BORDER_COLOR, border_color_step, frame);
      auto animated_style = widget.get_style();
      animated_style.get(Any()).
        set(BackgroundColor(frame_background_color)).
        set(border_color(frame_border_color));
      widget.set_style(std::move(animated_style));
    });
  auto style = widget.get_style();
  QObject::connect(time_line, &QTimeLine::finished, [=, &widget] () mutable {
    widget.set_style(std::move(style));
    time_line->deleteLater();
  });
  auto animated_style = StyleSheet();
  animated_style.get(Any()) = style.get(Any());
  animated_style.get(Any()).set(BackgroundColor(WARNING_BACKGROUND_COLOR));
  animated_style.get(Any()).set(border_color(WARNING_BORDER_COLOR));
  widget.set_style(std::move(animated_style));
  QTimer::singleShot(WARNING_FADE_OUT_DELAY, &widget, [=] {
    time_line->start();
  });
}
