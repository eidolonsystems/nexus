#ifndef SPIRE_STYLES_FLIP_SELECTOR_HPP
#define SPIRE_STYLES_FLIP_SELECTOR_HPP
#include "Spire/Styles/Selector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Used to flip what elements are selected. */
  class FlipSelector {
    public:

      /**
       * Constructs a FlipSelector.
       * @param selector The selector to apply.
       */
      FlipSelector(Selector selector);

      /** Returns the selector. */
      const Selector& get_selector() const;

      bool operator ==(const FlipSelector& selector) const;

      bool operator !=(const FlipSelector& selector) const;

    private:
      Selector m_selector;
  };

  /**
   * Provides an operator for the FlipSelector.
   * @param selector The selector to apply.
   */
  FlipSelector operator +(Selector base);

  std::vector<Stylist*> select(const FlipSelector& selector, Stylist& source);

  std::vector<QWidget*> build_reach(
    const FlipSelector& selector, QWidget& source);
}

#endif
