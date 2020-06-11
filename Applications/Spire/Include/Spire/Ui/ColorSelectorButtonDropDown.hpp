#ifndef SPIRE_COLOR_SELECTOR_BUTTON_DROP_DOWN_HPP
#define SPIRE_COLOR_SELECTOR_BUTTON_DROP_DOWN_HPP
#include <QWidget>

namespace Spire {

  class ColorSelectorButtonDropDown : public QWidget {
    public:

      ColorSelectorButtonDropDown(const QColor& color,
        QWidget* parent = nullptr);
  };
}

#endif
