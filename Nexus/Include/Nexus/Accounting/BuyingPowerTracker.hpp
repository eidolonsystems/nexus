#ifndef NEXUS_BUYINGPOWERTRACKER_HPP
#define NEXUS_BUYINGPOWERTRACKER_HPP
#include <unordered_map>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {
namespace Accounting {

  /*! \class BuyingPowerTracker
      \brief Tracks the amount of buying power used up by a series of Orders.
   */
  class BuyingPowerTracker {
    public:

      //! Constructs an empty BuyingPowerTracker.
      BuyingPowerTracker();

      //! Returns <code>true</code> iff an Order has been accounted for.
      /*!
        \param id The Order id to check.
        \return <code>true</code> iff the Order with the specified <i>id</i> has
                previously been accounted for.
      */
      bool HasOrder(OrderExecutionService::OrderId id) const;

      //! Returns the buying power used in a particular Currency.
      /*!
        \param currency The Currency to lookup.
        \return The buying power used in the <i>currency</i>.
      */
      Money GetBuyingPower(CurrencyId currency) const;

      //! Tracks a submission and returns the updated buying power.
      /*!
        \param id The id used to track this submission.
        \param orderFields The OrderFields storing the details of the
               submission.
        \param expectedPrice The expected price of the Order, this may differ
               from the price that the Order is submitted for.
        \return The updated buying power for the submission's Currency.
      */
      Money Submit(OrderExecutionService::OrderId id,
        const OrderExecutionService::OrderFields& orderFields,
        Money expectedPrice);

      //! Updates this tracker with the contents of an ExecutionReport.
      /*!
        \param executionReport The ExecutionReport to update this tracker with.
      */
      void Update(
        const OrderExecutionService::ExecutionReport& executionReport);

    private:
      struct OrderEntry {
        OrderExecutionService::OrderId m_id;
        OrderExecutionService::OrderFields m_fields;
        Money m_expectedPrice;
        Quantity m_remainingQuantity;

        OrderEntry(OrderExecutionService::OrderId id,
          const OrderExecutionService::OrderFields& fields,
          Money expectedPrice);
      };
      struct BuyingPowerEntry {
        std::vector<OrderEntry> m_asks;
        std::vector<OrderEntry> m_bids;
        Money m_expenditure;
        Quantity m_quantity;

        BuyingPowerEntry();
      };
      std::unordered_map<OrderExecutionService::OrderId,
        OrderExecutionService::OrderFields> m_idToOrderFields;
      std::unordered_map<Security, BuyingPowerEntry> m_buyingPowerEntries;
      std::unordered_map<CurrencyId, Money> m_currencyToBuyingPower;

      static Money ComputeBuyingPower(
        const std::vector<OrderEntry>& orderEntries, Quantity quantityOffset);
      static Money ComputeBuyingPower(const BuyingPowerEntry& entry);
  };

  inline BuyingPowerTracker::OrderEntry::OrderEntry(
      OrderExecutionService::OrderId id,
      const OrderExecutionService::OrderFields& fields, Money expectedPrice)
      : m_id(id),
        m_fields(fields),
        m_expectedPrice(expectedPrice),
        m_remainingQuantity(fields.m_quantity) {}

  inline BuyingPowerTracker::BuyingPowerEntry::BuyingPowerEntry()
      : m_quantity(0) {}

  inline BuyingPowerTracker::BuyingPowerTracker() {}

  inline bool BuyingPowerTracker::HasOrder(
      OrderExecutionService::OrderId id) const {
    return m_idToOrderFields.find(id) != m_idToOrderFields.end();
  }

  inline Money BuyingPowerTracker::GetBuyingPower(CurrencyId currency) const {
    auto currencyIterator = m_currencyToBuyingPower.find(currency);
    if(currencyIterator == m_currencyToBuyingPower.end()) {
      return Money::ZERO;
    }
    return currencyIterator->second;
  }

