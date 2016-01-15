#ifndef NEXUS_POSITIONORDERBOOK_HPP
#define NEXUS_POSITIONORDERBOOK_HPP
#include <functional>
#include <unordered_map>
#include <vector>
#include <Beam/Utilities/CachedValue.hpp>
#include <boost/range/adaptor/map.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {
namespace Accounting {

  /*! \class PositionOrderBook
      \brief Maintains an Order book for the purpose of keeping track of
             positions and opening/closing orders.
   */
  class PositionOrderBook {
    public:

      /*! \class PositionEntry
          \brief Stores a single position.
       */
      struct PositionEntry {

        //! The position's Security.
        Security m_security;

        //! The position's quantity.
        Quantity m_quantity;

        //! Constructs a PositionEntry.
        /*!
          \param security The position's Security.
          \param quantity The position's quantity.
        */
        PositionEntry(const Security& security, Quantity quantity);
      };

      //! Constructs a PositionOrderBook.
      PositionOrderBook();

      //! Returns all live Orders.
      const std::vector<const OrderExecutionService::Order*>&
        GetLiveOrders() const;

      //! Returns all opening Orders.
      /*!
        \return The list of all all opening Orders.
      */
      const std::vector<const OrderExecutionService::Order*>&
        GetOpeningOrders() const;

      //! Returns all Positions.
      /*!
        \return The list of all all Positions.
      */
      const std::vector<PositionEntry>& GetPositions() const;

      //! Returns <code>true</code> iff an Order submission would result in an
      //! opening Order.
      /*!
        \param fields The OrderFields being submitted.
        \return <code>true</code> iff the <i>fields</i> would result in an
                opening Order, otherwise the <i>fields</i> represent a closing
                Order.
      */
      bool TestOpeningOrderSubmission(
        const OrderExecutionService::OrderFields& fields) const;

      //! Adds an Order to this book.
      /*!
        \param order The Order to add.
      */
      void Add(const OrderExecutionService::Order& order);

      //! Updates an Order with an ExecutionReport.
      /*!
        \param report The ExecutionReport with the details of the update.
      */
      void Update(const OrderExecutionService::ExecutionReport& report);

    private:
      struct OrderEntry {
        const OrderExecutionService::Order* m_order;
        Quantity m_remainingQuantity;
        int m_sequenceNumber;

        OrderEntry(const OrderExecutionService::Order& order,
          int sequenceNumber);
      };
      struct SecurityEntry {
        std::vector<OrderEntry> m_asks;
        std::vector<OrderEntry> m_bids;
        Quantity m_position;
        Quantity m_askOpenQuantity;
        Quantity m_bidOpenQuantity;

        SecurityEntry();
      };
      std::unordered_map<Security, SecurityEntry> m_securityEntries;
      std::unordered_map<OrderExecutionService::OrderId,
        OrderExecutionService::OrderFields> m_fields;
      int m_orderSequenceNumber;
      Beam::CachedValue<std::vector<const OrderExecutionService::Order*>>
        m_liveOrders;
      Beam::CachedValue<std::vector<const OrderExecutionService::Order*>>
        m_openingOrders;
      Beam::CachedValue<std::vector<PositionEntry>> m_positions;
  };

  inline bool operator ==(const PositionOrderBook::PositionEntry& lhs,
      const PositionOrderBook::PositionEntry& rhs) {
    return lhs.m_security == rhs.m_security && lhs.m_quantity == rhs.m_quantity;
  }

  inline bool operator !=(const PositionOrderBook::PositionEntry& lhs,
      const PositionOrderBook::PositionEntry& rhs) {
    return !(rhs == lhs);
  }

  inline PositionOrderBook::PositionEntry::PositionEntry(
      const Security& security, Quantity quantity)
      : m_security(security),
        m_quantity(quantity) {}

  inline PositionOrderBook::OrderEntry::OrderEntry(
      const OrderExecutionService::Order& order, int sequenceNumber)
      : m_order(&order),
        m_remainingQuantity(m_order->GetInfo().m_fields.m_quantity),
        m_sequenceNumber(sequenceNumber) {}

  inline PositionOrderBook::SecurityEntry::SecurityEntry()
      : m_position(0),
        m_askOpenQuantity(0),
        m_bidOpenQuantity(0) {}

  inline PositionOrderBook::PositionOrderBook()
      : m_orderSequenceNumber(0) {
    m_liveOrders.SetComputation(
      [=] {
        std::vector<const OrderExecutionService::Order*> orders;
        for(const auto& entry : m_securityEntries |
            boost::adaptors::map_values) {
          std::transform(entry.m_asks.begin(), entry.m_asks.end(),
            std::back_inserter(orders), std::mem_fn(&OrderEntry::m_order));
          std::transform(entry.m_bids.begin(), entry.m_bids.end(),
            std::back_inserter(orders), std::mem_fn(&OrderEntry::m_order));
        }
        return orders;
      });
    m_openingOrders.SetComputation(
      [=] {
        std::vector<const OrderExecutionService::Order*> orders;
        for(const auto& entry : m_securityEntries |
            boost::adaptors::map_values) {
          if(entry.m_position == 0) {
            std::transform(entry.m_asks.begin(), entry.m_asks.end(),
              std::back_inserter(orders), std::mem_fn(&OrderEntry::m_order));
            std::transform(entry.m_bids.begin(), entry.m_bids.end(),
              std::back_inserter(orders), std::mem_fn(&OrderEntry::m_order));
          } else if(entry.m_position > 0) {
            auto remainingQuantity = entry.m_position;
            for(const auto& orderEntry : entry.m_asks) {
              if(remainingQuantity <= 0) {
                orders.push_back(orderEntry.m_order);
              } else if(remainingQuantity >= orderEntry.m_remainingQuantity) {
                remainingQuantity -= orderEntry.m_remainingQuantity;
              } else {
                remainingQuantity -= orderEntry.m_remainingQuantity;
                orders.push_back(orderEntry.m_order);
              }
            }
            std::transform(entry.m_bids.begin(), entry.m_bids.end(),
              std::back_inserter(orders), std::mem_fn(&OrderEntry::m_order));
          } else {
            std::transform(entry.m_asks.begin(), entry.m_asks.end(),
              std::back_inserter(orders), std::mem_fn(&OrderEntry::m_order));
            auto remainingQuantity = -entry.m_position;
            for(const auto& orderEntry : entry.m_bids) {
              if(remainingQuantity <= 0) {
                orders.push_back(orderEntry.m_order);
              } else if(remainingQuantity >= orderEntry.m_remainingQuantity) {
                remainingQuantity -= orderEntry.m_remainingQuantity;
              } else {
                remainingQuantity -= orderEntry.m_remainingQuantity;
                orders.push_back(orderEntry.m_order);
              }
            }
          }
        }
        return orders;
      });
    m_positions.SetComputation(
      [=] {
        std::vector<PositionEntry> positions;
        for(const auto& securityEntryPair : m_securityEntries) {
          if(securityEntryPair.second.m_position != 0) {
            positions.emplace_back(securityEntryPair.first,
              securityEntryPair.second.m_position);
          }
        }
        return positions;
      });
  }

