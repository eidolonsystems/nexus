#ifndef SPIRE_STYLES_NOT_SELECTOR_HPP
#define SPIRE_STYLES_NOT_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget that isn't selected by a selector. */
  class NotSelector {
    public:

      /**
       * Constructs a NotSelector.
       * @param selector The selector to negate.
       */
      explicit NotSelector(Selector selector);

      /** Returns the selector being negated. */
      const Selector& get_selector() const;

    private:
      Selector m_selector;
  };

  /**
   * Provides an operator for negating a selector.
   * @param selector The selector to negate.
   */
  template<typename T,
    typename = std::enable_if_t<std::is_constructible_v<Selector, const T&>>>
  auto operator !(T selector) {
    return NotSelector(std::move(selector));
  }
}

#endif
