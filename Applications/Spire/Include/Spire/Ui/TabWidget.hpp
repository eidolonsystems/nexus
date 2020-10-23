#ifndef SPIRE_TAB_WIDGET_HPP
#define SPIRE_TAB_WIDGET_HPP
#include <QTabWidget>

namespace Spire {

  //! Represents a Spire-styled QTabWidget.
  class TabWidget : public QTabWidget {
    public:

      //! Constructs a TabWidget.
      /*!
        \param parent The parent widget.
      */
      explicit TabWidget(QWidget* parent = nullptr);
  };
}

#endif
