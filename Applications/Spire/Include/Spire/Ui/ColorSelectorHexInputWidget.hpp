#ifndef SPIRE_COLOR_SELECTOR_HEX_INPUT_WIDGET_HPP
#define SPIRE_COLOR_SELECTOR_HEX_INPUT_WIDGET_HPP
#include <QLineEdit>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class ColorSelectorHexInputWidget : public QWidget {
    public:

      //! Signals that a color was selected.
      /*
        \param color The selected color.
      */
      using ColorSignal = Signal<void (const QColor& color)>;

      ColorSelectorHexInputWidget(const QColor& current_color,
        QWidget* parent = nullptr);

      void set_color(const QColor& color);

      //! Connects a slot to the color signal.
      boost::signals2::connection connect_color_signal(
        const ColorSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ColorSignal m_color_signal;
      QLineEdit* m_line_edit;
      QString m_color_name;

      void on_text_changed(const QString& text);
  };
}

#endif