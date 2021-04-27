#ifndef SPIRE_CHECKBOX_HPP
#define SPIRE_CHECKBOX_HPP
#include <QHBoxLayout>
#include <QWidget>
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects a checked Checkbox. */
  using Checked = StateSelector<void, struct CheckedTag>;
}

  //! Represents a Spire-styled checkbox.
  class Checkbox : public QWidget {
    public:

      /**
      * Signals that the checked state has changed.
      * @param is_checked True iff the Checkbox is checked.
      */
      using CheckedSignal = Signal<void (bool is_checked)>;

      //! Constructs a Checkbox without a label.
      /*!
        \param parent The parent widget.
      */
      explicit Checkbox(QWidget* parent = nullptr);
  
      //! Constructs a Checkbox with a label.
      /*!
        \param label The text to display next to the check box.
        \param parent The parent widget.
      */
      explicit Checkbox(const QString& label, QWidget* parent = nullptr);
  
      //! Sets the checked state of the Checkbox.
      /*!
        \param is_checked True iff the CheckBox is checked.
      */
      void set_checked(bool is_checked);

      //! Sets the text of the label.
      /*!
        \param label The label's text.
      */
      void set_label(const QString& label);

      //! Sets the read-only state of the Checkbox.
      /*!
        \param is_read_only Sets the Checkbox to read-only iff is_read_only is
                true.
      */
      void set_read_only(bool is_read_only);

      QSize sizeHint() const override;

      //! Connects a slot to the checked signal.
      boost::signals2::connection connect_checked_signal(
        const CheckedSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;

    private:
      mutable CheckedSignal m_checked_signal;
      Icon* m_check;
      TextBox* m_label;
      QHBoxLayout* m_body_layout;
      bool m_is_checked;
      bool m_is_read_only;
  };
}

#endif
