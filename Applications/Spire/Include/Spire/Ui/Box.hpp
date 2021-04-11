#ifndef SPIRE_BOX_HPP
#define SPIRE_BOX_HPP
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the background color of an element. */
  using BackgroundColor = BasicProperty<QColor, struct BackgroundColorTag>;

  /** Sets the size of the top border. */
  using BorderTopSize = BasicProperty<int, struct BorderTopSizeTag>;

  /** Sets the size of the right border. */
  using BorderRightSize = BasicProperty<int, struct BorderRightSizeTag>;

  /** Sets the size of the bottom border. */
  using BorderBottomSize = BasicProperty<int, struct BorderBottomSizeTag>;

  /** Sets the size of the left border. */
  using BorderLeftSize = BasicProperty<int, struct BorderLeftSizeTag>;

  /** Composes all border size properties. */
  using BorderSize = CompositeProperty<BorderTopSize, BorderRightSize,
    BorderBottomSize, BorderLeftSize>;

  /** Sets the color of the top border. */
  using BorderTopColor = BasicProperty<QColor, struct BorderTopColorTag>;

  /** Sets the color of the right border. */
  using BorderRightColor = BasicProperty<QColor, struct BorderRightColorTag>;

  /** Sets the color of the bottom border. */
  using BorderBottomColor = BasicProperty<QColor, struct BorderBottomColorTag>;

  /** Sets the color of the left border. */
  using BorderLeftColor = BasicProperty<QColor, struct BorderLeftColorTag>;

  /** Composes all border color properties. */
  using BorderColor = CompositeProperty<BorderTopColor, BorderRightColor,
    BorderBottomColor, BorderLeftColor>;

  /** Composes all border colors and sizes. */
  using Border = CompositeProperty<BorderSize, BorderColor>;

  /** Sets the size of the top padding. */
  using PaddingTop = BasicProperty<int, struct PaddingTopTag>;

  /** Sets the size of the right padding. */
  using PaddingRight = BasicProperty<int, struct PaddingRightTag>;

  /** Sets the size of the bottom padding. */
  using PaddingBottom = BasicProperty<int, struct PaddingBottomTag>;

  /** Sets the size of the left padding. */
  using PaddingLeft = BasicProperty<int, struct PaddingLeftTag>;

  /** Sets the size of the left and right padding. */
  using HorizontalPadding = CompositeProperty<PaddingRight, PaddingLeft>;

  /** Sets the size of the top and bottom padding. */
  using VerticalPadding = CompositeProperty<PaddingTop, PaddingBottom>;

  /** Composes all paddings into a single property. */
  using Padding = CompositeProperty<PaddingTop, PaddingRight, PaddingBottom,
    PaddingLeft>;

  /** Sets the alignment of the body. */
  using BodyAlign = BasicProperty<Qt::Alignment, struct BodyAlignTag>;

  /**
   * Applies a single size to all four corners of a border.
   * @param size The size to apply to all borders.
   */
  BorderSize border_size(Expression<int> size);

  /**
   * Applies a single color to all four corners of a border.
   * @param color The color to apply to all borders.
   */
  BorderColor border_color(Expression<QColor> color);

  /**
   * Applies a single color and size to all four corners of a border.
   * @param size The size to apply to all borders.
   * @param color The color to apply to all borders.
   */
  Border border(Expression<int> size, Expression<QColor> color);

  /**
   * Applies equal padding to the left and right of a widget.
   * @param size The size of the horizontal padding.
   */
  HorizontalPadding horizontal_padding(int size);

  /**
   * Applies equal padding to the top and bottom of a widget.
   * @param size The size of the vertical padding.
   */
  VerticalPadding vertical_padding(int size);

  /**
   * Applies equal padding to an entire widget.
   * @param size The size of the padding.
   */
  Padding padding(int size);
}

  /** Implements a containter displaying a component within a styled box. */
  class Box : public QWidget {
    public:

      /**
       * Constructs a Box.
       * @param body The component to display within the Box.
       * @param parent The parent widget.
       */
      explicit Box(QWidget* body, QWidget* parent = nullptr);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      QWidget* m_container;
      QWidget* m_body;

      void on_style();
  };

  /**
   * Displays a red warning indicator on a widget.
   * @param widget The widget to display the warning indicator over.
   */
  void display_warning_indicator(QWidget& widget);
}

#endif