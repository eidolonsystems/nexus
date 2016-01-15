#ifndef NEXUS_FUNCTIONEVALUATORS_HPP
#define NEXUS_FUNCTIONEVALUATORS_HPP
#include <Beam/Queries/FunctionEvaluatorNode.hpp>
#include <Beam/Queries/StandardFunctionExpressions.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus {
namespace Queries {
  struct AdditionExpressionTranslator {
    template<typename T0, typename T1>
    struct Operation {
      auto operator()(T0 left, T1 right) const {
        return left + right;
      }
    };

    using SupportedTypes = boost::mpl::list<
      boost::mpl::vector<Quantity, Quantity>,
      boost::mpl::vector<Money, Money>>;
  };
}
}

#endif
