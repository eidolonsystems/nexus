#include "Spire/Ui/ListView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto DEFAULT_GAP = 0;
  const auto DEFAULT_OVERFLOW_GAP = DEFAULT_GAP;
  const auto QUERY_TIMEOUT_MS = 500;

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(ListItemGap(scale_height(DEFAULT_GAP))).
      set(ListOverflowGap(scale_width(DEFAULT_OVERFLOW_GAP)));
    return style;
  }
}

ListView::ListView(std::shared_ptr<CurrentModel> current_model,
    std::shared_ptr<ArrayListModel> list_model,
    std::function<ListItem* (
      std::shared_ptr<ArrayListModel>, int index)> factory,
    QWidget* parent)
    : QWidget(parent),
      m_current_model(std::move(current_model)),
      m_list_model(std::move(list_model)),
      m_factory(std::move(factory)),
      m_direction(Qt::Vertical),
      m_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_selection_mode(SelectionMode::SINGLE),
      m_current_index(-1),
      m_previous_current_index(m_current_index),
      m_column_or_row_index(0),
      m_x(0),
      m_y(0) {
  set_style(*this, DEFAULT_STYLE());
  connect_style_signal(*this, [=] { update_layout(); });
  setFocusPolicy(Qt::StrongFocus);
  m_items.resize(m_list_model->get_size());
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    auto value = m_list_model->get<QString>(i);
    auto list_item = m_factory(m_list_model, i);
    m_items[i] = {list_item, value, connect_item_current(list_item, value),
      connect_item_submit(list_item, value)};
  }
  m_list_model_connection = m_list_model->connect_operation_signal(
    [=] (const ListModel::Operation& operation) {
      visit(operation,
        [&] (const ListModel::RemoveOperation& operation) {
          on_delete_item(operation.m_index);
        });
  });
  connect(&m_query_timer, &QTimer::timeout, this, [=] { m_query.clear(); });
}

const std::shared_ptr<ListView::CurrentModel>&
    ListView::get_current_model() const {
  return m_current_model;
}

const std::shared_ptr<ArrayListModel>& ListView::get_list_model() const {
  return m_list_model;
}

Qt::Orientation ListView::get_direction() const {
  return m_direction;
}

void ListView::set_direction(Qt::Orientation direction) {
  m_direction = direction;
  update_layout();
}

ListView::EdgeNavigation ListView::get_edge_navigation() const {
  return m_navigation;
}

void ListView::set_edge_navigation(EdgeNavigation navigation) {
  m_navigation = navigation;
}

ListView::Overflow ListView::get_overflow() const {
  return m_overflow;
}

void ListView::set_overflow(Overflow overflow) {
  m_overflow = overflow;
  update_layout();
}

ListView::SelectionMode ListView::get_selection_mode() const {
  return m_selection_mode;
}

void ListView::set_selection_mode(SelectionMode selection_mode) {
  m_selection_mode = selection_mode;
}

connection ListView::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void ListView::keyPressEvent(QKeyEvent* event) {
  if(m_selection_mode == SelectionMode::NONE || m_list_model->get_size() == 0) {
    QWidget::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
  case Qt::Key_Home:
  case Qt::Key_PageUp:
    update_current_item(0);
    update_x_y();
    break;
  case Qt::Key_End:
  case Qt::Key_PageDown:
    update_current_item(m_list_model->get_size() - 1);
    update_x_y();
    break;
  case Qt::Key_Down:
    if(m_direction == Qt::Vertical) {
      update_current_item(move_next());
      update_x_y();
    } else if(m_overflow == Overflow::WRAP) {
      auto row_height =
        layout()->itemAt(m_column_or_row_index)->geometry().height();
      if(m_y + row_height <
          layout()->itemAt(layout()->count() - 2)->geometry().bottom()) {
        m_y += row_height +
          layout()->itemAt(m_column_or_row_index + 1)->geometry().height();
        m_column_or_row_index += 2;
        select_nearest_item();
      } else {
        update_current_item(move_next());
        update_x_y();
      }
    }
    break;
  case Qt::Key_Up:
    if(m_direction == Qt::Vertical) {
      update_current_item(move_previous());
      update_x_y();
    } else if(m_overflow == Overflow::WRAP) {
      if(m_current_index < 0) {
        return;
      }
      if(m_y != rect().y()) {
        m_y -= layout()->itemAt(m_column_or_row_index - 2)->geometry().height() +
          layout()->itemAt(m_column_or_row_index - 1)->geometry().height();
        m_column_or_row_index -= 2;
        select_nearest_item();
      } else {
        update_current_item(move_previous());
        update_x_y();
      }
    }
    break;
  case Qt::Key_Left:
    if(m_direction == Qt::Horizontal) {
      update_current_item(move_previous());
      update_x_y();
    } else if(m_overflow == Overflow::WRAP) {
      if(m_x != rect().x()) {
        m_x -= layout()->itemAt(m_column_or_row_index - 2)->geometry().width() +
          layout()->itemAt(m_column_or_row_index - 1)->geometry().width();
        m_column_or_row_index -= 2;
        select_nearest_item();
      } else {
        update_current_item(move_previous());
        update_x_y();
      }
    }
    break;
  case Qt::Key_Right:
    if(m_direction == Qt::Horizontal) {
      update_current_item(move_next());
      update_x_y();
    } else if(m_overflow == Overflow::WRAP) {
      auto column_width =
        layout()->itemAt(m_column_or_row_index)->geometry().width();
      if(m_x + column_width <
          layout()->itemAt(layout()->count() - 2)->geometry().right()) {
        m_x += column_width +
          layout()->itemAt(m_column_or_row_index + 1)->geometry().width();
        m_column_or_row_index += 2;
        select_nearest_item();
      } else {
        update_current_item(move_next());
        update_x_y();
      }
    }
    break;
  default:
    auto key = event->text();
    if(!key.isEmpty() && key[0].isLetterOrNumber()) {
      m_query += key.toUpper();
      m_query_timer.start(QUERY_TIMEOUT_MS);
      query();
    }
    break;
  }
  QWidget::keyPressEvent(event);
}

