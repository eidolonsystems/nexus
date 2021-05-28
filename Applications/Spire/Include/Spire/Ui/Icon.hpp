#ifndef SPIRE_ICON_HPP
#define SPIRE_ICON_HPP
#include <boost/optional/optional.hpp>
#include "Spire/Styles/Stylist.hpp"

namespace Spire {
namespace Styles {

  /** Sets the fill color of an icon. */
  using Fill = BasicProperty<QColor, struct FillTag>;
}

  /** Displays an icon. */
  class Icon : public QWidget {
    public:

      /**
       * Constructs an Icon.
       * @param icon The icon image.
       * @param parnet The parent widget.
       */
      explicit Icon(QImage icon, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      QImage m_icon;
      QColor m_background_color;
      QColor m_fill;
      boost::optional<QColor> m_border_color;

      void on_style();
  };
}

#endif
