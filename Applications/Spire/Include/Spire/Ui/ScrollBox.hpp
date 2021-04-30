#ifndef SPIRE_SCROLL_BOX_HPP
#define SPIRE_SCROLL_BOX_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * A widget that serves as a container for another widget, with scroll bars
   * displayed on overflow.
   */
  class ScrollBox : public QWidget {
    public:

      /** Specifies when a scroll bar is displayed. */
      enum class DisplayPolicy {

        /** Scroll bars are never displayed. */
        NEVER,

        /** Scroll bars are always displayed. */
        ALWAYS,

        /**
         * Scroll bars are displayed only when the body overflows the ScrollBox.
         */
        ON_OVERFLOW,

        /**
         * Scroll bars are displayed only when the user scrolls or hovers over
         * the ScrollBox.
         */
        ON_ENGAGE
      };

      /**
       * Constructs a ScrollBox.
       * @param body The widget to display.
       * @param parent The parent widget.
       */
      explicit ScrollBox(QWidget* body, QWidget* parent = nullptr);

      /** Returns the horizontal display policy. */
      DisplayPolicy get_horizontal_display_policy() const;

      /** Sets the horizontal display policy. */
      void set_horizontal(DisplayPolicy policy);

      /** Returns the vertical display policy. */
      DisplayPolicy get_vertical_display_policy() const;

      /** Sets the vertical display policy. */
      void set_vertical(DisplayPolicy policy);

      /**
       * Sets both the horizontal and vertical display policies to the same
       * value.
       */
      void set(DisplayPolicy policy);

      /** Sets the horizontal and vertical display policies. */
      void set(DisplayPolicy horizontal_policy, DisplayPolicy vertical_policy);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_body;
      DisplayPolicy m_horizontal_display_policy;
      DisplayPolicy m_vertical_display_policy;
      ScrollableLayer* m_scrollable_layer;
      LayeredWidget* m_layers;

      void on_vertical_scroll(int position);
      void on_horizontal_scroll(int position);
      void update_ranges();
  };
}

#endif