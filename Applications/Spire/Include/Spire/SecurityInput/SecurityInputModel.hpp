#ifndef SPIRE_SECURITY_INPUT_MODEL_HPP
#define SPIRE_SECURITY_INPUT_MODEL_HPP
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Spire/QtPromise.hpp"

namespace Spire {

  //! Represents a searchable database of available securities.
  class SecurityInputModel : private boost::noncopyable {
    public:
      virtual ~SecurityInputModel() = default;

      //! Autocompletes a partial search string for a security. The search
      //! query can be a company name or ticker symbol.
      virtual QtPromise<std::vector<Nexus::SecurityInfo>> autocomplete(
        const std::string& query) = 0;
  };
}

#endif