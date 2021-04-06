#ifndef SPIRE_DURATION_BOX_HPP
#define SPIRE_DURATION_BOX_HPP
#include "Spire/Styles/StyledWidget.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/DecimalBox.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/LocalDurationModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget for inputting a time duration.
  class DurationBox : public Styles::StyledWidget {
    public:

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal =
        Signal<void (boost::posix_time::time_duration value)>;

      //! Signals that submission value has changed.
      /*!
        \param value The submission value.
      */
      using SubmitSignal =
        Signal<void (boost::posix_time::time_duration value)>;

      //! Constructs a DurationBox.
      /*!
        \param parent The parent widget.
      */
      explicit DurationBox(QWidget* parent = nullptr);

      //! Constructs a DurationBox.
      /*!
        \param model The model used for the current value.
        \param parent The parent widget.
      */
      explicit DurationBox(std::shared_ptr<LocalDurationModel> model,
        QWidget* parent = nullptr);

      //! Returns the current value model.
      const std::shared_ptr<LocalDurationModel>& get_model() const;

      //! Returns whether a warning is displayed when a submission is rejected.
      bool is_warning_displayed() const;

      //! Sets whether a warning is displayed when a submission is rejected.
      /*!
        \param is_displayed True iff the warning style should be displayed.
      */
      void set_warning_displayed(bool is_displayed);

      //! Connects a slot to the RejectSignal.
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

       //! Connects a slot to the value submission signal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    private:
      class InternalBox;
      mutable RejectSignal m_reject_signal;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<LocalDurationModel> m_model;
      boost::posix_time::time_duration m_submission;
      InternalBox* m_box;
      IntegerBox* m_hour_field;
      IntegerBox* m_minute_field;
      DecimalBox* m_second_field;
      TextBox* m_colon1;
      TextBox* m_colon2;
      bool m_is_warning_displayed;
      bool m_is_hour_field_inputting;
      bool m_is_minute_field_inputting;
      bool m_is_second_field_inputting;

      void create_hour_field();
      void create_minute_field();
      void create_second_field();
      void create_colon_fields();
      void on_hour_field_current(int current);
      void on_minute_field_current(int current);
      void on_second_field_current(const DecimalBox::Decimal& current);
      void on_current(boost::posix_time::time_duration current);
      void on_submit();
      void on_reject();
  };
}

#endif
