#ifndef SPIRE_STYLES_SIBLING_SELECTOR_HPP
#define SPIRE_STYLES_SIBLING_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget that is a sibling of another. */
  class SiblingSelector {
    public:

      /**
       * Constructs a SiblingSelector.
       * @param base The selector for the base widget.
       * @param sibling The selector for the sibling.
       */
      explicit SiblingSelector(Selector base, Selector sibling);

      /** Returns the base selector. */
      const Selector& get_base() const;

      /** Returns the sibling selector. */
      const Selector& get_sibling() const;

      bool is_match(const SiblingSelector& selector) const;

    private:
      Selector m_base;
      Selector m_sibling;
  };

  /**
   * Provides an operator for the SiblingSelector.
   * @param base The base selector.
   * @param sibling The sibling selector.
   */
  SiblingSelector operator %(Selector base, Selector sibling);
}

#endif
