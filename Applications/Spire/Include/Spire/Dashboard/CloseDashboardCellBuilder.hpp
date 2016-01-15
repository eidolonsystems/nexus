#ifndef SPIRE_CLOSEDASHBOARDCELLBUILDER_HPP
#define SPIRE_CLOSEDASHBOARDCELLBUILDER_HPP
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCellBuilder.hpp"

namespace Spire {

  /*! \class CloseDashboardCellBuilder
      \brief Builds a DashboardCell emitting a Security's closing price.
   */
  class CloseDashboardCellBuilder : public DashboardCellBuilder {
    public:

      //! Constructs a CloseDashboardCellBuilder.
      CloseDashboardCellBuilder() = default;

      virtual std::unique_ptr<DashboardCell> Build(
        const DashboardCell::Value& index,
        Beam::RefType<UserProfile> userProfile) const;

      virtual std::unique_ptr<DashboardCellBuilder> Clone() const;

    protected:
      friend struct Beam::Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CloseDashboardCellBuilder::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    DashboardCellBuilder::Shuttle(shuttle, version);
  }
}

#endif
