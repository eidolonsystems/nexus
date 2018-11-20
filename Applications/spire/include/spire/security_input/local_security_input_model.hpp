#ifndef SPIRE_LOCAL_SECURITY_INPUT_MODEL_HPP
#define SPIRE_LOCAL_SECURITY_INPUT_MODEL_HPP
#include <Beam/Utilities/Trie.hpp>
#include "spire/security_input/security_input.hpp"
#include "spire/security_input/security_input_model.hpp"

namespace Spire {

  //! Implements the security input model using a local database.
  class LocalSecurityInputModel final : public SecurityInputModel {
    public:

      //! Constructs an empty local security input model.
      LocalSecurityInputModel();

      //! Adds a security to this model.
      /*!
        \param security The security entry to add.
      */
      void add(Nexus::SecurityInfo security);

      QtPromise<std::vector<Nexus::SecurityInfo>> autocomplete(
        const std::string& query) override;

    private:
      rtv::Trie<char, Nexus::SecurityInfo> m_securities;
  };
}

#endif
