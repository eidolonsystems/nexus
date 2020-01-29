#ifndef SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#define SPIRE_TIME_AND_SALES_TABLE_VIEW_HPP
#include <QAbstractItemModel>
#include <QHeaderView>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a table with horizontal header and loading widget.
  class TimeAndSalesTableView : public QScrollArea {
    public:

      //! Constructs a TimeAndSalesTableView.
      /*!
        \param parent The parent to this widget.
      */
      explicit TimeAndSalesTableView(QWidget* parent = nullptr);

      //! Sets the model to get the table data from.
      /*!
        \param model The model.
      */
      void set_model(TimeAndSalesWindowModel* model);

      //! Sets the TimeAndSalesProperties of the table.
      /*!
        \param properties The properties the table will be updated to have.
      */
      void set_properties(const TimeAndSalesProperties& properties);

    protected:
      bool event(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

    private:
      QVBoxLayout* m_layout;
      QHeaderView* m_header;
      QWidget* m_header_padding;
      QTableView* m_table;
      TimeAndSalesWindowModel* m_model;
      boost::signals2::scoped_connection m_model_begin_loading_connection;
      boost::signals2::scoped_connection m_model_end_loading_connection;
      QTimer m_h_scroll_bar_timer;
      QTimer m_v_scroll_bar_timer;
      std::unique_ptr<SnapshotLoadingWidget> m_loading_widget;
      std::unique_ptr<TransitionWidget> m_transition_widget;

      void fade_out_horizontal_scroll_bar();
      void fade_out_vertical_scroll_bar();
      void set_scroll_bar_style(int size);
      void show_loading_widget();
      void update_table_height(int num_rows);
      bool is_within_horizontal_scroll_bar(const QPoint& pos);
      bool is_within_vertical_scroll_bar(const QPoint& pos);
      void on_end_loading_signal();
      void on_header_resize(int index, int old_size, int new_size);
      void on_header_move(int logical_index, int old_index, int new_index);
      void on_horizontal_slider_value_changed(int new_value);
      void on_vertical_slider_value_changed(int new_value);
      void on_rows_about_to_be_inserted(const QModelIndex& parent,
        int first_index, int last_index);
  };
}

#endif