  inline Money BuyingPowerTracker::Submit(OrderExecutionService::OrderId id,
      const OrderExecutionService::OrderFields& orderFields,
      Money expectedPrice) {
    auto& entry = m_buyingPowerEntries[orderFields.m_security];
    auto& buyingPower = m_currencyToBuyingPower[orderFields.m_currency];
    buyingPower -= ComputeBuyingPower(entry);
    auto& orderEntries = Pick(orderFields.m_side, entry.m_asks, entry.m_bids);
    OrderEntry orderEntry(id, orderFields, expectedPrice);
    auto insertIterator = std::lower_bound(orderEntries.begin(),
      orderEntries.end(), orderEntry,
      [](const OrderEntry& lhs, const OrderEntry& rhs) {
        return (lhs.m_fields.m_side == Side::ASK &&
          lhs.m_expectedPrice < rhs.m_expectedPrice) ||
          (lhs.m_fields.m_side == Side::BID &&
          lhs.m_expectedPrice > rhs.m_expectedPrice);
      });
    if(insertIterator != orderEntries.end() &&
        insertIterator->m_remainingQuantity == 0) {
      *insertIterator = orderEntry;
    } else {
      orderEntries.insert(insertIterator, orderEntry);
    }
    buyingPower += ComputeBuyingPower(entry);
    m_idToOrderFields.insert(std::make_pair(id, orderFields));
    return buyingPower;
  }

  inline void BuyingPowerTracker::Update(
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_status == OrderStatus::PENDING_NEW ||
        executionReport.m_status == OrderStatus::SUSPENDED ||
        executionReport.m_status == OrderStatus::PENDING_CANCEL ||
        executionReport.m_status == OrderStatus::NEW ||
        executionReport.m_status == OrderStatus::CANCEL_REJECT) {
      return;
    }
    auto& orderFields = m_idToOrderFields.at(executionReport.m_id);
    auto& buyingPowerEntry = m_buyingPowerEntries.at(orderFields.m_security);
    auto& buyingPower = m_currencyToBuyingPower[orderFields.m_currency];
    buyingPower -= ComputeBuyingPower(buyingPowerEntry);
    auto& orderEntries = Pick(orderFields.m_side, buyingPowerEntry.m_asks,
      buyingPowerEntry.m_bids);
    for(auto& orderEntry : orderEntries) {
      if(executionReport.m_id == orderEntry.m_id) {
        if(IsTerminal(executionReport.m_status)) {
          orderEntry.m_remainingQuantity = 0;
        } else {
          orderEntry.m_remainingQuantity -= executionReport.m_lastQuantity;
        }
        break;
      }
    }
    auto lastQuantity = executionReport.m_lastQuantity;
    if((orderFields.m_side == Side::BID && buyingPowerEntry.m_quantity < 0) ||
        (orderFields.m_side == Side::ASK && buyingPowerEntry.m_quantity > 0)) {
      auto delta =
        std::min(Abs(buyingPowerEntry.m_quantity), lastQuantity);
      buyingPowerEntry.m_expenditure -=
        GetDirection(GetOpposite(orderFields.m_side)) * delta *
        (buyingPowerEntry.m_expenditure / buyingPowerEntry.m_quantity);
      buyingPowerEntry.m_quantity += GetDirection(orderFields.m_side) * delta;
      lastQuantity -= delta;
    }
    buyingPowerEntry.m_quantity += GetDirection(orderFields.m_side) *
      lastQuantity;
    buyingPowerEntry.m_expenditure += GetDirection(orderFields.m_side) *
      lastQuantity * executionReport.m_lastPrice;
    buyingPower += ComputeBuyingPower(buyingPowerEntry);
  }

  inline Money BuyingPowerTracker::ComputeBuyingPower(
      const std::vector<OrderEntry>& orderEntries, Quantity quantityOffset) {
    Money buyingPower;
    for(const auto& orderEntry : orderEntries) {
      if(quantityOffset == 0) {
        buyingPower += orderEntry.m_remainingQuantity *
          orderEntry.m_expectedPrice;
      } else if(orderEntry.m_remainingQuantity < quantityOffset) {
        quantityOffset -= orderEntry.m_remainingQuantity;
      } else {
        buyingPower += (orderEntry.m_remainingQuantity - quantityOffset) *
          orderEntry.m_expectedPrice;
        quantityOffset = 0;
      }
    }
    return buyingPower;
  }

  inline Money BuyingPowerTracker::ComputeBuyingPower(
      const BuyingPowerEntry& entry) {
    Money askBuyingPower;
    Money bidBuyingPower;
    if(entry.m_quantity >= 0) {
      askBuyingPower = ComputeBuyingPower(entry.m_asks, entry.m_quantity);
      bidBuyingPower = ComputeBuyingPower(entry.m_bids, 0) +
        entry.m_expenditure;
    } else {
      askBuyingPower = ComputeBuyingPower(entry.m_asks, 0) -
        entry.m_expenditure;
      bidBuyingPower = ComputeBuyingPower(entry.m_bids, -entry.m_quantity);
    }
    return std::max(askBuyingPower, bidBuyingPower);
  }
}
}

#endif