  inline const std::vector<const OrderExecutionService::Order*>&
      PositionOrderBook::GetLiveOrders() const {
    return *m_liveOrders;
  }

  inline const std::vector<const OrderExecutionService::Order*>&
      PositionOrderBook::GetOpeningOrders() const {
    return *m_openingOrders;
  }

  inline const std::vector<PositionOrderBook::PositionEntry>&
      PositionOrderBook::GetPositions() const {
    return *m_positions;
  }

  inline bool PositionOrderBook::TestOpeningOrderSubmission(
      const OrderExecutionService::OrderFields& fields) const {
    auto securityEntryIterator = m_securityEntries.find(fields.m_security);
    if(securityEntryIterator == m_securityEntries.end()) {
      return true;
    }
    const auto& securityEntry = securityEntryIterator->second;
    if(securityEntry.m_position == 0) {
      return true;
    }
    if(Nexus::GetSide(securityEntry.m_position) == fields.m_side) {
      return true;
    }
    auto openQuantity = Pick(fields.m_side, securityEntry.m_askOpenQuantity,
      securityEntry.m_bidOpenQuantity);
    if(openQuantity + fields.m_quantity > std::abs(securityEntry.m_position)) {
      return true;
    }
    return false;
  }

  inline void PositionOrderBook::Add(
      const OrderExecutionService::Order& order) {
    auto& fields = order.GetInfo().m_fields;
    m_fields.emplace(order.GetInfo().m_orderId, fields);
    auto& securityEntry = m_securityEntries[fields.m_security];
    auto& orders = Pick(fields.m_side, securityEntry.m_asks,
      securityEntry.m_bids);
    auto& openQuantity = Pick(fields.m_side, securityEntry.m_askOpenQuantity,
      securityEntry.m_bidOpenQuantity);
    openQuantity += fields.m_quantity;
    OrderEntry entry(order, m_orderSequenceNumber);
    ++m_orderSequenceNumber;
    auto insertIterator = std::lower_bound(orders.begin(), orders.end(),
      entry,
      [] (const OrderEntry& lhs, const OrderEntry& rhs) {
        return std::tie(lhs.m_order->GetInfo().m_fields, lhs.m_sequenceNumber) <
          std::tie(rhs.m_order->GetInfo().m_fields, rhs.m_sequenceNumber);
      });
    orders.insert(insertIterator, entry);
    m_liveOrders.Invalidate();
    m_openingOrders.Invalidate();
  }

  inline void PositionOrderBook::Update(
      const OrderExecutionService::ExecutionReport& report) {
    if(report.m_lastQuantity == 0 && !IsTerminal(report.m_status)) {
      return;
    }
    auto fieldsIterator = m_fields.find(report.m_id);
    if(fieldsIterator == m_fields.end()) {
      return;
    }
    const auto& fields = fieldsIterator->second;
    auto securityEntryIterator = m_securityEntries.find(fields.m_security);
    if(securityEntryIterator == m_securityEntries.end()) {
      return;
    }
    auto& securityEntry = securityEntryIterator->second;
    auto& orders = Pick(fields.m_side, securityEntry.m_asks,
      securityEntry.m_bids);
    auto entryIterator = std::find_if(orders.begin(), orders.end(),
      [&] (const OrderEntry& entry) {
        return entry.m_order->GetInfo().m_orderId == report.m_id;
      });
    if(entryIterator == orders.end()) {
      return;
    }
    if(IsTerminal(report.m_status)) {
      m_liveOrders.Invalidate();
    }
    m_openingOrders.Invalidate();
    if(report.m_lastQuantity != 0) {
      m_positions.Invalidate();
    }
    securityEntry.m_position += GetDirection(fields.m_side) *
      report.m_lastQuantity;
    auto& openQuantity = Pick(fields.m_side, securityEntry.m_askOpenQuantity,
      securityEntry.m_bidOpenQuantity);
    openQuantity -= report.m_lastQuantity;
    auto& entry = *entryIterator;
    entry.m_remainingQuantity -= report.m_lastQuantity;
    if(entry.m_remainingQuantity == 0 || IsTerminal(report.m_status)) {
      openQuantity -= entry.m_remainingQuantity;
      m_fields.erase(report.m_id);
      orders.erase(entryIterator);
    }
  }
}
}

#endif
