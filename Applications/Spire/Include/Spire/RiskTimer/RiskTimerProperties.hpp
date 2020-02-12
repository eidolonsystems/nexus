#ifndef SPIRE_RISKTIMERPROPERTIES_HPP
#define SPIRE_RISKTIMERPROPERTIES_HPP
#include <Beam/Pointers/Out.hpp>
#include <QPoint>
#include "Spire/RiskTimer/RiskTimer.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"

namespace Spire {

  /*! \class RiskTimerProperties
      \brief Stores the properties used across RiskTimer classes.
    */
  class RiskTimerProperties {
    public:

      //! Returns the default RiskTimerProperties.
      static RiskTimerProperties GetDefault();

      //! Loads the RiskTimerProperties from a UserProfile.
      /*!
        \param userProfile The UserProfile to load the properties from.
      */
      static void Load(Beam::Out<UserProfile> userProfile);

      //! Saves a UserProfile's RiskTimerProperties.
      /*!
        \param userProfile The UserProfile's properties to save.
      */
      static void Save(const UserProfile& userProfile);

      //! Constructs an uninitialized RiskTimerProperties.
      RiskTimerProperties();

      //! Returns the initial position of the RiskTimerDialog.
      const QPoint& GetRiskTimerDialogInitialPosition() const;

      //! Sets the initial position of the RiskTimerDialog.
      /*!
        \param position The initial position to use for the RiskTimerDialog.
      */
      void SetRiskTimerDialogInitialPosition(const QPoint& position);

    private:
      friend struct Beam::Serialization::DataShuttle;
      QPoint m_riskTimerDialogInitialPosition;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void RiskTimerProperties::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("risk_timer_dialog_initial_position",
      m_riskTimerDialogInitialPosition);
  }
}

#endif