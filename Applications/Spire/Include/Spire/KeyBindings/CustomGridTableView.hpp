#ifndef SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#define SPIRE_CUSTOM_GRID_TABLE_VIEW_HPP
#include <boost/optional.hpp>
#include <QTableView>

namespace Spire {

  //! Represents a table view a cutom grid line and item highlight style.
  class CustomGridTableView : public QTableView {
    public:

      //! Constructs a CustomGridTableView.
      /*
        \param parent The parent widget.
      */
      explicit CustomGridTableView(QWidget* parent = nullptr);

    protected:
      void leaveEvent(QEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      boost::optional<QPoint> m_last_mouse_pos;

      void draw_border(const QModelIndex& index, QPainter* painter);
  };
}

#endif