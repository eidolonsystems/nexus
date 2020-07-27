#include "Spire/Ui/DropDownList.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto BORDER_PADDING = 2;
}

DropDownList::DropDownList(std::vector<DropDownItem*> items,
    bool is_click_activated, QWidget* parent)
    : DropDownWindow(is_click_activated, parent),
      m_max_displayed_items(5) {
  m_scroll_area = new ScrollArea(this);
  m_scroll_area->setFocusProxy(parent);
  m_scroll_area->setWidgetResizable(true);
  auto main_widget = new QWidget(this);
  m_layout = new QVBoxLayout(main_widget);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  m_scroll_area->setWidget(main_widget);
  initialize_widget(m_scroll_area);
  set_items(items);
  parent->installEventFilter(this);
}



bool DropDownList::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      switch(e->key()) {
        case Qt::Key_Down:
          if(isVisible()) {
            focus_next();
            return true;
          }
          break;
        case Qt::Key_Up:
          if(isVisible()) {
            focus_previous();
            return true;
          }
          break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
          if(isVisible() && m_highlight_index) {
            on_item_selected(get_widget(*m_highlight_index)->get_value(),
              *m_highlight_index);
            return true;
          }
          break;
      }
    }
  }
  return DropDownWindow::eventFilter(watched, event);
}

void DropDownList::hideEvent(QHideEvent* event) {
  m_scroll_area->verticalScrollBar()->setValue(0);
  if(m_highlight_index) {
    auto item = m_layout->itemAt(*m_highlight_index);
    if(item != nullptr) {
      static_cast<DropDownItem*>(item->widget())->reset_highlight();
    }
    m_highlight_index = boost::none;
  }
}

void DropDownList::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Down:
      if(isVisible()) {
        focus_next();
      }
      break;
    case Qt::Key_Enter:
      if(m_highlight_index) {
        on_item_selected(get_widget(*m_highlight_index)->get_value(),
          *m_highlight_index);
      }
      break;
    case Qt::Key_Escape:
      hide();
      break;
    case Qt::Key_Up:
      if(isVisible()) {
        focus_previous();
      }
      break;
  }
  DropDownWindow::keyPressEvent(event);
}

connection DropDownList::connect_activated_signal(
    const ActivatedSignal::slot_type& slot) const {
  return m_activated_signal.connect(slot);
}

connection DropDownList::connect_highlighted_signal(
    const HighlightedSignal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

connection DropDownList::connect_index_selected_signal(
    const IndexSelectedSignal::slot_type& slot) const {
  return m_index_selected_signal.connect(slot);
}

connection DropDownList::connect_value_selected_signal(
    const ValueSelectedSignal::slot_type& slot) const {
  return m_value_selected_signal.connect(slot);
}

QVariant DropDownList::get_value(unsigned int index) {
  if(static_cast<unsigned int>(m_layout->count() - 1) < index) {
    return QVariant();
  }
  return get_widget(index)->get_value();
}

void DropDownList::insert_item(DropDownItem* item) {
  m_item_selected_connections.push_back(std::move(
    item->connect_selected_signal(
      [=, index = m_layout->count()] (auto value) {
        on_item_selected(std::move(value), index);
      })));
  m_layout->insertWidget(m_layout->count(), item);
  update_height();
}

unsigned int DropDownList::item_count() const {
  return m_layout->count();
}

void DropDownList::remove_item(unsigned int index) {
  if(index > static_cast<unsigned int>(m_layout->count() - 1)) {
    return;
  }
  m_item_selected_connections.pop_back();
  for(auto i = std::size_t(index); i < m_item_selected_connections.size();
      ++i) {
    m_item_selected_connections[i] =
      std::move(get_widget(i + 1)->connect_selected_signal(
      [=] (auto value) {
        on_item_selected(std::move(value), i);
      }));
  }
  auto layout_item = m_layout->takeAt(index);
  delete layout_item->widget();
  delete layout_item;
  update_height();
}

bool DropDownList::set_highlight(const QString& text) {
  for(auto i = unsigned int(0); i < item_count(); ++i) {
    if(m_item_delegate.displayText(get_value(i)).startsWith(text,
        Qt::CaseInsensitive)) {
      set_highlight(i);
      return true;
    }
  }
  return false;
}

void DropDownList::set_items(std::vector<DropDownItem*> items) {
  while(auto item = m_layout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  m_item_selected_connections.clear();
  for(auto i = std::size_t(0); i < items.size(); ++i) {
    m_layout->addWidget(items[i]);
    m_item_selected_connections.push_back(std::move(
      items[i]->connect_selected_signal([=] (auto value) {
        on_item_selected(std::move(value), i);
      })));
  }
  if(m_layout->count() > 0) {
    update_height();
    static_cast<DropDownItem*>(m_layout->itemAt(0)->widget())->set_highlight();
    m_highlight_index = 0;
  } else {
    hide();
  }
}

DropDownItem* DropDownList::get_widget(unsigned int index) {
  return static_cast<DropDownItem*>(m_layout->itemAt(index)->widget());
}

void DropDownList::focus_next() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(0);
    return;
  }
  set_highlight((*m_highlight_index) + 1);
}

void DropDownList::focus_previous() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(m_layout->count() - 1);
    return;
  }
  set_highlight((*m_highlight_index) - 1);
}

void DropDownList::set_highlight(int index) {
  if(m_highlight_index) {
    get_widget(*m_highlight_index)->reset_highlight();
  }
  if(index < 0) {
    index = m_layout->count() - 1;
  } else if(index > m_layout->count() - 1) {
    index = 0;
  }
  m_highlight_index = index;
  auto highlighted_widget = get_widget(*m_highlight_index);
  highlighted_widget->set_highlight();
  scroll_to_highlight();
  m_activated_signal(highlighted_widget->get_value());
}

void DropDownList::scroll_to_highlight() {
  if(m_highlight_index != m_layout->count() - 1) {
    m_scroll_area->ensureWidgetVisible(get_widget(*m_highlight_index), 0,
      0);
  } else {
    m_scroll_area->verticalScrollBar()->setValue(
      m_scroll_area->verticalScrollBar()->maximum());
  }
}

void DropDownList::update_height() {
  if(m_layout->count() == 0) {
    return;
  }
  setFixedHeight(std::min(m_max_displayed_items, m_layout->count()) *
    m_layout->itemAt(0)->widget()->height() + BORDER_PADDING);
}

void DropDownList::on_item_selected(QVariant value, unsigned int index) {
  m_index_selected_signal(index);
  m_value_selected_signal(std::move(value));
  hide();
}
