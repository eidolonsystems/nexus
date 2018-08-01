#ifndef BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_LEVEL_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "spire/book_view/book_view.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays/modifies the properties used to display a book view's price
  //! levels.
  class BookViewLevelPropertiesWidget : public QWidget {
    public:

      //! Constructs a BookViewLevelPropertiesWidget.
      /*!
        \param properties The properties to display.
        \param parent The parent widget.
      */
      BookViewLevelPropertiesWidget(const BookViewProperties& properties,
        QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(BookViewProperties& properties) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      QListWidget* m_band_list_widget;
      QString m_band_list_stylesheet;
      flat_button* m_band_color_button;
      flat_button* m_gradient_start_button;
      flat_button* m_gradient_end_button;
      check_box* m_show_grid_lines_check_box;

      void set_color_button_stylesheet(flat_button* button,
        const QColor& color);
      void update_band_list_font(const QFont& font);
      void update_band_list_gradient();
      void update_band_list_stylesheet(int index);
      void on_band_color_button_clicked();
      void on_change_font_button_clicked();
      void on_gradient_apply_button_clicked();
      void on_gradient_end_button_clicked();
      void on_gradient_start_button_clicked();
      void on_number_of_bands_spin_box_changed(int value);
  };
}

#endif
