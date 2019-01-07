#ifndef SPIRE_CHARTING_WINDOW_HPP
#define SPIRE_CHARTING_WINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/charting/charting.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a financial chart.
  class ChartingWindow : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param security The security to display.
      */
      using ChangeSecuritySignal =
        Signal<void (const Nexus::Security& security)>;

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a charting window.
      /*!
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      ChartingWindow(Beam::Ref<SecurityInputModel> input_model,
        QWidget* parent = nullptr);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;

    private:
      mutable ClosedSignal m_closed_signal;
      SecurityWidget* m_security_widget;
      QWidget* m_body;
      QWidget* m_button_header_widget;
      QLineEdit* m_period_line_edit;
      DropdownMenu* m_period_dropdown;

      void on_period_line_edit_changed();
  };
}

#endif
