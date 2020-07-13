#ifndef NEXUS_BBO_QUOTE_HPP
#define NEXUS_BBO_QUOTE_HPP
#include <ostream>
#include <Beam/Serialization/DataShuttle.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "Nexus/Definitions/Quote.hpp"

namespace Nexus {

  /** Stores the best bid and ask Quotes across all markets. */
  struct BboQuote {

    /** The best bid. */
    Quote m_bid;

    /** The best ask. */
    Quote m_ask;

    /** The time of the transaction. */
    boost::posix_time::ptime m_timestamp;

    /** Constructs an uninitialized BboQuote. */
    BboQuote();

    /**
     * Constructs a BboQuote.
     * @param bid The best bid.
     * @param ask The best ask.
     * @param timestamp The time of the transaction.
     */
    BboQuote(Quote bid, Quote ask, boost::posix_time::ptime timestamp);

    /**
     * Tests if two BboQuotes are equal.
     * @param rhs The right hand side of the equality.
     * @return <code>true</code> iff <i>this</i> is equal to <i>rhs</i>.
     */
    bool operator ==(const BboQuote& rhs) const;

    /**
     * Tests if two BboQuotes are not equal.
     * @param rhs The right hand side of the equality.
     * @return <code>true</code> iff <i>this</i> is not equal to <i>rhs</i>.
     */
    bool operator !=(const BboQuote& rhs) const;
  };

  inline std::ostream& operator <<(std::ostream& out, const BboQuote& value) {
    return out << "(" << value.m_bid << " " << value.m_ask << " " <<
      value.m_timestamp << ")";
  }

  inline BboQuote::BboQuote() {
    m_bid.m_side = Side::BID;
    m_ask.m_side = Side::ASK;
  }

  inline BboQuote::BboQuote(Quote bid, Quote ask,
      boost::posix_time::ptime timestamp)
      : m_bid(std::move(bid)),
        m_ask(std::move(ask)),
        m_timestamp(timestamp) {
    assert(m_bid.m_side == Side::BID);
    assert(m_ask.m_side == Side::ASK);
  }

  inline bool BboQuote::operator ==(const BboQuote& rhs) const {
    return m_bid == rhs.m_bid && m_ask == rhs.m_ask &&
      m_timestamp == rhs.m_timestamp;
  }

  inline bool BboQuote::operator !=(const BboQuote& rhs) const {
    return !(*this == rhs);
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::BboQuote> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::BboQuote& value,
        unsigned int version) {
      shuttle.Shuttle("bid", value.m_bid);
      shuttle.Shuttle("ask", value.m_ask);
      shuttle.Shuttle("timestamp", value.m_timestamp);
    }
  };
}

#endif