void ListView::resizeEvent(QResizeEvent* event) {
  update_layout();
}

scoped_connection ListView::connect_item_current(ListItem* item,
    const QString& value) {
  return item->connect_current_signal([=] {
    if(m_selection_mode == SelectionMode::NONE) {
      return;
    }
    update_current_index(value);
    if(m_current_index == -1) {
      m_current_model->set_current(boost::none);
    } else {
      m_current_model->set_current(value);
    }
    });
}

scoped_connection ListView::connect_item_submit(ListItem* item,
    const QString& value) {
  return item->connect_submit_signal([=] {
    m_current_index = get_index_by_value(value);
    update_x_y();
    m_submit_signal(value);
  });
}

int ListView::get_index_by_value(const QString& value) {
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    if(m_list_model->get<QString>(i) == value) {
      return i;
    }
  }
  return -1;
}

int ListView::move_next() {
  if(m_navigation == EdgeNavigation::CONTAIN) {
    return std::min(m_current_index + 1, m_list_model->get_size() - 1);
  } else {
    return (m_current_index + 1) % m_list_model->get_size();
  }
}

int ListView::move_previous() {
  if(m_navigation == EdgeNavigation::CONTAIN) {
    return std::max(m_current_index - 1, 0);
  } else {
    auto index = (m_current_index - 1) % m_list_model->get_size();
    if(index < 0) {
      index = m_list_model->get_size() - 1;
    }
    return index;
  }
}

void ListView::on_delete_item(int index) {
  delete m_items[index].m_component;
  m_items.erase(std::next(m_items.begin(), index));
  if(m_current_index >= m_list_model->get_size()) {
    m_current_index = m_list_model->get_size() - 1;
  }
  update_current_item(m_current_index);
}

void ListView::update_column_row_index() {
  m_column_or_row_index = -1;
  if(auto list_view_layout = layout()) {
    for(auto i = 0; i < list_view_layout->count(); i += 2) {
      if(auto child_layout = list_view_layout->itemAt(i)->layout()) {
        if(child_layout->indexOf(m_items[m_current_index].m_component) >= 0) {
          m_column_or_row_index = i;
          break;
        }
      }
    }
  }
}

void ListView::update_current_item(int index) {
  if(m_selection_mode != SelectionMode::NONE && index != -1) {
    m_items[index].m_component->setFocus();
  }
}

void ListView::update_current_index(const QString& value) {
  m_current_index = get_index_by_value(value);
  if(m_previous_current_index == -1) {
    update_x_y();
  }
  m_previous_current_index = m_current_index;
}

