#ifndef SPIRE_TITLE_BAR_HPP
#define SPIRE_TITLE_BAR_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a window's title bar.
  class TitleBar : public QWidget {
    public:

      //! Constructs a title bar.
      /*!
        \param parent The parent widget.
      */
      explicit TitleBar(QWidget* parent = nullptr);

      //! Constructs a title bar.
      /*!
        \param icon The window icon to display at the top-left corner.
        \param parent The parent widget.
      */
      explicit TitleBar(const QImage& icon, QWidget* parent = nullptr);

      //! Constructs a title bar.
      /*!
        \param icon The window icon to display at the top-left corner.
        \param unfocused_icon The icon to display when the window lacks focus.
        \param parent The parent widget.
      */
      TitleBar(const QImage& icon, const QImage& unfocused_icon,
        QWidget* parent = nullptr);

      //! Sets the icon to display.
      /*!
        \param icon The icon to display when the window has focus.
      */
      void set_icon(const QImage& icon);

      //! Sets the icon to display.
      /*!
        \param icon The icon to display when the window has focus.
        \param icon The icon to display when the window lacks focus.
      */
      void set_icon(const QImage& icon, const QImage& unfocused_icon);

      //! Returns the title text QLabel.
      QLabel* get_title_label() const;

    protected:
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      IconButton* m_icon;
      QImage m_default_icon;
      QImage m_unfocused_icon;
      QLabel* m_title_label;
      IconButton* m_minimize_button;
      IconButton* m_maximize_button;
      IconButton* m_restore_button;
      IconButton* m_close_button;

      void on_window_title_change(const QString& title);
      void on_minimize_button_press();
      void on_maximize_button_press();
      void on_restore_button_press();
      void on_close_button_press();
      void set_title_text_stylesheet(const QColor& font_color);
  };
}

#endif
