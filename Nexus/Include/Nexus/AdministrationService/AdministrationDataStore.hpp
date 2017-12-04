#ifndef NEXUS_ADMINISTRATIONDATASTORE_HPP
#define NEXUS_ADMINISTRATIONDATASTORE_HPP
#include <Beam/IO/Connection.hpp>
#include <Beam/ServiceLocator/ServiceLocator.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class AdministrationDataStore
      \brief Base class used to store Nexus account info.
   */
  class AdministrationDataStore : private boost::noncopyable {
    public:
      virtual ~AdministrationDataStore() = default;

      //! Loads all AccountIdentities.
      virtual std::vector<
        std::tuple<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>>
        LoadAllAccountIdentities() = 0;

      //! Loads an AccountIdentity.
      /*!
        \param account The account whose identity is to be loaded.
        \return The AccountIdentity of the specified <i>account</i>.
      */
      virtual AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      //! Stores an AccountIdentity.
      /*!
        \param account The account of the identity to store.
        \param identity The AccountIdentity to store.
      */
      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) = 0;

      //! Loads all RiskParameters.
      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters>> LoadAllRiskParameters() = 0;

      //! Loads an account's RiskParameters.
      /*!
        \param account The account whose RiskParameters are to be loaded.
        \return The <i>account</i>'s RiskParameters.
      */
      virtual RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      //! Stores an account's RiskParameters.
      /*!
        \param account The account of the RiskParameters to store.
        \param riskParameters The RiskParameters to store for the
               <i>account</i>.
      */
      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) = 0;

      //! Loads all RiskStates.
      virtual std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState>> LoadAllRiskStates() = 0;

      //! Loads an account's RiskState.
      /*!
        \param account The account whose RiskState is to be loaded.
        \return The <i>account</i>'s RiskState.
      */
      virtual RiskService::RiskState LoadRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account) = 0;

      //! Stores an account's RiskState.
      /*!
        \param account The account of the RiskState to store.
        \param riskState The RiskState to store for the <i>account</i>.
      */
      virtual void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) = 0;

      //! Loads an AccountModificationRequest.
      /*!
        \param id The id of the request to load.
        \return The AccountModificationRequest with the specified <i>id</i>.
      */
      virtual AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) = 0;

      //! Loads all account modification request ids to modify an account.
      /*!
        \param account The account being modified.
        \param startId The id of the most recent request to load (exclusive), or
               -1 to start with the most recent request.
        \param maxCount The maximum number of ids to load.
        \return The list of ids specified.
      */
      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) = 0;

      //! Loads all account modification request ids.
      /*!
        \param startId The id of the most recent request to load (exclusive), or
               -1 to start with the most recent request.
        \param maxCount The maximum number of ids to load.
        \return The list of ids specified.
      */
      virtual std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        AccountModificationRequest::Id startId, int maxCount) = 0;

      //! Loads an EntitlementModification.
      /*!
        \param id The id of the request to load.
        \return The EntitlementModification with the specified <i>id</i>.
      */
      virtual EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) = 0;

      //! Stores an EntitlementModification.
      /*!
        \param request The modification request.
        \param modification The details of the modification.
      */
      virtual void Store(const AccountModificationRequest& request,
        const EntitlementModification& modification) = 0;

      //! Stores a Message associated with an account modification request.
      /*!
        \param id The id of the request.
        \param message The Message to store.
      */
      virtual void Store(AccountModificationRequest::Id id,
        const Message& message) = 0;

      //! Loads the status of an AccountModificationRequest.
      /*!
        \param id The id of the request.
        \return The status of the request with the specified <i>id</i>.
      */
      virtual AccountModificationRequest::Status
        LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) = 0;

      //! Stores the status of an AccountModificationRequest.
      /*!
        \param id The id of the request.
        \param status The status of the request.
      */
      virtual void Store(AccountModificationRequest::Id id,
        AccountModificationRequest::Status status) = 0;

      //! Loads the id of the last Message stored, or 0 if there are no stored
      //! Messages.
      virtual Message::Id LoadLastMessageId() = 0;

      //! Performs an atomic transaction.
      /*!
        \param transaction The transaction to perform.
      */
      virtual void WithTransaction(
        const std::function<void ()>& transaction) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;
  };
}
}

#endif
