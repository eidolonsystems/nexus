#ifndef SPIRE_DROP_DOWN_MENU_LIST_HPP
#define SPIRE_DROP_DOWN_MENU_LIST_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! The list used by the DropDownMenu.
  class DropDownMenuList : public QWidget {
    public:

      //! Signals that an item was highlighted using the navigation keys.
      using HighlightedSignal = Signal<void (const QString& item)>;

      //! Signals that an item was selected from the list.
      using SelectedSignal = Signal<void (const QString& item)>;

      //! Constructs a DropDownMenuList with the specified items.
      /*!
        \param items The initial items in the list.
        \param parent The parent to the list.
      */
      DropDownMenuList(const std::vector<QString>& items,
        QWidget* parent = nullptr);

      //! Clears the items from the list and populates it with the specified
      //! items.
      /*!
        \param items The items to display in the menu.
      */
      void set_items(const std::vector<QString>& items);

      //! Returns the item in the list that follows the item specified.
      /*!
        \param text The specified item.
      */
      QString get_next(const QString& text);

      //! Returns the item in the list that comes before the item specified.
      /*!
        \param text The specified item.
      */
      QString get_previous(const QString& text);

      //! Connects a slot to the item highlighted signal.
      boost::signals2::connection connect_highlighted_signal(
        const SelectedSignal::slot_type& slot) const;

      //! Connects a slot to the item selection signal.
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable HighlightedSignal m_highlighted_signal;
      mutable SelectedSignal m_selected_signal;
      DropShadow* m_shadow;
      ScrollArea* m_scroll_area;
      QWidget* m_list_widget;
      int m_highlight_index;

      int get_index(const QString& text);
      void on_select(const QString& text);
      void focus_next();
      void focus_previous();
      void update_highlights(int old_index, int new_index);
  };
}

#endif
