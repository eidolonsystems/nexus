#include "Spire/Ui/TabWidget.hpp"
#include <QPainter>
#include <QTabBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

TabWidget::TabWidget(QWidget* parent)
  : TabWidget(PaddingStyle::NONE, parent) {}

TabWidget::TabWidget(PaddingStyle padding_style, QWidget* parent)
    : QTabWidget(parent) {
  tabBar()->setFixedHeight(scale_height(40));
  auto tab_bar_padding = [&] {
    if(padding_style == PaddingStyle::LEFT) {
      return scale_width(8);
    }
    return 0;
  }();
  setStyleSheet(QString(R"(
    QTabWidget::pane {
      border: none;
    }

    QTabBar {
      outline: none;
    }

    QTabBar::tab {
      background-color: #EBEBEB;
      font-family: Roboto;
      font-size: %1px;
      height: %2px;
      margin: %3px %4px %3px 0px;
      width: %5px;
    }

    QTabBar::tab:selected {
      background-color: #4B23A0;
      color: white;
    }

    QTabBar::tab:first {
      margin-left: %7px;
    }

    QTabBar::tab:focus {
      background-color: #EBEBEB;
      border: %6px solid #4B23A0;
      color: black;
      padding: -%6px 0px 0px -%6px;
    }

    QTabBar::tab:hover {
      background-color: #4B23A0;
      color: white;
    })").arg(scale_height(12)).arg(scale_height(20)).arg(scale_height(10))
        .arg(scale_width(2)).arg(scale_width(80)).arg(scale_width(1))
        .arg(tab_bar_padding));
}

void TabWidget::paintEvent(QPaintEvent* event) {
  QTabWidget::paintEvent(event);
  auto painter = QPainter(this);
  painter.setPen(QColor("#E0E0E0"));
  painter.drawLine(tabBar()->width(), tabBar()->height() - scale_height(11),
    width(), tabBar()->height() - scale_height(11));
}
