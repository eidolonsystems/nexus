#ifndef NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#define NEXUS_SQL_ADMINISTRATION_DATA_STORE_HPP
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Sql/PosixTimeToSqlDateTime.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Utilities/KeyValueCache.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AdministrationDataStore.hpp"
#include "Nexus/AdministrationService/AdministrationDataStoreException.hpp"
#include "Nexus/AdministrationService/SqlDefinitions.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus::AdministrationService {

  /** Stores account administration data in a MySQL database.
      \tparam C The type of SQL connection.
   */
  template<typename C>
  class SqlAdministrationDataStore : public AdministrationDataStore {
    public:

      //! The type of SQL connection.
      using Connection = C;

      //! The function used to load DirectoryEntries.
      using DirectoryEntrySourceFunction = Beam::KeyValueCache<unsigned int,
        Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex>::SourceFunction;

      //! Constructs an SqlAdministrationDataStore.
      /*!
        \param connection The connection to the SQL database.
        \param directoryEntrySourceFunction The function used to load
               DirectoryEntries.
      */
      SqlAdministrationDataStore(std::unique_ptr<Connection> connection,
        const DirectoryEntrySourceFunction& directoryEntrySourceFunction);

      ~SqlAdministrationDataStore() override;

      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        AccountIdentity>> LoadAllAccountIdentities() override;

      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskParameters>> LoadAllRiskParameters() override;

      RiskService::RiskParameters LoadRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
        RiskService::RiskState>> LoadAllRiskStates() override;

      RiskService::RiskState LoadRiskState(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void Store(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override;

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        AccountModificationRequest::Id startId, int maxCount) override;

      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override;

      void Store(const AccountModificationRequest& request,
        const EntitlementModification& modification) override;

      RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) override;

      void Store(const AccountModificationRequest& request,
        const RiskModification& modification) override;

      void Store(AccountModificationRequest::Id id,
        const Message& message) override;

      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      void Store(AccountModificationRequest::Id id,
        const AccountModificationRequest::Update& status) override;

      Message::Id LoadLastMessageId() override;

      Message LoadMessage(Message::Id id) override;

      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      void WithTransaction(const std::function<void ()>& transaction) override;

      void Open() override;

      void Close() override;

    private:
      mutable Beam::Threading::Mutex m_mutex;
      std::unique_ptr<Connection> m_connection;
      Beam::KeyValueCache<unsigned int, Beam::ServiceLocator::DirectoryEntry,
        Beam::Threading::Mutex> m_directoryEntries;
      Beam::IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename C>
  SqlAdministrationDataStore<C>::SqlAdministrationDataStore(
      std::unique_ptr<Connection> connection,
      const DirectoryEntrySourceFunction& directoryEntrySourceFunction)
      : m_connection(std::move(connection)),
        m_directoryEntries(directoryEntrySourceFunction) {}

  template<typename C>
  SqlAdministrationDataStore<C>::~SqlAdministrationDataStore() {
    Close();
  }

  template<typename C>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry, AccountIdentity>>
      SqlAdministrationDataStore<C>::LoadAllAccountIdentities() {
    auto rows = std::vector<IndexedAccountIdentity>();
    try {
      m_connection->execute(Viper::select(GetIndexedAccountIdentityRow(),
        "account_identities", std::back_inserter(rows)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    auto identities = std::vector<std::tuple<
      Beam::ServiceLocator::DirectoryEntry, AccountIdentity>>();
    std::transform(rows.begin(), rows.end(), std::back_inserter(identities),
      [] (auto& row) {
        auto account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
          row.m_account);
        return std::make_tuple(std::move(account), std::move(row.m_identity));
      });
    return identities;
  }

  template<typename C>
  AccountIdentity SqlAdministrationDataStore<C>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto identity = std::optional<AccountIdentity>();
    try {
      m_connection->execute(Viper::select(GetAccountIdentityRow(),
        "account_identities", Viper::sym("account") == account.m_id,
        &identity));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    if(identity) {
      return std::move(*identity);
    }
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto row = IndexedAccountIdentity{account.m_id, identity, ""};
    try {
      m_connection->execute(Viper::upsert(GetIndexedAccountIdentityRow(),
        "account_identities", &row));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
  }

  template<typename C>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskParameters>> SqlAdministrationDataStore<C>::
      LoadAllRiskParameters() {
    auto rows = std::vector<IndexedRiskParameters>();
    try {
      m_connection->execute(Viper::select(GetIndexedRiskParameters(),
        "risk_parameters", std::back_inserter(rows)));
    } catch(const Viper::ExecuteException& e) {
      BOOST_THROW_EXCEPTION(AdministrationDataStoreException(e.what()));
    }
    auto parameters = std::vector<std::tuple<
      Beam::ServiceLocator::DirectoryEntry, RiskParameters>>();
    std::transform(rows.begin(), rows.end(), std::back_inserter(parameters),
      [] (auto& row) {
        auto account = Beam::ServiceLocator::DirectoryEntry::MakeAccount(
          row.m_account);
        return std::make_tuple(std::move(account), std::move(row.m_parameters));
      });
    return parameters;
  }

  template<typename C>
  RiskService::RiskParameters SqlAdministrationDataStore<C>::LoadRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {}

  template<typename C>
  std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      RiskService::RiskState>> SqlAdministrationDataStore<C>::
      LoadAllRiskStates() {
    return {};
  }

  template<typename C>
  RiskService::RiskState SqlAdministrationDataStore<C>::LoadRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {}

  template<typename C>
  AccountModificationRequest SqlAdministrationDataStore<C>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    return {};
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      SqlAdministrationDataStore<C>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    return {};
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      SqlAdministrationDataStore<C>::LoadAccountModificationRequestIds(
      AccountModificationRequest::Id startId, int maxCount) {
    return {};
  }

  template<typename C>
  EntitlementModification
      SqlAdministrationDataStore<C>::LoadEntitlementModification(
      AccountModificationRequest::Id id) {
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const AccountModificationRequest& request,
      const EntitlementModification& modification) {}

  template<typename C>
  RiskModification SqlAdministrationDataStore<C>::LoadRiskModification(
      AccountModificationRequest::Id id) {
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(
      const AccountModificationRequest& request,
      const RiskModification& modification) {}

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(AccountModificationRequest::Id id,
      const Message& message) {}

  template<typename C>
  AccountModificationRequest::Update
      SqlAdministrationDataStore<C>::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::Store(AccountModificationRequest::Id id,
      const AccountModificationRequest::Update& status) {}

  template<typename C>
  Message::Id SqlAdministrationDataStore<C>::LoadLastMessageId() {
    return {};
  }

  template<typename C>
  Message SqlAdministrationDataStore<C>::LoadMessage(Message::Id id) {
    return {};
  }

  template<typename C>
  std::vector<Message::Id> SqlAdministrationDataStore<C>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    return {};
  }

  template<typename C>
  void SqlAdministrationDataStore<C>::WithTransaction(
      const std::function<void ()>& transaction) {}

  template<typename C>
  void SqlAdministrationDataStore<C>::Open() {}

  template<typename C>
  void SqlAdministrationDataStore<C>::Close() {}

  template<typename C>
  void SqlAdministrationDataStore<C>::Shutdown() {}
}

#endif
