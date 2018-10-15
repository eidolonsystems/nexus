#ifndef SPIRE_DASHBOARDWIDGET_HPP
#define SPIRE_DASHBOARDWIDGET_HPP
#include <functional>
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/SignalHandling/ConnectionGroup.hpp>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QTimer>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/PersistentWindow.hpp"

namespace Spire {

  /*! \class DashboardWidget
      \brief A QWidget that displays a DashboardModel.
   */
  class DashboardWidget : public QWidget, public UI::PersistentWindow {
    public:

      //! Constructs a DashboardWidget.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      DashboardWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      virtual ~DashboardWidget();

      //! Initializes this widget.
      /*!
        \param model The model to render.
        \param rowBuilder Builds the rows to display.
        \param userProfile The user's profile.
      */
      void Initialize(Beam::Ref<DashboardModel> model,
        const DashboardRowBuilder& rowBuilder,
        Beam::Ref<UserProfile> userProfile);

      //! Returns the selection model.
      const DashboardSelectionModel& GetSelectionModel() const;

      //! Returns the selection model.
      DashboardSelectionModel& GetSelectionModel();

      //! Returns the row builder.
      const DashboardRowBuilder& GetRowBuilder() const;

      //! Returns this widget's renderer.
      const DashboardRenderer& GetRenderer() const;

      //! Returns the display index of the row at a specified position.
      /*!
        \param position The position, relative to this renderer, of the row to
               get the index of.
        \return The display index of the row at the specified <i>position</i>.
      */
      boost::optional<int> GetRowDisplayIndex(const QPoint& position) const;

      virtual std::unique_ptr<UI::WindowSettings> GetWindowSettings() const;

    protected:
      virtual void keyPressEvent(QKeyEvent* event);
      virtual void keyReleaseEvent(QKeyEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void mouseDoubleClickEvent(QMouseEvent* event);
      virtual void paintEvent(QPaintEvent* event);
      virtual void resizeEvent(QResizeEvent* event);

    private:
      template<typename, typename> friend struct Beam::Serialization::Shuttle;
      friend class DashboardWidgetWindowSettings;
      enum MouseState {
        NONE,
        RESIZING_COLUMN,
        MOVING_COLUMN
      };
      struct SortOrder {
        enum class Order {
          ASCENDING,
          DESCENDING
        };
        int m_index;
        Order m_direction;
      };
      struct RowComparator;
      struct RendererComparator;
      UserProfile* m_userProfile;
      DashboardModel* m_model;
      std::unique_ptr<DashboardRowBuilder> m_rowBuilder;
      std::unique_ptr<DashboardRenderer> m_renderer;
      std::unique_ptr<DashboardSelectionModel> m_selectionModel;
      std::unique_ptr<DashboardSelectionController> m_selectionController;
      bool m_isHoveringOverColumnResize;
      MouseState m_mouseState;
      int m_activeColumnIndex;
      QPoint m_lastMousePressPosition;
      bool m_hasRepaintEvent;
      QTimer m_repaintTimer;
      std::vector<SortOrder> m_columnSortOrder;
      boost::signals2::scoped_connection m_drawConnection;
      boost::signals2::scoped_connection m_selectedRowsConnection;
      boost::signals2::scoped_connection m_activeRowConnection;
      boost::signals2::scoped_connection m_rowAddedConnection;
      Beam::SignalHandling::ConnectionGroup m_cellUpdateConnections;

      void ModifyColumnSortOrder(int index);
      void SortRows();
      void ActivateRow(int index, const std::string& prefix);
      void DeleteSelectedRows();
      void TestHoveringColumnExpansion(const QMouseEvent& event);
      int GetColumnAt(const QPoint& point);
      void ResizeColumn(const QMouseEvent& event);
      void MoveColumn(const QMouseEvent& event);
      void OnSelectedRowsUpdatedSignal();
      void OnRowAddedSignal(const DashboardRow& row);
      void OnCellUpdatedSignal(const DashboardRow& row,
        const DashboardCell::Value& value);
      void OnActiveRowUpdatedSignal(boost::optional<int> activeRow);
      void OnDrawSignal();
      void OnRepaintTimer();
  };
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Spire::DashboardWidget::SortOrder> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Spire::DashboardWidget::SortOrder& value, unsigned int version) {
      shuttle.Shuttle("index", value.m_index);
      shuttle.Shuttle("direction", value.m_direction);
    }
  };
}
}

#endif
