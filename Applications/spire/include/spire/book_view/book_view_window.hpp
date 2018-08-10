#ifndef SPIRE_BOOK_VIEW_WINDOW_HPP
#define SPIRE_BOOK_VIEW_WINDOW_HPP
#include <boost/optional.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"
#include "spire/book_view/bbo_quote_panel.hpp"
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_model.hpp"
#include "spire/book_view/book_view_properties.hpp"
#include "spire/ui/security_stack.hpp"
#include "spire/ui/ui.hpp"

namespace Spire {

  //! Displays a book view window.
  class BookViewWindow : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param s The security to display.
      */
      using ChangeSecuritySignal = Signal<void (const Nexus::Security& s)>;

      //! Signals that the window closed.
      using ClosedSignal = Signal<void ()>;

      //! Constructs a book view window.
      /*!
        \param properties The display properties.
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      BookViewWindow(const BookViewProperties& properties,
        SecurityInputModel& input_model, QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<BookViewModel> model);

      //! Returns the display properties.
      const BookViewProperties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const BookViewProperties& properties);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

      //! Connects a slot to the window closed signal.
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      mutable ChangeSecuritySignal m_change_security_signal;
      mutable ClosedSignal m_closed_signal;
      BookViewProperties m_properties;
      SecurityInputModel* m_input_model;
      std::shared_ptr<BookViewModel> m_model;
      SecurityStack m_securities;
      Nexus::Security m_current_security;
      QWidget* m_body;
      QVBoxLayout* m_layout;
      TechnicalsPanel* m_technicals_panel;
      std::unique_ptr<BboQuotePanel> m_bbo_quote_panel;
      std::unique_ptr<QWidget> m_overlay_widget;
      std::unique_ptr<QLabel> m_empty_window_label;
      std::unique_ptr<TransitionWidget> m_transition_widget;
      QWidget* m_quote_widgets_container;
      QWidget* m_table;
      QtPromise<void> m_data_loaded_promise;
      bool m_is_data_loaded;

      void set_current(const Nexus::Security& s);
      void show_context_menu(const QPoint& pos);
      void show_overlay_widget();
      void show_properties_dialog();
      void show_transition_widget();
      void on_security_input_accept(SecurityInputDialog* dialog);
      void on_security_input_reject(SecurityInputDialog* dialog);
      void on_data_loaded(Beam::Expect<void> value);
  };
}

#endif
