#ifndef SPIRE_BOOKVIEWPANEL_HPP
#define SPIRE_BOOKVIEWPANEL_HPP
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <QMetaObject>
#include <QTimer>
#include <QWidget>
#include "Nexus/Definitions/Quote.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/BookView/BookViewProperties.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

class QModelIndex;
class QTableView;
class Ui_BookViewPanel;

namespace Spire {

  /*! \class BookViewPanel
      \brief Displays one Side of a Security's book.
   */
  class BookViewPanel : public QWidget {
    public:

      //! Constructs a BookViewPanel.
      /*!
        \param parent The parent widget.
        \param flags The flags passed to the <i>parent</i> widget.
      */
      BookViewPanel(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

      virtual ~BookViewPanel();

      //! Initializes this BookViewPanel.
      /*!
        \param userProfile The user's profile.
        \param properties The BookViewProperties used to display this panel.
        \param side The Side to display.
      */
      void Initialize(Beam::RefType<UserProfile> userProfile,
        const BookViewProperties& properties, Nexus::Side side);

      //! Returns the best Quote currently displayed.
      const Nexus::Quote& GetBestQuote() const;

      //! Returns the table displaying the Quotes.
      QTableView& GetQuoteList();

      //! Sets the BookViewProperties used to display this panel.
      void SetProperties(const BookViewProperties& properties);

      //! Sets the Security to display.
      /*!
        \param security The Security to display.
      */
      void DisplaySecurity(const Nexus::Security& security);

    protected:
      virtual void resizeEvent(QResizeEvent* event);

    private:
      friend class BookViewWindowSettings;
      std::unique_ptr<Ui_BookViewPanel> m_ui;
      UserProfile* m_userProfile;
      Beam::DelayPtr<UI::CustomVariantItemDelegate> m_itemDelegate;
      BookViewProperties m_properties;
      Nexus::Security m_security;
      Nexus::Side m_side;
      Nexus::Quantity m_boardLot;
      Nexus::Quote m_bestQuote;
      QTimer m_updateTimer;
      std::unique_ptr<BookViewModel> m_model;
      int m_topRow;
      int m_currentRow;
      QMetaObject::Connection m_rowsAboutToBeModifiedConnection;
      QMetaObject::Connection m_rowsModifiedConnection;
      QMetaObject::Connection m_rowsAboutToBeRemovedConnection;
      QMetaObject::Connection m_rowsRemovedConnection;
      Beam::DelayPtr<Beam::TaskQueue> m_slotHandler;

      void ConnectModel();
      void DisconnectModel();
      void OnBbo(const Nexus::Security& security, const Nexus::BboQuote& bbo);
      void OnUpdateTimer();
      void OnRowsAboutToBeModified(const QModelIndex& parent, int start,
        int end);
      void OnRowsModified(const QModelIndex& parent, int start, int end);
  };
}

#endif
