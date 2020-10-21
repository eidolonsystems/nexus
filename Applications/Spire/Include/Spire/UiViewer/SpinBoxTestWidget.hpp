#ifndef SPIRE_SPIN_BOX_TEST_WIDGET_HPP
#define SPIRE_SPIN_BOX_TEST_WIDGET_HPP
#include <QGridLayout>
#include <QLabel>
#include "Spire/Ui/StaticDropDownMenu.hpp"
#include "Spire/Ui/TextInputWidget.hpp"
#include "Spire/UiViewer/SpinBoxAdapter.hpp"

namespace Spire {

  //! Represents a widget for testing Spire spin boxes.
  class SpinBoxTestWidget : public QWidget {
    public:

      //! Constructs a SpinBoxTestWidget.
      /*!
        \param spin_box The spin box to test.
        \param parent The parent widget.
      */
      explicit SpinBoxTestWidget(SpinBoxAdapter* spin_box,
        QWidget* parent = nullptr);

    private:
      boost::signals2::scoped_connection m_change_connection;
      QGridLayout* m_layout;
      SpinBoxAdapter* m_spin_box;
      QLabel* m_value_label;
      TextInputWidget* m_initial_input;
      TextInputWidget* m_min_input;
      TextInputWidget* m_max_input;
      TextInputWidget* m_increment_input;
      StaticDropDownMenu* m_modifier_menu;
      QLabel* m_no_increment_label;
      QLabel* m_shift_increment_label;
      QLabel* m_ctrl_increment_label;
      QLabel* m_ctrl_shift_increment_label;

      void reset_spin_box();
      void update_increment_labels();
      void on_initial_set();
      void on_min_set();
      void on_max_set();
      void on_increment_set();
  };
}

#endif
