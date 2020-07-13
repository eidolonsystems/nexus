#ifndef NEXUS_REGION_HPP
#define NEXUS_REGION_HPP
#include <string>
#include <unordered_set>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleUnorderedSet.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /** Represents a group of trading locations. */
  class Region {
    public:

      /** Returns the global Region. */
      static Region Global();

      /**
       * Constructs a global named Region.
       * @param name The name of the Region.
       */
      static Region Global(std::string name);

      /** Constructs an empty Region. */
      Region();

      /**
       * Constructs an empty named Region.
       * @param name The name of the Region.
       */
      explicit Region(std::string name);

      /**
       * Constructs a Region consisting of a single country.
       * @param country The country to represent.
       */
      Region(CountryCode country);

      /**
       * Constructs a Region consisting of a single market.
       * @param market The market to represent.
       */
      Region(const MarketDatabase::Entry& market);

      /**
       * Constructs a Region consisting of a single Security.
       * @param security The Security to represent.
       */
      Region(Security security);

      /** Returns the name of this Region. */
      const std::string& GetName() const;

      /** Sets the name of this Region. */
      void SetName(const std::string& name);

      /** Returns <code>true</code> iff this is the global Region. */
      bool IsGlobal() const;

      /** Returns the countries in this Region. */
      const std::unordered_set<CountryCode>& GetCountries() const;

      /** Returns the Securities in this Region. */
      const std::unordered_set<Security>& GetSecurities() const;

      /**
       * Combines <i>this</i> Region with another.
       * @param region The Region to combine.
       * @return A Region containing all of <i>this</i>'s elements and
       *         <i>region</i>'s elements.
       */
      Region operator +(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a strict subset of
       * another.
       */
      bool operator <(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a subset of
       * another.
       */
      bool operator <=(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is equal to another.
       */
      bool operator ==(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is not equal to
       * another.
       */
      bool operator !=(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a superset of
       * another.
       */
      bool operator >=(const Region& region) const;

      /**
       * Returns <code>true</code> iff <i>this</i> Region is a strict superset
       * of another.
       */
      bool operator >(const Region& region) const;

    private:
      struct GlobalTag {};
      struct MarketEntry {
        MarketCode m_market;
        CountryCode m_country;

        MarketEntry() = default;
        MarketEntry(MarketCode market, CountryCode country);
        bool operator ==(const MarketEntry& marketEntry) const;
        friend struct Beam::Serialization::Shuttle<MarketEntry>;
      };
      struct MarketEntryHash {
        std::size_t operator ()(const MarketEntry& marketEntry) const;
      };
      friend struct Beam::Serialization::Shuttle<Region>;
      friend struct Beam::Serialization::Shuttle<Region::MarketEntry>;
      std::string m_name;
      bool m_isGlobal;
      std::unordered_set<CountryCode> m_countries;
      std::unordered_set<MarketEntry, MarketEntryHash> m_markets;
      std::unordered_set<Security> m_securities;

      explicit Region(GlobalTag);
      Region(GlobalTag, std::string name);
  };

  inline Region::MarketEntry::MarketEntry(MarketCode market,
    CountryCode country)
    : m_market(market),
      m_country(country) {}

  inline bool Region::MarketEntry::operator ==(
      const MarketEntry& marketEntry) const {
    return m_market == marketEntry.m_market;
  }

  inline std::size_t Region::MarketEntryHash::operator ()(
      const Region::MarketEntry& value) const {
    return std::hash<MarketCode>()(value.m_market);
  }

  inline Region Region::Global() {
    return Region(GlobalTag{});
  }

  inline Region Region::Global(std::string name) {
    return Region(GlobalTag{}, std::move(name));
  }

  inline Region::Region()
    : m_isGlobal(false) {}

  inline Region::Region(std::string name)
    : m_isGlobal(false),
      m_name(std::move(name)) {}

  inline Region::Region(CountryCode country)
      : m_isGlobal(false) {
    m_countries.insert(country);
  }

  inline Region::Region(const MarketDatabase::Entry& market)
      : m_isGlobal(false) {
    m_markets.insert(MarketEntry(market.m_code, market.m_countryCode));
  }

  inline Region::Region(Security security)
      : m_isGlobal(false) {
    m_securities.insert(std::move(security));
  }

  inline const std::string& Region::GetName() const {
    return m_name;
  }

  inline void Region::SetName(const std::string& name) {
    m_name = name;
  }

  inline bool Region::IsGlobal() const {
    return m_isGlobal;
  }

  inline const std::unordered_set<CountryCode>& Region::GetCountries() const {
    return m_countries;
  }

  inline const std::unordered_set<Security>& Region::GetSecurities() const {
    return m_securities;
  }

  inline Region Region::operator +(const Region& region) const {
    if(m_isGlobal) {
      return *this;
    } else if(region.m_isGlobal) {
      return region;
    }
    auto unionRegion = *this;
    unionRegion.m_countries.insert(region.m_countries.begin(),
      region.m_countries.end());
    unionRegion.m_markets.insert(region.m_markets.begin(),
      region.m_markets.end());
    unionRegion.m_securities.insert(region.m_securities.begin(),
      region.m_securities.end());
    return unionRegion;
  }

  inline bool Region::operator <(const Region& region) const {
    return (*this <= region) && *this != region;
  }

  inline bool Region::operator <=(const Region& region) const {
    if(region.m_isGlobal) {
      return true;
    } else if(m_isGlobal) {
      return false;
    }
    auto leftOverSecurities = std::unordered_set<Security>();
    for(auto& security : m_securities) {
      if(region.m_securities.find(security) == region.m_securities.end()) {
        leftOverSecurities.insert(security);
      }
    }
    auto securityIterator = leftOverSecurities.begin();
    while(securityIterator != leftOverSecurities.end()) {
      auto entry = MarketEntry(securityIterator->GetMarket(),
        securityIterator->GetCountry());
      if(region.m_markets.find(entry) != region.m_markets.end()) {
        securityIterator = leftOverSecurities.erase(securityIterator);
      } else {
        ++securityIterator;
      }
    }
    securityIterator = leftOverSecurities.begin();
    while(securityIterator != leftOverSecurities.end()) {
      if(region.m_countries.find(securityIterator->GetCountry()) !=
          region.m_countries.end()) {
        securityIterator = leftOverSecurities.erase(securityIterator);
      } else {
        ++securityIterator;
      }
    }
    if(!leftOverSecurities.empty()) {
      return false;
    }
    auto leftOverMarkets = std::unordered_set<MarketEntry, MarketEntryHash>();
    for(auto& market : m_markets) {
      if(region.m_markets.find(market) == region.m_markets.end()) {
        leftOverMarkets.insert(market);
      }
    }
    auto marketIterator = leftOverMarkets.begin();
    while(marketIterator != leftOverMarkets.end()) {
      if(region.m_countries.find(marketIterator->m_country) !=
          region.m_countries.end()) {
        marketIterator = leftOverMarkets.erase(marketIterator);
      } else {
        ++marketIterator;
      }
    }
    if(!leftOverMarkets.empty()) {
      return false;
    }
    for(auto& country : m_countries) {
      if(region.m_countries.find(country) == region.m_countries.end()) {
        return false;
      }
    }
    return true;
  }

  inline bool Region::operator ==(const Region& region) const {
    return std::tie(m_isGlobal, m_countries, m_markets, m_securities) ==
      std::tie(region.m_isGlobal, region.m_countries, region.m_markets,
      region.m_securities);
  }

  inline bool Region::operator !=(const Region& region) const {
    return !(*this == region);
  }

  inline bool Region::operator >=(const Region& region) const {
    return region <= *this;
  }

  inline bool Region::operator >(const Region& region) const {
    return (*this >= region) && *this != region;
  }

  inline Region::Region(GlobalTag)
    : m_isGlobal(true) {}

  inline Region::Region(GlobalTag, std::string name)
    : m_isGlobal(true),
      m_name(std::move(name)) {}
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::Region::MarketEntry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Region::MarketEntry& value,
        unsigned int version) {
      shuttle.Shuttle("market", value.m_market);
      shuttle.Shuttle("country", value.m_country);
    }
  };

  template<>
  struct Shuttle<Nexus::Region> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Region& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("is_global", value.m_isGlobal);
      shuttle.Shuttle("countries", value.m_countries);
      shuttle.Shuttle("markets", value.m_markets);
      shuttle.Shuttle("securities", value.m_securities);
    }
  };
}

#endif
