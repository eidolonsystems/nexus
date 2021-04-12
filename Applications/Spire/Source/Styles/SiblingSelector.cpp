#include "Spire/Styles/SiblingSelector.hpp"
#include <unordered_set>
#include <QLayout>
#include <QWidget>
#include "Spire/Styles/DisambiguateSelector.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace Spire;
using namespace Spire::Styles;

SiblingSelector::SiblingSelector(Selector base, Selector sibling)
  : m_base(std::move(base)),
    m_sibling(std::move(sibling)) {}

const Selector& SiblingSelector::get_base() const {
  return m_base;
}

const Selector& SiblingSelector::get_sibling() const {
  return m_sibling;
}

bool SiblingSelector::operator ==(const SiblingSelector& selector) const {
  return m_base == selector.get_base() && m_sibling == selector.get_sibling();
}

bool SiblingSelector::operator !=(const SiblingSelector& selector) const {
  return !(*this == selector);
}

SiblingSelector Spire::Styles::operator %(Selector base, Selector sibling) {
  return SiblingSelector(std::move(base), std::move(sibling));
}

std::vector<Stylist*> Spire::Styles::select(
    const SiblingSelector& selector, Stylist& source) {
  if(!source.get_widget().parentWidget()) {
    return {};
  }
  auto bases = select(selector.get_base(), source);
  auto is_disambiguated = selector.get_base().get_type() ==
    typeid(DisambiguateSelector);
  auto selection = std::unordered_set<Stylist*>();
  for(auto base : bases) {
    auto siblings = source.get_widget().parent()->children();
    auto i = 0;
    while(i != siblings.size()) {
      auto child = siblings[i];
      if(child != &base->get_widget()) {
        if(auto sibling = qobject_cast<QWidget*>(child)) {
          auto sibling_selection = select(selector.get_sibling(),
            find_stylist(*sibling));
          if(!sibling_selection.empty()) {
            if(is_disambiguated) {
              selection.insert(base);
              break;
            } else {
              selection.insert(
                sibling_selection.begin(), sibling_selection.end());
            }
          }
        } else if(auto layout = qobject_cast<QLayout*>(child)) {
          siblings.append(layout->children());
        }
      }
      ++i;
    }
  }
  return std::vector(selection.begin(), selection.end());
}
