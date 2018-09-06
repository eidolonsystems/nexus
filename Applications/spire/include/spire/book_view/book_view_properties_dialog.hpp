#ifndef SPIRE_BOOK_VIEW_PROPERTIES_DIALOG_HPP
#define SPIRE_BOOK_VIEW_PROPERTIES_DIALOG_HPP
#include <QDialog>
#include <QTabWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_properties.hpp"

namespace Spire {

  //! Displays the properties for a book view window.
  class BookViewPropertiesDialog : public QDialog {
    public:

      //! Signals that properties should be applied to the parent window.
      using ApplySignal = Signal<void (const BookViewProperties& properties)>;

      //! Signals that properties should be applied to all windows.
      using ApplyAllSignal = Signal<
        void (const BookViewProperties& properties)>;

      //! Signals that properties should be saved as the default.
      using SaveDefaultSignal = Signal<
        void (const BookViewProperties& properties)>;

      //! Constructs a book view properties dialog.
      /*!
        \param properties The properties to modify.
        \param security The security used for modifying interactions.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      BookViewPropertiesDialog(const BookViewProperties& properties,
        const Nexus::Security& security, QWidget* parent = nullptr,
        Qt::WindowFlags flags = 0);

      //! Returns the properties represented by this dialog.
      BookViewProperties get_properties() const;

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
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ApplySignal m_apply_signal;
      mutable ApplyAllSignal m_apply_all_signal;
      mutable SaveDefaultSignal m_save_default_signal;
      QTabWidget* m_tab_widget;
      BookViewLevelPropertiesWidget* m_levels_tab_widget;
      BookViewHighlightPropertiesWidget* m_highlights_tab_widget;
      bool m_last_focus_was_key;

      void on_tab_bar_clicked(int index);
      void on_tab_changed();
  };
}

#endif
