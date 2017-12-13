#ifndef NEXUS_COUNTRY_HPP
#define NEXUS_COUNTRY_HPP
#include <algorithm>
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  //! Identifies a country by a unique code.
  using CountryCode = std::uint16_t;

  /*! \class CountryDatabase
      \brief Stores the database of all countries.
   */
  class CountryDatabase {
    public:

      //! Specifies the code for an invalid country.
      static const CountryCode NONE = ~0;

      /*! \struct Entry
          \brief Stores a single entry in a CountryDatabase.
       */
      struct Entry {

        //! The country's code.
        CountryCode m_code;

        //! The short form name.
        std::string m_name;

        //! The country's two letter code.
        Beam::FixedString<2> m_twoLetterCode;

        //! The country's three letter code.
        Beam::FixedString<3> m_threeLetterCode;
      };

      //! Constructs an empty CountryDatabase.
      CountryDatabase() = default;

      //! Returns the list of countries represented.
      const std::vector<Entry>& GetEntries() const;

      //! Returns the country represented by its numeric code.
      const Entry& FromCode(CountryCode code) const;

      //! Returns the country represented by its name.
      const Entry& FromName(const std::string& name) const;

      //! Returns the country represented by its two letter code.
      const Entry& FromTwoLetterCode(const Beam::FixedString<2>& code) const;

      //! Returns the country represented by its three letter code.
      const Entry& FromThreeLetterCode(const Beam::FixedString<3>& code) const;

      //! Adds an Entry.
      /*!
        \param entry The Entry to add.
      */
      void Add(const Entry& entry);

      //! Deletes an Entry.
      /*!
        \param code The CountryCode of the Entry to delete.
      */
      void Delete(CountryCode code);

    private:
      friend struct Beam::Serialization::Shuttle<CountryDatabase>;
      static Entry MakeNoneEntry();
      template<typename T>
      struct NoneEntry {
        static Entry NONE_ENTRY;
      };
      std::vector<Entry> m_entries;
  };

  //! Tests two CountryDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two CountryDatabase Entries are equal.
  */
  inline bool operator ==(const CountryDatabase::Entry& lhs,
      const CountryDatabase::Entry& rhs) {
    return lhs.m_code == rhs.m_code && lhs.m_name == rhs.m_name &&
      lhs.m_twoLetterCode == rhs.m_twoLetterCode && lhs.m_threeLetterCode ==
      rhs.m_threeLetterCode;
  }

  //! Tests two CountryDatabase Entries for equality.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff the two CountryDatabase Entries are equal.
  */
  inline bool operator !=(const CountryDatabase::Entry& lhs,
      const CountryDatabase::Entry& rhs) {
    return !(lhs == rhs);
  }

  //! Parses a CountryCode from a string.
  /*!
    \param source The string to parse.
    \param countryDatabase The CountryDatabase used to find the CountryCode.
    \return The CountryCode represented by the <i>source</i>.
  */
  inline CountryCode ParseCountryCode(const std::string& source,
      const CountryDatabase& countryDatabase) {
    auto code = countryDatabase.FromTwoLetterCode(source);
    if(code.m_code != CountryDatabase::NONE) {
      return code.m_code;
    }
    code = countryDatabase.FromThreeLetterCode(source);
    if(code.m_code != CountryDatabase::NONE) {
      return code.m_code;
    }
    return countryDatabase.FromName(source).m_code;
  }

  //! Parses a CountryDatabase Entry from a YAML node.
  /*!
    \param node The node to parse the CountryDatabase Entry from.
    \return The CountryDatabase Entry represented by the <i>node</i>.
  */
  inline CountryDatabase::Entry ParseCountryDatabaseEntry(
      const YAML::Node& node) {
    CountryDatabase::Entry entry;
    entry.m_name = Beam::Extract<std::string>(node, "name");
    entry.m_twoLetterCode = Beam::Extract<std::string>(node, "two_letter_code");
    entry.m_threeLetterCode = Beam::Extract<std::string>(node,
      "three_letter_code");
    entry.m_code = Beam::Extract<CountryCode>(node, "code");
    return entry;
  }

  //! Parses a CountryDatabase from a YAML node.
  /*!
    \param node The node to parse the CountryDatabase from.
    \return The CountryDatabase represented by the <i>node</i>.
  */
  inline CountryDatabase ParseCountryDatabase(const YAML::Node& node) {
    CountryDatabase countryDatabase;
    for(auto& entryNode : node) {
      auto entry = ParseCountryDatabaseEntry(entryNode);
      countryDatabase.Add(entry);
    }
    return countryDatabase;
  }

  inline const std::vector<CountryDatabase::Entry>&
      CountryDatabase::GetEntries() const {
    return m_entries;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromCode(
      CountryCode code) const {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_code == code;
      });
    if(entryIterator == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *entryIterator;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromName(
      const std::string& name) const {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_name == name;
      });
    if(entryIterator == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *entryIterator;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromTwoLetterCode(
      const Beam::FixedString<2>& code) const {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_twoLetterCode == code;
      });
    if(entryIterator == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *entryIterator;
  }

  inline const CountryDatabase::Entry& CountryDatabase::FromThreeLetterCode(
      const Beam::FixedString<3>& code) const {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_threeLetterCode == code;
      });
    if(entryIterator == m_entries.end()) {
      return NoneEntry<void>::NONE_ENTRY;
    }
    return *entryIterator;
  }

  inline void CountryDatabase::Add(const Entry& entry) {
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

  inline void CountryDatabase::Delete(CountryCode code) {
    auto entryIterator = std::find_if(m_entries.begin(), m_entries.end(),
      [=] (const Entry& entry) {
        return entry.m_code == code;
      });
    if(entryIterator == m_entries.end()) {
      return;
    }
    m_entries.erase(entryIterator);
  }

  inline CountryDatabase::Entry CountryDatabase::MakeNoneEntry() {
    Entry noneEntry;
    noneEntry.m_twoLetterCode = "??";
    noneEntry.m_threeLetterCode = "???";
    noneEntry.m_name = "None";
    noneEntry.m_code = ~0;
    return noneEntry;
  }

  template<typename T>
  CountryDatabase::Entry CountryDatabase::NoneEntry<T>::NONE_ENTRY =
    CountryDatabase::MakeNoneEntry();
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Nexus::CountryDatabase::Entry> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CountryDatabase::Entry& value,
        unsigned int version) {
      shuttle.Shuttle("code", value.m_code);
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("two_letter_code", value.m_twoLetterCode);
      shuttle.Shuttle("three_letter_code", value.m_threeLetterCode);
    }
  };

  template<>
  struct Shuttle<Nexus::CountryDatabase> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::CountryDatabase& value,
        unsigned int version) {
      shuttle.Shuttle("entries", value.m_entries);
    }
  };
}
}

#endif
