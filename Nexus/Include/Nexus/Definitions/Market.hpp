#ifndef NEXUS_MARKET_HPP
#define NEXUS_MARKET_HPP
#include <algorithm>
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  //! Represents a market.
  typedef Beam::FixedString<4> MarketCode;

  /*! \class MarketDatabase
      \brief Stores the database of all markets.
   */
  class MarketDatabase {
    public:

      /*! \struct Entry
          \brief Stores a single entry in a MarketDatabase.
       */
      struct Entry {

        //! The market identifier code.
        MarketCode m_code;

        //! The market's country code.
        CountryCode m_countryCode;

        //! The market's time zone.
        std::string m_timeZone;

        //! The default currency used.
        CurrencyId m_currency;

        //! The size of a board lot.
        Quantity m_boardLot;

        //! The institution's description.
        std::string m_description;

        //! The common display name.
        std::string m_displayName;
      };

      //! Returns an Entry representing no market.
      static const Entry& GetNoneEntry();

      //! Constructs an empty MarketDatabase.
      MarketDatabase() = default;

      //! Returns all Entries.
      const std::vector<Entry>& GetEntries() const;

      //! Returns an Entry from its MarketCode.
      /*!
        \param code The MarketCode to lookup.
        \return The Entry with the specified <i>code</i>.
      */
      const Entry& FromCode(MarketCode code) const;

      //! Returns an Entry from its display name.
      /*!
        \param displayName The market's display name.
        \return The Entry with the specified <i>displayName</i>.
      */
      const Entry& FromDisplayName(const std::string& displayName) const;

      //! Returns all Entries participating in a specified country.
      /*!
        \param country The CountryCode to lookup.
        \return The list of all Entries with the specified <i>country</i>.
      */
      std::vector<Entry> FromCountry(CountryCode country) const;

      //! Adds an Entry.
      /*!
        \param entry The Entry to add.
      */
      void Add(const Entry& entry);

      //! Deletes an Entry.
      /*!
        \param code The MarketCode of the Entry to delete.
      */
      void Delete(const MarketCode& code);

    private:
      friend struct Beam::Serialization::Shuttle<MarketDatabase>;
      static Entry MakeNoneEntry();
      std::vector<Entry> m_entries;
  };

  //! Returns the time of the start of day relative to a specified market in
  //! UTC.
  /*!
    \param marketCode The market whose start of day in UTC is to be returned.
    \param dateTime The date/time, in UTC, to convert into the market's start
           of day, in UTC.
    \param marketDatabase The MarketDatabase to use for time zone info.
    \param timeZoneDatabase The time zone database to use for time zone
           conversions.
    \return Takes the date represented by <i>dateTime</i>, converts it into the
            market's local time, truncates the time of day so that the time of
            day is the beginning of the day in that market's local time, then
            converts that value back to UTC.
  */
  inline boost::posix_time::ptime MarketDateToUtc(MarketCode marketCode,
      const boost::posix_time::ptime& dateTime,
      const MarketDatabase& marketDatabase,
      const boost::local_time::tz_database& timeZoneDatabase) {
    boost::local_time::time_zone_ptr marketTimeZone =
      timeZoneDatabase.time_zone_from_region(
      marketDatabase.FromCode(marketCode).m_timeZone);
    boost::local_time::time_zone_ptr utcTimeZone =
      timeZoneDatabase.time_zone_from_region("UTC");
    boost::local_time::local_date_time universalDateTime(dateTime.date(),
      dateTime.time_of_day(), utcTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    boost::local_time::local_date_time currentMarketDateTime =
      universalDateTime.local_time_in(marketTimeZone);
    boost::posix_time::ptime marketCutOffDateTime(
      currentMarketDateTime.local_time().date(),
      boost::posix_time::seconds(0));
    boost::local_time::local_date_time marketCutOffLocalDateTime(
      marketCutOffDateTime.date(), marketCutOffDateTime.time_of_day(),
      marketTimeZone,
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    boost::local_time::local_date_time universalCutOffTime =
      marketCutOffLocalDateTime.local_time_in(utcTimeZone);
    boost::posix_time::ptime utcMarketDate(
      universalCutOffTime.local_time().date(),
      universalCutOffTime.local_time().time_of_day());
    return utcMarketDate;
  }

  //! Parses a MarketCode from a string.
  /*!
    \param source The string to parse.
    \param marketDatabase The MarketDatabase containing the available
           MarketCodes.
    \return The MarketCode represented by the <i>source</i>.
  */
  inline MarketCode ParseMarketCode(const std::string& source,
      const MarketDatabase& marketDatabase) {
    auto marketEntry = &marketDatabase.FromDisplayName(source);
    if(marketEntry->m_code == MarketCode{}) {
      marketEntry = &marketDatabase.FromCode(source);
      if(marketEntry->m_code == MarketCode{}) {
        return MarketCode{};
      }
    }
    return marketEntry->m_code;
  }

  //! Parses a MarketEntry from a string.
  /*!
    \param source The string to parse.
    \param marketDatabase The MarketDatabase containing the available
           MarketEntry.
    \return The MarketCode represented by the <i>source</i>.
  */
  inline const MarketDatabase::Entry& ParseMarketEntry(
      const std::string& source, const MarketDatabase& marketDatabase) {
    auto marketEntry = &marketDatabase.FromDisplayName(source);
    if(marketEntry->m_code == MarketCode{}) {
      marketEntry = &marketDatabase.FromCode(source);
      if(marketEntry->m_code == MarketCode{}) {
        return MarketDatabase::GetNoneEntry();
      }
    }
    return *marketEntry;
  }

  //! Parses a MarketDatabase Entry from a YAML node.
  /*!
    \param node The node to parse the MarketDatabase Entry from.
    \return The MarketDatabase Entry represented by the <i>node</i>.
  */
  inline MarketDatabase::Entry ParseMarketDatabaseEntry(
      const YAML::Node& node, const CountryDatabase& countryDatabase,
      const CurrencyDatabase& currencyDatabase) {
    MarketDatabase::Entry entry;
    entry.m_code = Beam::Extract<std::string>(node, "code");
    entry.m_countryCode = ParseCountryCode(
      Beam::Extract<std::string>(node, "country_code"), countryDatabase);
    if(entry.m_countryCode == CountryDatabase::NONE) {
      BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException(
        "Invalid country code.", node.GetMark()));
    }
    entry.m_timeZone = Beam::Extract<std::string>(node, "time_zone");
    entry.m_currency = ParseCurrency(
      Beam::Extract<std::string>(node, "currency"), currencyDatabase);
    if(entry.m_currency == CurrencyId::NONE()) {
      BOOST_THROW_EXCEPTION(Beam::MakeYamlParserException("Invalid currency.",
        node.GetMark()));
    }
    entry.m_boardLot = Beam::Extract<int>(node, "board_lot");
    entry.m_description = Beam::Extract<std::string>(node, "description");
    entry.m_displayName = Beam::Extract<std::string>(node, "display_name");
    return entry;
  }

  //! Parses a MarketDatabase from a YAML node.
  /*!
    \param node The node to parse the MarketDatabase from.
    \param countryDatabase The CountryDatabase used to parse country codes.
    \param currencyDatabase The CurrencyDatabase used to parse currencies.
    \return The MarketDatabase represented by the <i>node</i>.
  */
  inline MarketDatabase ParseMarketDatabase(const YAML::Node& node,
      const CountryDatabase& countryDatabase,
      const CurrencyDatabase& currencyDatabase) {
    MarketDatabase marketDatabase;
    for(auto& entryNode : node) {
      auto entry = ParseMarketDatabaseEntry(entryNode, countryDatabase,
        currencyDatabase);
      marketDatabase.Add(entry);
    }
    return marketDatabase;
  }

  //! Tests two MarketDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two MarketDatabase Entries are equal.
  */
  inline bool operator ==(const MarketDatabase::Entry& lhs,
      const MarketDatabase::Entry& rhs) {
    return lhs.m_code == rhs.m_code && lhs.m_countryCode == rhs.m_countryCode &&
      lhs.m_timeZone == rhs.m_timeZone && lhs.m_currency == rhs.m_currency &&
      lhs.m_boardLot == rhs.m_boardLot &&
      lhs.m_description == rhs.m_description &&
      lhs.m_displayName == rhs.m_displayName;
  }

  //! Tests two MarketDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two MarketDatabase Entries are equal.
  */
  inline bool operator !=(const MarketDatabase::Entry& lhs,
      const MarketDatabase::Entry& rhs) {
    return !(lhs == rhs);
  }

  inline const MarketDatabase::Entry& MarketDatabase::GetNoneEntry() {
    static auto NONE = MakeNoneEntry();
    return NONE;
  }

  inline const std::vector<MarketDatabase::Entry>&
      MarketDatabase::GetEntries() const {
    return m_entries;
  }

  inline const MarketDatabase::Entry& MarketDatabase::FromCode(
      MarketCode code) const {
    Entry comparator;
    comparator.m_code = code;
    auto entryIterator = std::lower_bound(m_entries.begin(), m_entries.end(),
      comparator,
      [] (const Entry& lhs, const Entry& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(entryIterator != m_entries.end() && entryIterator->m_code == code) {
      return *entryIterator;
    }
    return GetNoneEntry();
  }

  inline const MarketDatabase::Entry& MarketDatabase::FromDisplayName(
      const std::string& displayName) const {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [&] (const Entry& entry) {
        return entry.m_displayName == displayName;
      });
    if(entryIterator == m_entries.end()) {
      return GetNoneEntry();
    }
    return *entryIterator;
  }

  inline std::vector<MarketDatabase::Entry> MarketDatabase::FromCountry(
      CountryCode country) const {
    std::vector<MarketDatabase::Entry> entries;
    for(auto i = m_entries.begin(); i != m_entries.end(); ++i) {
      if(i->m_countryCode == country) {
        entries.push_back(*i);
      }
    }
    return entries;
  }

  inline void MarketDatabase::Add(const Entry& entry) {
    auto entryIterator = std::lower_bound(m_entries.begin(), m_entries.end(),
      entry,
      [] (const Entry& lhs, const Entry& rhs) {
        return lhs.m_code < rhs.m_code;
      });
    if(entryIterator == m_entries.end() ||
        entryIterator->m_code != entry.m_code) {
      m_entries.insert(entryIterator, entry);
    }
  }

  inline void MarketDatabase::Delete(const MarketCode& code) {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_code == code;
      });
    if(entryIterator == m_entries.end()) {
      return;
    }
    m_entries.erase(entryIterator);
  }

  inline MarketDatabase::Entry MarketDatabase::MakeNoneEntry() {
    Entry noneEntry;
    noneEntry.m_code = MarketCode();
    noneEntry.m_countryCode = CountryDatabase::NONE;
    noneEntry.m_timeZone = "UTC";
    noneEntry.m_description = "None";
    return noneEntry;
  }
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::MarketDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::MarketDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("country_code", value.m_countryCode);
      shuttle.Shuttle("time_zone", value.m_timeZone);
      shuttle.Shuttle("currency", value.m_currency);
      shuttle.Shuttle("board_lot", value.m_boardLot);
      shuttle.Shuttle("description", value.m_description);
      shuttle.Shuttle("display_name", value.m_displayName);
    }
  };

  template<>
  struct Shuttle<Nexus::MarketDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::MarketDatabase& value,
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
    }
  };
}
}

#endif
