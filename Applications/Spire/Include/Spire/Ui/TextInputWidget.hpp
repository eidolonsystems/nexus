#ifndef SPIRE_TEXT_INPUT_WIDGET_HPP
#define SPIRE_TEXT_INPUT_WIDGET_HPP
#include <QLineEdit>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a Spire-style line edit.
  class TextInputWidget : public QLineEdit {
    public:

      //! Constructs a TextInputWidget with the default style.
      /*!
        \param parent The parent widget.
      */
      explicit TextInputWidget(QWidget* parent = nullptr);

      //! Constructs a TextInputWidget with the default style.
      /*!
        \param text The text to display.
        \param parent The parent widget.
      */
      explicit TextInputWidget(QString text, QWidget* parent = nullptr);

    protected:
      void focusInEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
  };
}

#endif