void ListView::update_layout() {
  delete layout();
  if(m_items.empty()) {
    return;
  }
  auto gap = 0;
  auto overflow_gap = 0;
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  if(auto gap_property = Styles::find<ListItemGap>(block)) {
    stylist.evaluate(*gap_property, [&] (auto value) {
      gap = value;
    });
  }
  if(auto overflow_gap_property = Styles::find<ListOverflowGap>(block)) {
    stylist.evaluate(*overflow_gap_property, [&] (auto value) {
      overflow_gap = value;
    });
  }
  if(m_direction == Qt::Vertical) {
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    if(m_overflow == Overflow::NONE) {
      auto column_layout = new QVBoxLayout();
      column_layout->setSpacing(0);
      column_layout->setContentsMargins({});
      column_layout->addWidget(m_items[0].m_component);
      for(auto i = 1; i < m_list_model->get_size(); ++i) {
        column_layout->addSpacing(gap);
        column_layout->addWidget(m_items[i].m_component);
      }
      layout->addLayout(column_layout);
    } else {
      auto column_layout = new QVBoxLayout();
      column_layout->setSpacing(0);
      column_layout->setContentsMargins({});
      auto first_item = m_items[0].m_component;
      column_layout->addWidget(first_item);
      auto column_height = first_item->height();
      for(auto i = 1; i < m_list_model->get_size(); ++i) {
        auto item = m_items[i].m_component;
        column_height += item->height() + gap;
        if(column_height <= height()) {
          column_layout->addSpacing(gap);
          column_layout->addWidget(item);
        } else {
          column_layout->addStretch();
          layout->addLayout(column_layout);
          layout->addSpacing(overflow_gap);
          column_layout = new QVBoxLayout();
          column_layout->setSpacing(0);
          column_layout->setContentsMargins({});
          column_layout->addWidget(item);
          column_height = item->height();
        }
      }
      column_layout->addStretch();
      layout->addLayout(column_layout);
      layout->addStretch();
    }
  } else {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    if(m_overflow == Overflow::NONE) {
      auto row_layout = new QHBoxLayout();
      row_layout->setSpacing(0);
      row_layout->setContentsMargins({});
      row_layout->addWidget(m_items[0].m_component);
      for(auto i = 1; i < m_list_model->get_size(); ++i) {
        row_layout->addSpacing(gap);
        row_layout->addWidget(m_items[i].m_component);
      }
      layout->addLayout(row_layout);
    } else {
      auto row_layout = new QHBoxLayout();
      row_layout->setSpacing(0);
      row_layout->setContentsMargins({});
      auto first_item = m_items[0].m_component;
      row_layout->addWidget(first_item);
      auto row_width = first_item->width();
      for(auto i = 1; i < m_list_model->get_size(); ++i) {
        auto item = m_items[i].m_component;
        row_width += item->width() + gap;
        if(row_width <= width()) {
          row_layout->addSpacing(gap);
          row_layout->addWidget(item);
        } else {
          row_layout->addStretch();
          layout->addLayout(row_layout);
          layout->addSpacing(overflow_gap);
          row_layout = new QHBoxLayout();
          row_layout->setSpacing(0);
          row_layout->setContentsMargins({});
          row_layout->addWidget(item);
          row_width = item->width();
        }
      }
      row_layout->addStretch();
      layout->addLayout(row_layout);
      layout->addStretch();
    }
  }
  adjustSize();
  update_x_y();
}

void ListView::update_x_y() {
  if(m_current_index == -1) {
    return;
  }
  m_x = m_items[m_current_index].m_component->x();
  m_y = m_items[m_current_index].m_component->y();
  update_column_row_index();
}

void ListView::select_nearest_item() {
  auto column_layout = layout()->itemAt(m_column_or_row_index)->layout();
  auto pos = [=] {
    if(m_direction == Qt::Vertical) {
      return m_y;
    } else {
      return m_x;
    }
  }();
  auto min_offset = [=] {
  if(m_direction == Qt::Vertical) {
    return std::abs(column_layout->itemAt(0)->geometry().y() - pos);
  } else {
    return std::abs(column_layout->itemAt(0)->geometry().x() - pos);
  }
  }();
  auto index = 0;
  for(auto i = 2; i < column_layout->count(); i += 2) {
    if(!column_layout->itemAt(i)->widget()) {
      continue;
    }
    auto offset = [=] {
      if(m_direction == Qt::Vertical) {
        return std::abs(column_layout->itemAt(i)->geometry().y() - pos);
      } else {
        return std::abs(column_layout->itemAt(i)->geometry().x() - pos);
      }
    }();
    if(offset < min_offset) {
      index = i;
      min_offset = offset;
    }
  }
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    if(column_layout->itemAt(index)->widget() == m_items[i].m_component) {
      update_current_item(i);
      break;
    }
  }
}

void ListView::query() {
  if(m_query.isEmpty()) {
    return;
  }
  auto index = (m_current_index + 1) % m_list_model->get_size();
  while(index != m_current_index) {
    if(auto value = m_list_model->get<QString>(index).toUpper();
        value.startsWith(m_query)) {
      update_current_item(index);
      break;
    }
    index = (index + 1) % m_list_model->get_size();
  }
}
