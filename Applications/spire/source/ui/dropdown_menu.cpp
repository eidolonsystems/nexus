#include "spire/ui/dropdown_menu.hpp"
#include <QLayout>
#include <QPainter>
#include <QPaintEvent>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/dropdown_menu_item.hpp"
#include "spire/ui/dropdown_menu_list.hpp"
#include "spire/ui/ui.hpp"

using namespace Spire;

DropdownMenu::DropdownMenu(const std::initializer_list<QString>& items,
    QWidget* parent)
    : QWidget(parent),
      m_dropdown_image(imageFromSvg(":/icons/arrow-down.svg", scale(6, 4))) {
  if(items.size() > 0) {
    m_current_text = *items.begin();
  }
  setFocusPolicy(Qt::StrongFocus);
  m_menu_list = new DropdownMenuList(items, this);
  m_menu_list->connect_selected_signal([=] (auto& t) { on_item_selected(t); });
  m_menu_list->hide();
  window()->installEventFilter(this);
}

void DropdownMenu::set_items(const std::vector<QString>& items) {
  auto index = 0;
  for(auto i = 0; i < m_menu_list->layout()->count(); ++i) {
    auto t = static_cast<DropdownMenuItem*>(
      m_menu_list->layout()->itemAt(0)->widget())->text();
    if(t == m_current_text) {
      index = i;
    }
  }
  m_menu_list->set_items(items);
  if(m_menu_list->layout()->count() > 0) {
    m_current_text = items[index];
  }
  update();
}

const QString& DropdownMenu::get_text() const {
  return m_current_text;
}

bool DropdownMenu::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::Move) {
      if(m_menu_list->isVisible()) {
        move_menu_list();
      }
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_menu_list->hide();
    }
  }
  return false;
}

void DropdownMenu::focusOutEvent(QFocusEvent* event) {
  m_menu_list->hide();
  update();
}

void DropdownMenu::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
      event->key() == Qt::Key_Space) {
    on_clicked();
  }
}

void DropdownMenu::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    if(m_menu_list->isVisible()) {
      m_menu_list->hide();
    } else {
      on_clicked();
    }
  }
}

void DropdownMenu::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  if(hasFocus() || m_menu_list->hasFocus()) {
    painter.fillRect(event->rect(), QColor("#4B23A0"));
  } else {
    painter.fillRect(event->rect(), QColor("#C8C8C8"));
  }
  painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
  auto font = QFont("Roboto", scale_height(7), QFont::Normal);
  painter.setFont(font);
  auto metrics = QFontMetrics(font);
  painter.drawText(QPoint(scale_width(8),
    (height() / 2) + (metrics.ascent() / 2) - 1), m_current_text);
  painter.drawImage(
    QPoint(width() - (m_dropdown_image.width() + scale_width(8)),
    scale_height(11)), m_dropdown_image);
}

void DropdownMenu::move_menu_list() {
  auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    geometry().bottomLeft()).x();
  auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
    frameGeometry().bottomLeft()).y();
  m_menu_list->move(x_pos, y_pos + 1);
  m_menu_list->raise();
}

void DropdownMenu::on_clicked() {
  move_menu_list();
  m_menu_list->setFixedWidth(width());
  m_menu_list->show();
  m_menu_list->raise();
}

void DropdownMenu::on_item_selected(const QString& text) {
  m_menu_list->hide();
  m_current_text = text;
  update();
}
