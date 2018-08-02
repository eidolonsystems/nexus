#ifndef SPIRE_LOGIN_WINDOW_HPP
#define SPIRE_LOGIN_WINDOW_HPP
#include <string>
#include <boost/signals2/connection.hpp>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QWidget>
#include "spire/login/login.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Displays the login window.
  class LoginWindow : public QWidget {
    public:

      //! The login state to display to the user.
      enum class State {

        //! Nothing to report.
        NONE,

        //! The user is in the process of logging in.
        LOGGING_IN,

        //! The user requested to cancel.
        CANCELLING,

        //! The login attempt used incorrect credentials.
        INCORRECT_CREDENTIALS,

        //! The server is unavailable.
        SERVER_UNAVAILABLE
      };

      //! Signals an attempt to login.
      /*!
        \param username The username to login with.
        \param password The password to login with.
      */
      using LoginSignal = Signal<void (const std::string& username,
        const std::string& password)>;

      //! Signals to cancel a previous login operation.
      using CancelSignal = Signal<void ()>;

      //! Constructs a login window in the NONE state.
      LoginWindow(const std::string& version, QWidget* parent = nullptr);

      ~LoginWindow();

      //! Sets the state to display to the user.
      void set_state(State state);

      //! Connects a slot to the login signal.
      boost::signals2::connection connect_login_signal(
        const LoginSignal::slot_type& slot) const;

      //! Connects a slot to the cancel signal.
      boost::signals2::connection connect_cancel_signal(
        const CancelSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable LoginSignal m_login_signal;
      mutable CancelSignal m_cancel_signal;
      State m_state;
      std::unique_ptr<drop_shadow> m_shadow;
      QLabel* m_logo_widget;
      QLabel* m_status_label;
      QLineEdit* m_username_line_edit;
      QLineEdit* m_password_line_edit;
      flat_button* m_sign_in_button;
      icon_button* m_exit_button;
      bool m_is_dragging;
      QPoint m_last_pos;
      ChromaHashWidget* m_chroma_hash_widget;

      void reset_all();
      void reset_visuals();
      void try_login();
      void on_input_updated();
      void on_password_updated();
  };
}

#endif
