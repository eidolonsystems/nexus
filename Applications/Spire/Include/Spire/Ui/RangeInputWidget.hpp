#ifndef SPIRE_RANGE_INPUT_WIDGET_HPP
#define SPIRE_RANGE_INPUT_WIDGET_HPP
#include <QWidget>
#include "Spire/Spire/RangeInputModel.hpp"
#include "Spire/Spire/Scalar.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Represents a widget with an editable input range.
  class RangeInputWidget : public QWidget {
    public:

      //! Signals that a value was modified.
      using ChangedSignal = Signal<void (Scalar value)>;

      //! Constructs a RangeInputWidget.
      /*
        \param model Model that defines the acceptable minimum and maximum
                     values.
        \param min_widget The widget to display the minimum value.
        \param max_widget The widget to display the maximum value.
        \param step The minimum step between consecutive input values.
        \param parent The parent widget.
      */
      RangeInputWidget(std::shared_ptr<RangeInputModel> model,
        ScalarWidget* min_widget, ScalarWidget* max_widget,
        Scalar step, QWidget* parent = nullptr);

      //! Connects a slot to the minimum value changed signal.
      boost::signals2::connection connect_min_changed_signal(
        const ChangedSignal::slot_type& slot) const;

      //! Connects a slot to the maximum value changed signal.
      boost::signals2::connection connect_max_changed_signal(
        const ChangedSignal::slot_type& slot) const;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable ChangedSignal m_min_changed_signal;
      mutable ChangedSignal m_max_changed_signal;
      std::shared_ptr<RangeInputModel> m_model;
      ScalarWidget* m_min_widget;
      ScalarWidget* m_max_widget;
      RangeInputSlider* m_slider;

      void on_min_edited(Scalar value);
      void on_max_edited(Scalar value);
      void on_min_handle_moved(Scalar value);
      void on_max_handle_moved(Scalar value);
  };
}

#endif
