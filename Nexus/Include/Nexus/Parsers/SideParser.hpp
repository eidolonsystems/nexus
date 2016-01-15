#ifndef NEXUS_SIDEPARSER_HPP
#define NEXUS_SIDEPARSER_HPP
#include <Beam/Collections/EnumIterator.hpp>
#include <Beam/Parsers/EnumeratorParser.hpp>
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /*! \class SideParser
      \brief Matches a Side.
   */
  class SideParser : public Beam::Parsers::EnumeratorParser<Side> {
    public:

      //! Constructs a SideParser.
      SideParser();
  };

  inline SideParser::SideParser()
      : Beam::Parsers::EnumeratorParser<Side>(begin(Beam::MakeRange<Side>()),
          end(Beam::MakeRange<Side>()),
          static_cast<std::string (*)(Side)>(ToString)) {}
}

#endif
