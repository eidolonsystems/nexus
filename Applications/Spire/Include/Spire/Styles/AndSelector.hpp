#ifndef SPIRE_STYLES_AND_SELECTOR_HPP
#define SPIRE_STYLES_AND_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget that is selected by two selectors. */
  class AndSelector {
    public:

      /**
       * Constructs an AndSelector.
       * @param left The left hand selector to match.
       * @param right The right hand selector to match.
       */
      AndSelector(Selector left, Selector right);

      /** Returns the left hand selector. */
      const Selector& get_left() const;

      /** Returns the right hand selector. */
      const Selector& get_right() const;

      bool operator ==(const AndSelector& selector) const;

      bool operator !=(const AndSelector& selector) const;

    private:
      Selector m_left;
      Selector m_right;
  };

  /**
   * Provides an operator for the AndSelector.
   * @param left The left hand selector.
   * @param right The right hand selector.
   */
  AndSelector operator &&(Selector left, Selector right);

  std::unordered_set<Stylist*> select(
    const AndSelector& selector, std::unordered_set<Stylist*> sources);
}

#endif
