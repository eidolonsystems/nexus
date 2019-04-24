#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <QWidget>
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar.hpp"
#include "spire/toolbar/toolbar_menu.hpp"
#include "spire/ui/ui.hpp"
#include "spire/ui/window.hpp"

namespace Spire {

  //! Displays the toolbar window.
  class ToolbarWindow : public Window {
    public:

      //! Signals a window should be opened.
      /*!
        \param w The type of window to open.
      */
      using OpenSignal = Signal<void (RecentlyClosedModel::Type w)>;

      //! Signals that a recently closed window should be re-opened.
      using ReopenSignal = Signal<void (const RecentlyClosedModel::Entry& w)>;

      //! Constructs a ToolbarWindow.
      /*!
        \param model The model used to populate the recently closed menu.
        \param account The account that logged in.
        \param parent The parent widget to ToolbarWindow.
      */
      ToolbarWindow(Beam::Ref<RecentlyClosedModel> model,
        const Beam::ServiceLocator::DirectoryEntry& account,
        QWidget* parent = nullptr);

      //! Connects a slot to the open signal.
      boost::signals2::connection connect_open_signal(
        const OpenSignal::slot_type& slot) const;

      //! Connects a slot to the reopen window signal.
      boost::signals2::connection connect_reopen_signal(
        const ReopenSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable OpenSignal m_open_signal;
      mutable ReopenSignal m_reopen_signal;
      RecentlyClosedModel* m_model;
      std::vector<RecentlyClosedModel::Entry> m_entries;
      ToolbarMenu* m_window_manager_button;
      ToolbarMenu* m_recently_closed_button;
      IconButton* m_account_button;
      IconButton* m_key_bindings_button;
      IconButton* m_canvas_button;
      IconButton* m_book_view_button;
      IconButton* m_time_and_sales_button;
      IconButton* m_chart_button;
      IconButton* m_dashboard_button;
      IconButton* m_order_imbalances_button;
      IconButton* m_blotter_button;

      void entry_added(const RecentlyClosedModel::Entry& e);
      void entry_removed(const RecentlyClosedModel::Entry& e);
      void on_item_selected(int index);
      void on_open_window(RecentlyClosedModel::Type w);
  };
}

#endif
