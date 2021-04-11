#include "Spire/Styles/ChildSelector.hpp"
#include <unordered_set>
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

ChildSelector::ChildSelector(Selector base, Selector child)
  : m_base(std::move(base)),
    m_child(std::move(child)) {}

const Selector& ChildSelector::get_base() const {
  return m_base;
}

const Selector& ChildSelector::get_child() const {
  return m_child;
}

bool ChildSelector::operator ==(const ChildSelector& selector) const {
  return m_base == selector.get_base() && m_child == selector.get_child();
}

bool ChildSelector::operator !=(const ChildSelector& selector) const {
  return !(*this == selector);
}

ChildSelector Spire::Styles::operator >(Selector base, Selector child) {
  return ChildSelector(std::move(base), std::move(child));
}

std::vector<Stylist*> Spire::Styles::select(const ChildSelector& selector,
    Stylist& source) {
  auto selection = std::unordered_set<Stylist*>();
  auto bases = select(selector.get_base(), source);
  for(auto base : bases) {
    for(auto& child : base->get_widget().children()) {
      if(child->isWidgetType()) {
        auto child_selection = select(selector.get_child(),
          find_stylist(*static_cast<QWidget*>(child)));
        selection.insert(child_selection.begin(), child_selection.end());
      }
    }
  }
  return std::vector(selection.begin(), selection.end());
}

std::vector<QWidget*> Spire::Styles::build_reach(
    const ChildSelector& selector, QWidget& source) {
  auto reach = std::unordered_set<QWidget*>();
  auto bases = build_reach(selector.get_base(), source);
  reach.insert(bases.begin(), bases.end());
  for(auto base : bases) {
    for(auto& child : base->children()) {
      if(child->isWidgetType()) {
        auto child_reach = build_reach(selector.get_child(),
          *static_cast<QWidget*>(child));
        reach.insert(child_reach.begin(), child_reach.end());
      }
    }
  }
  return std::vector(reach.begin(), reach.end());
}
