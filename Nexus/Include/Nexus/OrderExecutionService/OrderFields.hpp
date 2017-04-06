#ifndef NEXUS_ORDERFIELDS_HPP
#define NEXUS_ORDERFIELDS_HPP
#include <ostream>
#include <string>
#include <tuple>
#include <vector>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/optional/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \struct OrderFields
      \brief Contains the fields needed to represent an Order.
   */
  struct OrderFields {

    //! The account to assign the Order to.
    Beam::ServiceLocator::DirectoryEntry m_account;

    //! The Security the Order was submitted for.
    Security m_security;

    //! The Currency being used.
    CurrencyId m_currency;

    //! The type of Order.
    OrderType m_type;

    //! The Side of the Order.
    Side m_side;

    //! The destination to submit the Order to.
    std::string m_destination;

    //! The quantity to order.
    Quantity m_quantity;

    //! The price of the Order.
    Money m_price;

    //! The Order's TimeInForce.
    TimeInForce m_timeInForce;

    //! Carries any additional fields to submit.
    std::vector<Tag> m_additionalFields;

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param account The account to assign the Order to.
      \param security The Security the Order was submitted for.
      \param currency The Currency being used.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const std::string& destination, Quantity quantity, Money price);

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param currency The Currency being used.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(const Security& security,
      CurrencyId currency, Side side, const std::string& destination,
      Quantity quantity, Money price);

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param account The account to assign the Order to.
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, const std::string& destination,
      Quantity quantity, Money price);

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(const Security& security, Side side,
      const std::string& destination, Quantity quantity, Money price);

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param currency The Currency being used.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity, Money price);

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param account The account to assign the Order to.
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity, Money price);

    //! Builds an OrderFields for a LIMIT order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildLimitOrder(const Security& security, Side side,
      Quantity quantity, Money price);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param account The account to assign the Order to.
      \param security The Security the Order was submitted for.
      \param currency The Currency being used.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const std::string& destination, Quantity quantity);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param currency The Currency being used.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(const Security& security,
      CurrencyId currency, Side side, const std::string& destination,
      Quantity quantity);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param account The account to assign the Order to.
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, const std::string& destination,
      Quantity quantity);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param destination The destination to submit the Order to.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(const Security& security, Side side,
      const std::string& destination, Quantity quantity);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param currency The Currency being used.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param account The account to assign the Order to.
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity);

    //! Builds an OrderFields for a MARKET order with all mandatory fields
    //! populated.
    /*!
      \param security The Security the Order was submitted for.
      \param orderType The type of Order.
      \param side The Side of the Order.
      \param quantity The quantity to order.
      \param price The price of the Order.
    */
    static OrderFields BuildMarketOrder(const Security& security, Side side,
      Quantity quantity);

    //! Constructs an OrderFields.
    OrderFields();

    //! Returns <code>true</code> iff one OrderFields comes before another.
    /*!
      \param rhs The right hand side of the comparison.
      \return <code>true</code> iff <code>this</code> comes before <i>rhs</i>.
    */
    bool operator <(const OrderFields& rhs) const;

    //! Returns <code>true</code> iff <code>this</code> is equal to another
    //! OrderFields object.
    /*!
      \param rhs The right hand side of the comparison.
      \return <code>true</code> iff <code>this</code> is equal to <i>rhs</i>.
    */
    bool operator ==(const OrderFields& rhs) const;
  };

  //! Checks if a Tag with a specified key is part of an OrderFields.
  /*!
    \param fields The OrderFields to search.
    \param key The key of the Tag to find.
    \return The Tag found with the specified <i>key</i>.
  */
  inline boost::optional<Tag> FindField(const OrderFields& fields, int key) {
    for(const Tag& tag : fields.m_additionalFields) {
      if(tag.GetKey() == key) {
        return tag;
      }
    }
    return boost::optional<Tag>();
  }

  //! Checks if a specified Tag is part of an OrderFields.
  /*!
    \param fields The OrderFields to search.
    \param tag The Tag to find.
    \return <code>true</code> iff the specified <i>tag</i> was found.
  */
  inline bool HasField(const OrderFields& fields, const Tag& tag) {
    for(const Tag& t : fields.m_additionalFields) {
      if(t == tag) {
        return true;
      }
    }
    return false;
  }

  inline std::ostream& operator <<(std::ostream& out,
      const OrderFields& value) {
    out << "(" << value.m_account << " " << value.m_security << " " <<
      value.m_currency << " " << value.m_type << " " << value.m_side << " " <<
      value.m_destination << " " << value.m_quantity << " " << value.m_price <<
      " " << value.m_timeInForce << " ";
    return ::operator <<(out, value.m_additionalFields) << ")";
  }

  inline OrderFields OrderFields::BuildLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const std::string& destination, Quantity quantity, Money price) {
    OrderFields fields;
    fields.m_account = account;
    fields.m_security = security;
    fields.m_currency = currency;
    fields.m_type = OrderType::LIMIT;
    fields.m_side = side;
    fields.m_destination = destination;
    fields.m_quantity = quantity;
    fields.m_price = price;
    fields.m_timeInForce = TimeInForce{TimeInForce::Type::DAY};
    return fields;
  }

  inline OrderFields OrderFields::BuildLimitOrder(const Security& security,
      CurrencyId currency, Side side, const std::string& destination,
      Quantity quantity, Money price) {
    return BuildLimitOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      currency, side, destination, quantity, price);
  }

  inline OrderFields OrderFields::BuildLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side,
      const std::string& destination, Quantity quantity, Money price) {
    return BuildLimitOrder(account, security, CurrencyId::NONE(), side,
      destination, quantity, price);
  }

  inline OrderFields OrderFields::BuildLimitOrder(const Security& security,
      Side side, const std::string& destination, Quantity quantity,
      Money price) {
    return BuildLimitOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      CurrencyId::NONE(), side, destination, quantity, price);
  }

  inline OrderFields OrderFields::BuildLimitOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity, Money price) {
    return BuildLimitOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      currency, side, {}, quantity, price);
  }

  inline OrderFields OrderFields::BuildLimitOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity, Money price) {
    return BuildLimitOrder(account, security, CurrencyId::NONE(), side, {},
      quantity, price);
  }

  inline OrderFields OrderFields::BuildLimitOrder(const Security& security,
      Side side, Quantity quantity, Money price) {
    return BuildLimitOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      CurrencyId::NONE(), side, {}, quantity, price);
  }

  inline OrderFields OrderFields::BuildMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, CurrencyId currency, Side side,
      const std::string& destination, Quantity quantity) {
    OrderFields fields;
    fields.m_account = account;
    fields.m_security = security;
    fields.m_currency = currency;
    fields.m_type = OrderType::MARKET;
    fields.m_side = side;
    fields.m_destination = destination;
    fields.m_quantity = quantity;
    fields.m_timeInForce = TimeInForce{TimeInForce::Type::DAY};
    return fields;
  }

  inline OrderFields OrderFields::BuildMarketOrder(const Security& security,
      CurrencyId currency, Side side, const std::string& destination,
      Quantity quantity) {
    return BuildMarketOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      currency, side, destination, quantity);
  }

  inline OrderFields OrderFields::BuildMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side,
      const std::string& destination, Quantity quantity) {
    return BuildMarketOrder(account, security, CurrencyId::NONE(), side,
      destination, quantity);
  }

  inline OrderFields OrderFields::BuildMarketOrder(const Security& security,
      Side side, const std::string& destination, Quantity quantity) {
    return BuildMarketOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      CurrencyId::NONE(), side, destination, quantity);
  }

  inline OrderFields OrderFields::BuildMarketOrder(const Security& security,
      CurrencyId currency, Side side, Quantity quantity) {
    return BuildMarketOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      currency, side, {}, quantity);
  }

  inline OrderFields OrderFields::BuildMarketOrder(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Security& security, Side side, Quantity quantity) {
    return BuildMarketOrder(account, security, CurrencyId::NONE(), side, {},
      quantity);
  }

  inline OrderFields OrderFields::BuildMarketOrder(const Security& security,
      Side side, Quantity quantity) {
    return BuildMarketOrder(Beam::ServiceLocator::DirectoryEntry{}, security,
      CurrencyId::NONE(), side, {}, quantity);
  }

  inline OrderFields::OrderFields()
      : m_type{OrderType::NONE},
        m_side{Side::NONE},
        m_quantity{0},
        m_price{Money::ZERO},
        m_timeInForce{TimeInForce::Type::GTC} {}

  inline bool OrderFields::operator <(const OrderFields& rhs) const {
    return m_type < rhs.m_type || (m_type == rhs.m_type &&
      ((m_side == Side::BID && m_price < rhs.m_price) ||
       (m_side == Side::ASK && rhs.m_price < m_price)));
  }

  inline bool OrderFields::operator ==(const OrderFields& rhs) const {
    return boost::tie(m_account, m_security, m_currency, m_type, m_side,
      m_destination, m_quantity, m_price, m_timeInForce, m_additionalFields) ==
      boost::tie(rhs.m_account, rhs.m_security, rhs.m_currency, rhs.m_type,
      rhs.m_side, rhs.m_destination, rhs.m_quantity, rhs.m_price,
      rhs.m_timeInForce, rhs.m_additionalFields);
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::OrderExecutionService::OrderFields> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::OrderExecutionService::OrderFields& value,
        unsigned int version) {
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("security", value.m_security);
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("type", value.m_type);
      shuttle.Shuttle("side", value.m_side);
      shuttle.Shuttle("destination", value.m_destination);
      shuttle.Shuttle("quantity", value.m_quantity);
      shuttle.Shuttle("price", value.m_price);
      shuttle.Shuttle("time_in_force", value.m_timeInForce);
      shuttle.Shuttle("additional_fields", value.m_additionalFields);
    }
  };
}
}

#endif
