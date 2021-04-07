#include "Spire/Styles/StyledWidget.hpp"

using namespace Spire;
using namespace Spire::Styles;

std::size_t StyledWidget::SelectorHash::operator ()(
    const Selector& selector) const {
  return selector.get_type().hash_code();
}

bool StyledWidget::SelectorEquality::operator ()(
    const Selector& left, const Selector& right) const {
  return left.is_match(right);
}

StyledWidget::StyledWidget(QWidget* parent, Qt::WindowFlags flags)
  : StyledWidget({}, parent, flags) {}

StyledWidget::StyledWidget(StyleSheet style, QWidget* parent,
  Qt::WindowFlags flags)
  : QWidget(parent, flags),
    m_style(std::move(style)),
    m_visibility(VisibilityOption::VISIBLE) {}

StyledWidget::~StyledWidget() {
  for(auto dependent : m_dependents) {
    dependent->apply(*this, {});
  }
  while(!m_destinations.empty()) {
    unpropagate_style(**m_destinations.begin());
  }
}

const StyleSheet& StyledWidget::get_style() const {
  return m_style;
}

void StyledWidget::set_style(const StyleSheet& style) {
  m_style = style;
  apply_rules();
}

bool StyledWidget::is_enabled(const Selector& selector) const {
  return m_enabled_selectors.find(selector) != m_enabled_selectors.end();
}

Block StyledWidget::compute_style() const {
  return compute_style(VoidSelector());
}

Block StyledWidget::compute_style(const Selector& element) const {
  auto block = Block();
  for(auto& entry : m_blocks) {
    merge(block, entry->m_block);
  }
  return block;
}

void StyledWidget::propagate_style(QWidget& widget) {
  m_destinations.insert(&widget);
  dynamic_cast<StyledWidget*>(&widget)->m_sources.insert(this);
}

void StyledWidget::unpropagate_style(QWidget& widget) {
  dynamic_cast<StyledWidget*>(&widget)->m_sources.erase(this);
  m_destinations.erase(&widget);
}

void StyledWidget::enable(const Selector& selector) {
  if(m_enabled_selectors.insert(selector).second) {
    apply_rules();
  }
}

void StyledWidget::disable(const Selector& selector) {
  auto i = m_enabled_selectors.find(selector);
  if(i != m_enabled_selectors.end()) {
    m_enabled_selectors.erase(i);
    apply_rules();
  }
}

void StyledWidget::apply_style() {
  auto style = compute_style();
  auto visibility_option = [&] {
    if(auto visibility = Spire::Styles::find<Visibility>(style)) {
      return visibility->get_expression().as<VisibilityOption>();
    }
    return VisibilityOption::VISIBLE;
  }();
  if(visibility_option != m_visibility) {
    if(visibility_option == VisibilityOption::VISIBLE) {
      show();
    } else if(visibility_option == VisibilityOption::NONE) {
      auto size = sizePolicy();
      size.setRetainSizeWhenHidden(false);
      setSizePolicy(size);
      hide();
    } else if(visibility_option == VisibilityOption::INVISIBLE) {
      auto size = sizePolicy();
      size.setRetainSizeWhenHidden(true);
      setSizePolicy(size);
      hide();
    }
    m_visibility = visibility_option;
  }
  update();
}

void StyledWidget::apply(const StyledWidget& source, Block block) {
  auto i = m_source_to_block.find(&source);
  if(i == m_source_to_block.end()) {
    auto entry = std::make_shared<BlockEntry>();
    entry->m_source = &source;
    entry->m_priority = 0;
    m_blocks.push_back(entry);
    i = m_source_to_block.insert(i, std::pair(&source, std::move(entry)));
  }
  i->second->m_block = std::move(block);
  apply_style();
}

void StyledWidget::apply_rules() {
  auto blocks = std::unordered_map<StyledWidget*, Block>();
  for(auto& rule : m_style.get_rules()) {
    auto selection = select(rule.get_selector(), *this);
    for(auto& selected : selection) {
      if(auto styled_widget = dynamic_cast<StyledWidget*>(selected)) {
        merge(blocks[styled_widget], rule.get_block());
      }
    }
  }
  auto previous_dependents = std::move(m_dependents);
  for(auto& block : blocks) {
    m_dependents.insert(block.first);
    block.first->apply(*this, std::move(block.second));
  }
  for(auto previous_dependent : previous_dependents) {
    if(m_dependents.find(previous_dependent) == m_dependents.end()) {
      previous_dependent->apply(*this, {});
    }
  }
}

std::vector<QWidget*> Spire::Styles::select(
    const Active& selector, QWidget& source) {
  if(source.isActiveWindow()) {
    return {&source};
  }
  return {};
}

std::vector<QWidget*> Spire::Styles::select(
    const Disabled& selector, QWidget& source) {
  if(!source.isEnabled()) {
    return {&source};
  }
  return {};
}

std::vector<QWidget*> Spire::Styles::select(
    const Hover& selector, QWidget& source) {
  if(source.isEnabled() &&
      source.rect().contains(source.mapFromGlobal(QCursor::pos()))) {
    return {&source};
  }
  return {};
}

std::vector<QWidget*> Spire::Styles::select(
    const Focus& selector, QWidget& source) {
  if(source.hasFocus()) {
    return {&source};
  }
  return {};
}
