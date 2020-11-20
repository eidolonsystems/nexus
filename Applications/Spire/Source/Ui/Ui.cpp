#include "Spire/Ui/Ui.hpp"
#include <QIcon>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FlatButton.hpp"

using namespace Spire;

QImage Spire::imageFromSvg(const QString& path, const QSize& size) {
  return imageFromSvg(path, size, QRect(0, 0, size.width(), size.height()));
}

QImage Spire::imageFromSvg(const QString& path, const QSize& size,
    const QRect& box) {
  // TODO: Revert this when Qt fixes the regression.
  // https://bugreports.qt.io/browse/QTBUG-81259
  auto svg_pixmap = QIcon(path).pixmap(box.width(), box.height());
  auto image = QImage(size, QImage::Format_ARGB32);
  image.fill(QColor(0, 0, 0, 0));
  auto painter = QPainter(&image);
  painter.drawPixmap(box.topLeft(), svg_pixmap);
  return image;
}

QHeaderView* Spire::make_fixed_header(QWidget* parent) {
  auto header = new QHeaderView(Qt::Horizontal, parent);
  header->setFixedHeight(scale_height(30));
  header->setStretchLastSection(true);
  header->setSectionsClickable(false);
  header->setSectionsMovable(false);
  header->setSectionResizeMode(QHeaderView::Fixed);
  header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  header->setStyleSheet(QString(R"(
    QHeaderView::section {
      background-color: #FFFFFF;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}

QHeaderView* Spire::make_header(QWidget* parent) {
  auto header = make_fixed_header(parent);
  header->setSectionsMovable(true);
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setStyleSheet(QString(R"(
    QHeaderView {
      padding-left: %1px;
    }

    QHeaderView::section {
      background-color: #FFFFFF;
      background-image: url(:/Icons/column-border.png);
      background-position: left;
      background-repeat: repeat;
      border: none;
      color: #4B23A0;
      font-family: Roboto;
      font-size: %2px;
      font-weight: 550;
      padding-left: %1px;
      padding-right: %1px;
    }

    QHeaderView::section::first {
      background: none;
      background-color: #FFFFFF;
    })").arg(scale_width(8)).arg(scale_height(12)));
  return header;
}
