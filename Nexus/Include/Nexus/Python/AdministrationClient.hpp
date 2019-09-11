#ifndef NEXUS_PYTHON_ADMINISTRATION_CLIENT_HPP
#define NEXUS_PYTHON_ADMINISTRATION_CLIENT_HPP
#include <pybind11/pybind11.h>
#include "Nexus/AdministrationService/VirtualAdministrationClient.hpp"

namespace Nexus::AdministrationService {

  /**
   * Wraps an AdministrationClient for use with Python.
   * @tparam <C> The type of AdministrationClient to wrap.
   */
  template<typename C>
  class ToPythonAdministrationClient final :
      public VirtualAdministrationClient {
    public:

      //! The type of AdministrationClient to wrap.
      using Client = C;

      //! Constructs a ToPythonAdministrationClient.
      /*!
        \param client The AdministrationClient to wrap.
      */
      ToPythonAdministrationClient(std::unique_ptr<Client> client);

      ~ToPythonAdministrationClient() override;

      std::string LoadOrganizationName() override;

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAccountsByRoles(AccountRoles roles) override;

      Beam::ServiceLocator::DirectoryEntry
        LoadAdministratorsRootEntry() override;

      Beam::ServiceLocator::DirectoryEntry LoadServicesRootEntry() override;

      Beam::ServiceLocator::DirectoryEntry
        LoadTradingGroupsRootEntry() override;

      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child) override;

      Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreIdentity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity) override;

      TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory) override;

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account) override;

      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadAdministrators() override;

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadServices() override;

      MarketDataService::EntitlementDatabase LoadEntitlements() override;

      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>&
        entitlements) override;

      const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters) override;

      const Beam::Publisher<RiskService::RiskState>& GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account) override;

      void StoreRiskState(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState) override;

      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id) override;

      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount) override;

      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id) override;

      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
        const EntitlementModification& modification,
        const Message& comment) override;

      RiskModification LoadRiskModification(
        AccountModificationRequest::Id id) override;

      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
        const RiskModification& modification, const Message& comment) override;

      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id) override;

      AccountModificationRequest::Update ApproveAccountModificationRequest(
        AccountModificationRequest::Id id,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Message& comment) override;

      AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Message& comment) override;

      Message LoadMessage(Message::Id id) override;

      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id) override;

      Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message) override;

      void Open() override;

      void Close() override;

    private:
      std::unique_ptr<Client> m_client;
  };

  /**
   * Makes a ToPythonAdministrationClient.
   * @param client The AdministrationClient to wrap.
   */
  template<typename Client>
  auto MakeToPythonAdministrationClient(std::unique_ptr<Client> client) {
    return std::make_unique<ToPythonAdministrationClient<Client>>(
      std::move(client));
  }

  template<typename C>
  ToPythonAdministrationClient<C>::ToPythonAdministrationClient(
    std::unique_ptr<Client> client)
    : m_client(std::move(client)) {}

  template<typename C>
  ToPythonAdministrationClient<C>::~ToPythonAdministrationClient() {
    Close();
    auto release = pybind11::gil_scoped_release();
    m_client.reset();
  }

  template<typename C>
  std::string ToPythonAdministrationClient<C>::LoadOrganizationName() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadOrganizationName();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadAccountsByRoles(AccountRoles roles) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAccountsByRoles(roles);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadAdministratorsRootEntry() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAdministratorsRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadServicesRootEntry() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadServicesRootEntry();
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadTradingGroupsRootEntry() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadTradingGroupsRootEntry();
  }

  template<typename C>
  bool ToPythonAdministrationClient<C>::CheckAdministrator(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->CheckAdministrator(account);
  }

  template<typename C>
  AccountRoles ToPythonAdministrationClient<C>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAccountRoles(account);
  }

  template<typename C>
  AccountRoles ToPythonAdministrationClient<C>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAccountRoles(parent, child);
  }

  template<typename C>
  Beam::ServiceLocator::DirectoryEntry
      ToPythonAdministrationClient<C>::LoadTradingGroupEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadTradingGroupEntry(account);
  }

  template<typename C>
  AccountIdentity ToPythonAdministrationClient<C>::LoadIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadIdentity(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto release = pybind11::gil_scoped_release();
    m_client->StoreIdentity(account, identity);
  }

  template<typename C>
  TradingGroup ToPythonAdministrationClient<C>::LoadTradingGroup(
      const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadTradingGroup(directory);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadManagedTradingGroups(account);
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadAdministrators() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAdministrators();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadServices() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadServices();
  }

  template<typename C>
  MarketDataService::EntitlementDatabase
      ToPythonAdministrationClient<C>::LoadEntitlements() {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadEntitlements();
  }

  template<typename C>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      ToPythonAdministrationClient<C>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadEntitlements(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto release = pybind11::gil_scoped_release();
    m_client->StoreEntitlements(account, entitlements);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskParameters>&
      ToPythonAdministrationClient<C>::GetRiskParametersPublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->GetRiskParametersPublisher(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreRiskParameters(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto release = pybind11::gil_scoped_release();
    m_client->StoreRiskParameters(account, riskParameters);
  }

  template<typename C>
  const Beam::Publisher<RiskService::RiskState>&
      ToPythonAdministrationClient<C>::GetRiskStatePublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto release = pybind11::gil_scoped_release();
    return m_client->GetRiskStatePublisher(account);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto release = pybind11::gil_scoped_release();
    m_client->StoreRiskState(account, riskState);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::
      LoadAccountModificationRequest(AccountModificationRequest::Id id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAccountModificationRequest(id);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  std::vector<AccountModificationRequest::Id>
      ToPythonAdministrationClient<C>::LoadManagedAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id startId, int maxCount) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadManagedAccountModificationRequestIds(account, startId,
      maxCount);
  }

  template<typename C>
  EntitlementModification ToPythonAdministrationClient<C>::
      LoadEntitlementModification(AccountModificationRequest::Id id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadEntitlementModification(id);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
      const EntitlementModification& modification, const Message& comment) {
    auto release = pybind11::gil_scoped_release();
    return m_client->SubmitAccountModificationRequest(account,
      submissionAccount, modification, comment);
  }

  template<typename C>
  RiskModification ToPythonAdministrationClient<C>::
      LoadRiskModification(AccountModificationRequest::Id id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadRiskModification(id);
  }

  template<typename C>
  AccountModificationRequest ToPythonAdministrationClient<C>::
      SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
      const RiskModification& modification, const Message& comment) {
    auto release = pybind11::gil_scoped_release();
    return m_client->SubmitAccountModificationRequest(account,
      submissionAccount, modification, comment);
  }

  template<typename C>
  AccountModificationRequest::Update ToPythonAdministrationClient<C>::
      LoadAccountModificationRequestStatus(AccountModificationRequest::Id id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadAccountModificationRequestStatus(id);
  }

  template<typename C>
  AccountModificationRequest::Update ToPythonAdministrationClient<C>::
      ApproveAccountModificationRequest(AccountModificationRequest::Id id,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Message& comment) {
    auto release = pybind11::gil_scoped_release();
    return m_client->ApproveAccountModificationRequest(id, account, comment);
  }

  template<typename C>
  AccountModificationRequest::Update ToPythonAdministrationClient<C>::
      RejectAccountModificationRequest(AccountModificationRequest::Id id,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Message& comment) {
    auto release = pybind11::gil_scoped_release();
    return m_client->RejectAccountModificationRequest(id, account, comment);
  }

  template<typename C>
  Message ToPythonAdministrationClient<C>::LoadMessage(Message::Id id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadMessage(id);
  }

  template<typename C>
  std::vector<Message::Id> ToPythonAdministrationClient<C>::
      LoadMessageIds(AccountModificationRequest::Id id) {
    auto release = pybind11::gil_scoped_release();
    return m_client->LoadMessageIds(id);
  }

  template<typename C>
  Message ToPythonAdministrationClient<C>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
      const Message& message) {
    auto release = pybind11::gil_scoped_release();
    return m_client->SendAccountModificationRequestMessage(id, message);
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::Open() {
    auto release = pybind11::gil_scoped_release();
    m_client->Open();
  }

  template<typename C>
  void ToPythonAdministrationClient<C>::Close() {
    auto release = pybind11::gil_scoped_release();
    m_client->Close();
  }
}

#endif
