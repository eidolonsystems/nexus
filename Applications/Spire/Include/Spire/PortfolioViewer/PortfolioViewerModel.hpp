#ifndef SPIRE_PORTFOLIOVIEWERMODEL_HPP
#define SPIRE_PORTFOLIOVIEWERMODEL_HPP
#include <unordered_map>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Routines/RoutineHandlerGroup.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/signals2/signal.hpp>
#include <QAbstractItemModel>
#include <QTimer>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Spire/PortfolioViewer/PortfolioViewer.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class PortfolioViewerModel
      \brief Models the portfolio's viewed by a manager's account.
   */
  class PortfolioViewerModel : public QAbstractTableModel {
    public:

      /*! \struct Entry
          \brief Stores a single portfolio entry.
       */
      struct Entry {

        //! The Entry's account.
        Beam::ServiceLocator::DirectoryEntry m_account;

        //! The Entry's group.
        Beam::ServiceLocator::DirectoryEntry m_group;

        //! A single Inventory item held by the <i>account</i>.
        Nexus::RiskService::RiskInventory m_inventory;

        //! Constructs an Entry.
        Entry();

        //! Constructs an Entry.
        /*!
          \param account The Entry's account.
        */
        Entry(const Beam::ServiceLocator::DirectoryEntry& account);
      };

      /*! \struct TotalEntry
          \brief Stores the totals among all displayed Entries.
       */
      struct TotalEntry {

        //! The total quantity transacted.
        Nexus::Quantity m_volume;

        //! The total number of transactions.
        int m_trades;

        //! The unrealized profit and loss.
        Nexus::Money m_unrealizedProfitAndLoss;

        //! The realized profit and loss.
        Nexus::Money m_realizedProfitAndLoss;

        //! The total fees paid.
        Nexus::Money m_fees;

        //! Constructs a TotalEntry.
        TotalEntry();
      };

      /*! \enum Columns
          \brief The available columns to display.
       */
      enum Columns {

        //! The group the account belongs to.
        GROUP_COLUMN,

        //! The account that executed the trades.
        ACCOUNT_COLUMN,

        //! The account's total profit/loss.
        ACCOUNT_TOTAL_PROFIT_LOSS_COLUMN,

        //! The account's unrealized profit/loss.
        ACCOUNT_UNREALIZED_PROFIT_LOSS_COLUMN,

        //! The account's realized profit/loss.
        ACCOUNT_REALIZED_PROFIT_LOSS_COLUMN,

        //! The total amount of fees paid/received by the account.
        ACCOUNT_FEES_COLUMN,

        //! The Security traded.
        SECURITY_COLUMN,

        //! The position's current quantity.
        OPEN_QUANTITY_COLUMN,

        //! The position's Side.
        SIDE_COLUMN,

        //! The average price of the position.
        AVERAGE_PRICE_COLUMN,

        //! The position's total profit/loss.
        TOTAL_PROFIT_LOSS_COLUMN,

        //! The position's unrealized profit/loss.
        UNREALIZED_PROFIT_LOSS_COLUMN,

        //! The position's realized profit/loss.
        REALIZED_PROFIT_LOSS_COLUMN,

        //! The total fees paid in the position.
        FEES_COLUMN,

        //! The cost basis.
        COST_BASIS_COLUMN,

        //! The position's Currency.
        CURRENCY_COLUMN,

        //! The total quantity traded.
        VOLUME_COLUMN,

        //! The total number of trades executed.
        TRADES_COLUMN,
      };

      //! The number of columns in this model.
      static const int COLUMN_COUNT = 18;

      //! Signals a change to this model's TotalEntry.
      /*!
        \param totals The updated TotalEntry.
      */
      typedef boost::signals2::signal<void (const TotalEntry& entry)>
        TotalsUpdatedSignal;

      //! Constructs a PorfolioViewerModel.
      /*!
        \param userProfile The user's profile.
        \param selectionModel The PortfolioSelectionModel used to filter this
               model.
      */
      PortfolioViewerModel(Beam::Ref<UserProfile> userProfile,
        Beam::Ref<PortfolioSelectionModel> selectionModel);

      virtual ~PortfolioViewerModel();

      //! Returns the Entry at a specified index.
      /*!
        \param index The index of the entry to return.
        \return The Entry at the specified <i>index</i>.
      */
      const Entry& GetEntry(int index) const;

      //! Returns this model's TotalEntry.
      const TotalEntry& GetTotals() const;

      //! Connects a slot to the TotalsUpdatedSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectTotalsUpdatedSignal(
        const TotalsUpdatedSignal::slot_type& slot) const;

      virtual int rowCount(const QModelIndex& parent) const;

      virtual int columnCount(const QModelIndex& parent) const;

      virtual QVariant data(const QModelIndex& index, int role) const;

      virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

    private:
      struct AccountTotals {
        Nexus::Money m_unrealizedProfitAndLoss;
        Nexus::Money m_realizedProfitAndLoss;
        Nexus::Money m_fees;
      };
      struct IndexedEntry : public Entry {
        bool m_isDisplayed;
        int m_displayIndex;
      };
      UserProfile* m_userProfile;
      Nexus::CurrencyId m_totalCurrency;
      const Nexus::ExchangeRateTable* m_exchangeRates;
      PortfolioSelectionModel* m_selectionModel;
      std::vector<IndexedEntry> m_entries;
      int m_displayCount;
      TotalEntry m_totals;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        Beam::ServiceLocator::DirectoryEntry> m_groups;
      std::unordered_map<Nexus::Security, Nexus::Accounting::SecurityValuation>
        m_valuations;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry, AccountTotals>
        m_accountTotals;
      std::unordered_map<Nexus::Security, std::vector<int>> m_securityToIndexes;
      std::unordered_map<Nexus::RiskService::RiskPortfolioKey, int>
        m_inventoryKeyToIndex;
      QTimer m_updateTimer;
      Beam::Routines::RoutineHandlerGroup m_loadRoutines;
      std::shared_ptr<Beam::TaskQueue> m_slotHandler;
      mutable TotalsUpdatedSignal m_totalsUpdatedSignal;

      boost::optional<Nexus::Money> GetUnrealizedProfitAndLoss(
        const Nexus::RiskService::RiskInventory& inventory) const;
      void OnBboQuote(const Nexus::Security& security,
        const Nexus::BboQuote& bboQuote);
      void OnRiskPortfolioInventoryUpdate(
        const Nexus::RiskService::RiskInventoryEntry& entry);
      void OnSelectionModelUpdated(const QModelIndex& topLeft,
        const QModelIndex& bottomRight);
      void OnUpdateTimer();
  };
}

#endif
