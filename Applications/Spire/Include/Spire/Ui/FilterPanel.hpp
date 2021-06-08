#ifndef SPIRE_FILTER_PANEL_HPP
#define SPIRE_FILTER_PANEL_HPP
#include "Spire/Ui/OverlayPanel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** 
   * Represents a panel to display inputs for filtering values.
   */
  class FilterPanel : public QWidget {
    public:

      /** Signals that the filter is reset to the default values. */
      using ResetSignal = Signal<void ()>;

      /**
       * Constructs a FilterPanel.
       * @param title The title of the FilterPanel.
       * @param component The filter component displayed in the FilterPanel.
       * @param parent The parent widget that shows the panel.
       */
      FilterPanel(const QString& title, QWidget* component,
        QWidget* parent);

      /** Returns the component displayed in the panel. */
      const QWidget& get_component() const;

      /** Returns the component displayed in the panel. */
      QWidget& get_component();

      /** Connects a slot to the reset signal. */
      boost::signals2::connection connect_reset_signal(
        const ResetSignal::slot_type& slot) const;

    protected:
      bool event(QEvent* event) override;

    private:
      mutable ResetSignal m_reset_signal;
      QWidget* m_component;
      OverlayPanel* m_panel;
  };
}

#endif
