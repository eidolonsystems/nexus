#ifndef SPIRE_SERVICES_SECURITY_INPUT_MODEL_HPP
#define SPIRE_SERVICES_SECURITY_INPUT_MODEL_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Nexus/MarketDataService/VirtualMarketDataClient.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/SecurityInput/SecurityInputModel.hpp"

namespace Spire {

  //! Implements the security input model using remote service calls.
  class ServicesSecurityInputModel final : public SecurityInputModel {
    public:

      //! Constructs a services security input model.
      /*!
        \param client The client to submit autocomplete requests to.
      */
      explicit ServicesSecurityInputModel(
        Beam::Ref<Nexus::MarketDataService::VirtualMarketDataClient> client);

      QtPromise<std::vector<Nexus::SecurityInfo>> autocomplete(
        const std::string& query) override;

    private:
      Nexus::MarketDataService::VirtualMarketDataClient* m_client;
  };
}

#endif
