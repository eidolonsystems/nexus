#ifndef SPIRE_TIME_AND_SALES_PROPERTIES_DIALOG_HPP
#define SPIRE_TIME_AND_SALES_PROPERTIES_DIALOG_HPP
#include <QDialog>
#include <QListWidget>
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"
#include "spire/ui/dialog.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays the properties for a time and sales window.
  class TimeAndSalesPropertiesDialog : public Dialog {
    public:

      //! Signals that properties should be applied to the parent window.
      using ApplySignal = Signal<
        void (const TimeAndSalesProperties& properties)>;

      //! Signals that properties should be applied to all windows.
      using ApplyAllSignal = Signal<
        void (const TimeAndSalesProperties& properties)>;

      //! Signals that properties should be saved as the default.
      using SaveDefaultSignal = Signal<
        void (const TimeAndSalesProperties& properties)>;

      //! Constructs a time and sales properties dialog.
      /*!
        \param properties The properties to modify.
        \param parent The parent widget.
      */
      explicit TimeAndSalesPropertiesDialog(
        const TimeAndSalesProperties& properties, QWidget* parent = nullptr);

      //! Returns the properties represented by this dialog.
      TimeAndSalesProperties get_properties() const;

      //! Connects a slot to the apply signal.
      boost::signals2::connection connect_apply_signal(
        const ApplySignal::slot_type& slot) const;

      //! Connects a slot to the apply all signal.
      boost::signals2::connection connect_apply_all_signal(
        const ApplyAllSignal::slot_type& slot) const;

      //! Connects a slot to the save default signal.
      boost::signals2::connection connect_save_default_signal(
        const SaveDefaultSignal::slot_type& slot) const;

    protected:
      void mousePressEvent(QMouseEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable ApplySignal m_apply_signal;
      mutable ApplyAllSignal m_apply_all_signal;
      mutable SaveDefaultSignal m_save_default_signal;
      TimeAndSalesProperties m_properties;
      QListWidget* m_band_list;
      FlatButton* m_band_color_button;
      FlatButton* m_text_color_button;
      CheckBox* m_show_grid_check_box;

      void set_band_color();
      void set_color_button_stylesheet(FlatButton* button,
        const QColor& color);
      void set_color_settings_stylesheet(int band_index);
      void set_font();
      void set_properties(const TimeAndSalesProperties& properties);
      void set_text_color();
      void update_band_list_stylesheet(int highlighted_band_index);
  };
}

#endif
