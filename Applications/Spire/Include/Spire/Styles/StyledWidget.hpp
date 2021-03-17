#ifndef SPIRE_STYLES_STYLED_WIDGET_HPP
#define SPIRE_STYLES_STYLED_WIDGET_HPP
#include <QWidget>
#include "Spire/Styles/AncestorSelector.hpp"
#include "Spire/Styles/AndSelector.hpp"
#include "Spire/Styles/ChildSelector.hpp"
#include "Spire/Styles/DescendantSelector.hpp"
#include "Spire/Styles/IsASelector.hpp"
#include "Spire/Styles/NotSelector.hpp"
#include "Spire/Styles/OrSelector.hpp"
#include "Spire/Styles/ParentSelector.hpp"
#include "Spire/Styles/PseudoElement.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/StyleSheet.hpp"

namespace Spire::Styles {

  /** Base class for a QWidget styled according to a StyleSheet. */
  class StyledWidget : public QWidget {
    public:

      /**
       * Constructs a StyledWidget with an empty StyleSheet.
       * @param parent The parent widget.
       * @param flags The initial window flags.
       */
      explicit StyledWidget(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /**
       * Constructs a StyledWidget.
       * @param style The initial StyleSheet.
       * @param parent The parent widget.
       * @param flags The initial window flags.
       */
      explicit StyledWidget(StyleSheet style, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      /** Returns the style. */
      const StyleSheet& get_style() const;

      /** Sets the style and update's the QWidget. */
      void set_style(const StyleSheet& style);

      /** Returns a Block containing this immediate widget's computed style. */
      Block compute_style() const;

      /** Returns a Block containing the computed style for a pseudo-element. */
      Block compute_style(const Selector& element) const;

    protected:

      /**
       * Tests if a Selector applies to this StyledWidget's pseudo-element.
       * @param element The pseudo-element to test.
       * @param selector The Selector to test.
       * @return <code>true</code> iff the <i>selector</i> applies to this
       *         StyledWidget's <i>element</i>.
       */
      virtual bool test_selector(const Selector& element,
        const Selector& selector) const;

      /** Indicates the StyleSheet has been updated. */
      virtual void style_updated();

    private:
      StyleSheet m_style;

      friend bool test_selector(const QWidget& widget, const Selector& element,
        const Selector& selector);
  };

  /**
   * Tests if a Selector applies to a QWidget.
   * @param widget The widget to test.
   * @param selector The Selector to test.
   * @return <code>true</code> iff the <i>selector</i> applies to <i>widget</i>.
   */
  bool test_selector(const QWidget& widget, const Selector& selector);

  /**
   * Tests if a Selector applies to a QWidget's pseudo-element.
   * @param widget The widget to test.
   * @param element The pseudo-element to select.
   * @param selector The Selector to test.
   * @return <code>true</code> iff the <i>selector</i> applies to the
   *         <i>widget</i>'s pseudo-element.
   */
  bool test_selector(const QWidget& widget, const Selector& element,
    const Selector& selector);
}

#endif
