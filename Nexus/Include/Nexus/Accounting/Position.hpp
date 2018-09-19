#ifndef NEXUS_POSITION_HPP
#define NEXUS_POSITION_HPP
#include <cstdint>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/functional/hash.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus::Accounting {
namespace Details {

  /** Identifies an inventory managed in a specific Currency. */
  template<typename IndexType>
  struct Key {

    //! The inventory's index.
    IndexType m_index;

    //! The Currency used to value the inventory.
    CurrencyId m_currency;

    //! Constructs an empty Key.
    Key() = default;

    //! Constructs a Key.
    /*!
      \param index The inventory's index.
      \param currency The Currency used to value the inventory.
    */
    Key(const IndexType& index, CurrencyId currency);

    //! Tests two Keys for equality.
    /*!
      \param key The Key to test for equality.
      \return <code>true</code> iff <i>key</i> has the same Index and Currency.
    */
    bool operator ==(const Key& key) const;
  };

  template<typename IndexType>
  Key<IndexType>::Key(const IndexType& index, CurrencyId currency)
      : m_index(index),
        m_currency{currency} {}

  template<typename IndexType>
  bool Key<IndexType>::operator ==(const Key& key) const {
    return m_index == key.m_index && m_currency == key.m_currency;
  }
}

  /** Stores information about a single Inventory position.
      \tparam IndexType Used to identify the Position.
   */
  template<typename IndexType>
  struct Position {

    //! Used to identify the Position.
    using Index = IndexType;

    //! The type used to uniquely identifier this Position.
    using Key = Details::Key<Index>;

    //! Uniquely identifies this Position.
    Key m_key;

    //! The quantity of inventory held.
    Quantity m_quantity;

    //! The total cost of the currently held inventory.
    Money m_costBasis;

    //! Constructs a default Position.
    Position();

    //! Constructs a Position.
    Position(const Key& key);
  };

  //! Returns the average price of a Position.
  /*!
    \param position The Position to measure.
    \return The average price of the <i>position</i>.
  */
  template<typename IndexType>
  Money GetAveragePrice(const Position<IndexType>& position) {
    if(position.m_quantity == 0) {
      return Money::ZERO;
    }
    return position.m_costBasis / position.m_quantity;
  }

  //! Returns the Position's Side.
  /*!
    \param position The Position to measure.
    \return The Side corresponding to the <i>position</i>.
  */
  template<typename IndexType>
  Side GetSide(const Position<IndexType>& position) {
    if(position.m_quantity == 0) {
      return Side::NONE;
    } else if(position.m_quantity > 0) {
      return Side::BID;
    }
    return Side::ASK;
  }

  template<typename IndexType>
  Position<IndexType>::Position()
      : m_quantity(0) {}

  template<typename IndexType>
  Position<IndexType>::Position(const Key& key)
      : m_quantity(0),
        m_key(key) {}
}

namespace Beam::Serialization {
  template<typename IndexType>
  struct Shuttle<Nexus::Accounting::Details::Key<IndexType>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::Details::Key<IndexType>& value,
        unsigned int version) {
      shuttle.Shuttle("index", value.m_index);
      shuttle.Shuttle("currency", value.m_currency);
    }
  };

  template<typename IndexType>
  struct Shuttle<Nexus::Accounting::Position<IndexType>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::Accounting::Position<IndexType>& value, unsigned int version) {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("quantity", value.m_quantity);
      shuttle.Shuttle("cost_basis", value.m_costBasis);
    }
  };
}

namespace std {
  template <typename IndexType>
  struct hash<Nexus::Accounting::Details::Key<IndexType>> {
    size_t operator()(const Nexus::Accounting::Details::Key<
        IndexType>& value) const {
      std::size_t seed = 0;
      boost::hash_combine(seed, value.m_index);
      boost::hash_combine(seed, value.m_currency);
      return seed;
    }
  };
}

#endif
