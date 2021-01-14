#ifndef BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#define BOOK_VIEW_HIGHLIGHT_PROPERTIES_WIDGET_HPP
#include <QListWidget>
#include <QWidget>
#include "Spire/BookView/BookView.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays/modifies the properties used to highlight a book view's quotes.
  class BookViewHighlightPropertiesWidget : public QWidget {
    public:

      //! Constructs a BookViewHighlightPropertiesWidget.
      /*!
        \param properties The properties to display.
        \param parent The parent widget.
      */
      explicit BookViewHighlightPropertiesWidget(
        const BookViewProperties& properties, QWidget* parent = nullptr);

      //! Applies the properties represented by this widget to an instance.
      /*!
        \param properties The instance to apply the properties to.
      */
      void apply(BookViewProperties& properties) const;

    protected:
      void showEvent(QShowEvent* event) override;

    private:
      QListWidget* m_markets_list_widget;
      Checkbox* m_highlight_none_checkbox;
      Checkbox* m_highlight_top_level_checkbox;
      Checkbox* m_highlight_all_levels_checkbox;
      ColorSelectorButton* m_market_highlight_color_button;
      Checkbox* m_hide_orders_checkbox;
      Checkbox* m_display_orders_checkbox;
      Checkbox* m_highlight_orders_checkbox;
      ColorSelectorButton* m_order_highlight_color_button;

      void update_market_widgets();
      void update_market_list_stylesheet(int selected_item_index);
      void on_market_highlight_color_selected(const QColor& color);
      void on_highlight_none_checkbox_checked(int state);
      void on_highlight_top_level_checkbox_checked(int state);
      void on_highlight_all_levels_checkbox_checked(int state);
  };
}

#endif
