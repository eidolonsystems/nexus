#ifndef NEXUS_ADMINISTRATIONCLIENT_HPP
#define NEXUS_ADMINISTRATIONCLIENT_HPP
#include <vector>
#include <Beam/IO/Connection.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Services/ServiceProtocolClientHandler.hpp>
#include <Beam/Utilities/SynchronizedMap.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/AdministrationService/AccountIdentity.hpp"
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/AdministrationService/EntitlementModification.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"
#include "Nexus/AdministrationService/AdministrationServices.hpp"
#include "Nexus/AdministrationService/Message.hpp"
#include "Nexus/AdministrationService/TradingGroup.hpp"
#include "Nexus/RiskService/RiskParameters.hpp"

namespace Nexus {
namespace AdministrationService {

  //! Used to push updates about an account's RiskState.
  using RiskStateQueue = Beam::AbstractQueue<RiskService::RiskState>;

  //! Used to push updates about an account's RiskParameters.
  using RiskParametersQueue = Beam::AbstractQueue<RiskService::RiskParameters>;

  /*! \class AdministrationClient
      \brief Client used to access the Nexus Administration service.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class AdministrationClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder = Beam::GetTryDereferenceType<
        ServiceProtocolClientBuilderType>;

      //! Constructs an AdministrationClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      AdministrationClient(ClientBuilderForward&& clientBuilder);

      ~AdministrationClient();

      //! Returns the name of the organization.
      /*!
        \return The name of the organization.
      */
      std::string LoadOrganizationName();

      //! Loads the list of accounts that match a set of roles.
      /*!
        \param roles The roles to match.
        \return The list of directory entries of accounts that match the
                specified roles.
      */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAccountsByRoles(
        AccountRoles roles);

      //! Loads the DirectoryEntry containing all administrators.
      Beam::ServiceLocator::DirectoryEntry LoadAdministratorsRootEntry();

      //! Loads the DirectoryEntry containing all service accounts.
      Beam::ServiceLocator::DirectoryEntry LoadServicesRootEntry();

      //! Loads the DirectoryEntry containing all trading groups.
      Beam::ServiceLocator::DirectoryEntry LoadTradingGroupsRootEntry();

      //! Returns <code>true</code> iff an account is an administrator.
      /*!
        \param account The account to test.
        \return <code>true</code> iff the <i>account</i> is an administrator.
      */
      bool CheckAdministrator(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Returns an accounts roles.
      /*!
        \param account The account to lookup.
        \return The roles associated with the <i>account</i>.
      */
      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Returns the roles one account has over another.
      /*!
        \param parent The account whose roles are to be loaded.
        \param child The account being supervised.
        \return The roles that the <i>parent</i> account has over the
                <i>child</i> account.
      */
      AccountRoles LoadAccountRoles(
        const Beam::ServiceLocator::DirectoryEntry& parent,
        const Beam::ServiceLocator::DirectoryEntry& child);

      //! Loads an account's trading group Directory.
      /*!
        \param account The account whose trading group is to be loaded.
        \return The directory of the trading <i>account</i>'s group.
      */
      Beam::ServiceLocator::DirectoryEntry LoadTradingGroupEntry(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Loads an account's identity.
      /*!
        \param account The account whose identity is to be loaded.
        \return The AccountIdentity for the specified <i>account</i>.
      */
      AccountIdentity LoadIdentity(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Sets the identity for an account.
      /*!
        \param account The account to set the identity for.
        \param identity The AccountIdentity to assign to the <i>account</i>.
      */
      void StoreIdentity(const Beam::ServiceLocator::DirectoryEntry& account,
        const AccountIdentity& identity);

      //! Loads a TradingGroup from its DirectoryEntry.
      /*!
        \param directory The DirectoryEntry of the TradingGroup to load.
        \return The TradingGroup represented by the specified <i>directory</i>.
      */
      TradingGroup LoadTradingGroup(
        const Beam::ServiceLocator::DirectoryEntry& directory);

      //! Loads the DirectoryEntries of TradingGroups managed by an account.
      /*!
        \param account The account to load the TradingGroups from.
        \return The list of TradingGroups managed by the <i>account</i>.
      */
      std::vector<Beam::ServiceLocator::DirectoryEntry>
        LoadManagedTradingGroups(const Beam::ServiceLocator::DirectoryEntry&
        account);

      //! Loads the system administrators.
      /*!
        \return The list of system administrators.
      */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadAdministrators();

      //! Loads the accounts providing system services.
      /*!
        \return The list of accounts providing system services.
      */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadServices();

      //! Loads the EntitlementDatabase.
      /*!
        \return The EntitlementDatabase.
      */
      MarketDataService::EntitlementDatabase LoadEntitlements();

      //! Loads the entitlements granted to an account.
      /*!
        \param account The account to load the entitlements for.
        \return The list of entitlements granted to the <i>account</i>.
      */
      std::vector<Beam::ServiceLocator::DirectoryEntry> LoadEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Sets an account's entitlements.
      /*!
        \param account The account of the entitlements to set.
        \param entitlements The list of entitlements to grant to the
               <i>account</i>.
      */
      void StoreEntitlements(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements);

      //! Returns the object publishing an account's RiskParameters.
      /*!
        \param account The account to monitor.
      */
      const Beam::Publisher<RiskService::RiskParameters>&
        GetRiskParametersPublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Sets an account's RiskParameters.
      /*!
        \param account The account whose RiskParameters are to be set.
        \param riskParameters The RiskParameters to assign to the
               <i>account</i>.
      */
      void StoreRiskParameters(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);

      //! Returns the object publishing an account's RiskState.
      /*!
        \param account The account to monitor.
      */
      const Beam::Publisher<RiskService::RiskState>& GetRiskStatePublisher(
        const Beam::ServiceLocator::DirectoryEntry& account);

      //! Sets an account's RiskState.
      /*!
        \param account The account to set RiskState of.
        \param riskState The <i>account</i>'s current RiskState.
      */
      void StoreRiskState(const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskState& riskState);

      //! Loads an account modification request.
      /*!
        \param id The id of the request to load.
        \return The request with the specified <i>id</i>.
      */
      AccountModificationRequest LoadAccountModificationRequest(
        AccountModificationRequest::Id id);

      //! Given an account, loads the ids of requests to modify that account.
      /*!
        \param account The account whose requests are to be loaded.
        \param startId The id of the first request to load (exclusive) or -1
               to start with the most recent request.
        \param maxCount The maximum number of ids to load.
        \return The list of account modification requests.
      */
      std::vector<AccountModificationRequest::Id>
        LoadAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount);

      //! Given an account, loads the ids of requests that the account is
      //! authorized to manage.
      /*!
        \param account The account managing modifications.
        \param startId The id of the first request to load (exclusive) or -1
               to start with the most recent request.
        \param maxCount The maximum number of ids to load.
        \return The list of account modification requests.
      */
      std::vector<AccountModificationRequest::Id>
        LoadManagedAccountModificationRequestIds(
        const Beam::ServiceLocator::DirectoryEntry& account,
        AccountModificationRequest::Id startId, int maxCount);

      //! Loads an entitlement modification.
      /*!
        \param id The id of the request to load.
        \return The entitlement modification with the specified <i>id</i>.
      */
      EntitlementModification LoadEntitlementModification(
        AccountModificationRequest::Id id);

      //! Submits a request to modify an account's entitlements.
      /*!
        \param account The account to modify.
        \param submissionAccount The account submitting the request.
        \param modification The modification to apply.
        \param comment The comment to associate with the request.
        \return An object representing the request.
      */
      AccountModificationRequest SubmitAccountModificationRequest(
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
        const EntitlementModification& modification, const Message& comment);

      //! Loads the status of an account modification request.
      /*!
        \param id The id of the request.
        \return The update representing the current status of the request.
      */
      AccountModificationRequest::Update LoadAccountModificationRequestStatus(
        AccountModificationRequest::Id id);

      //! Approves an account modification request.
      /*!
        \param id The id of the request to approve.
        \param account The account that approved the request.
        \param comment The comment to associate with the update.
        \return An object representing the update.
      */
      AccountModificationRequest::Update ApproveAccountModificationRequest(
        AccountModificationRequest::Id id,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Message& comment);

      //! Rejects an account modification request.
      /*!
        \param id The id of the request to reject.
        \param account The account that rejected the request.
        \param comment The comment to associate with the update.
        \return An object representing the update.
      */
      AccountModificationRequest::Update RejectAccountModificationRequest(
        AccountModificationRequest::Id id,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const Message& comment);

      //! Loads a message.
      /*!
        \param id The id of the message.
        \return The message with the specified <i>id</i>.
      */
      Message LoadMessage(Message::Id id);

      //! Loads the list of messages associated with an account modification.
      /*!
        \param id The id of the request.
        \return A list of message ids associated with the request.
      */
      std::vector<Message::Id> LoadMessageIds(
        AccountModificationRequest::Id id);

      //! Appends a message to an account modification request.
      /*!
        \param id The id of the request to send the message to.
        \param message The message to append.
        \return The appended message.
      */
      Message SendAccountModificationRequestMessage(
        AccountModificationRequest::Id id, const Message& message);

      void Open();

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      using RiskParameterPublisher = Beam::StatePublisher<
        RiskService::RiskParameters>;
      using RiskStatePublisher = Beam::StatePublisher<RiskService::RiskState>;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      Beam::IO::OpenState m_openState;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskParameterPublisher>> m_riskParameterPublishers;
      Beam::SynchronizedUnorderedMap<Beam::ServiceLocator::DirectoryEntry,
        std::shared_ptr<RiskStatePublisher>> m_riskStatePublishers;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnReconnect(const std::shared_ptr<ServiceProtocolClient>& client);
      void RecoverRiskParameters(ServiceProtocolClient& client);
      void RecoverRiskState(ServiceProtocolClient& client);
      void OnRiskParametersMessage(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        const RiskService::RiskParameters& riskParameters);
      void OnRiskStateMessage(ServiceProtocolClient& client,
        const Beam::ServiceLocator::DirectoryEntry& account,
        RiskService::RiskState riskState);
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  AdministrationClient<ServiceProtocolClientBuilderType>::AdministrationClient(
      ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    m_clientHandler.SetReconnectHandler(
      std::bind(&AdministrationClient::OnReconnect, this,
      std::placeholders::_1));
    RegisterAdministrationServices(Beam::Store(m_clientHandler.GetSlots()));
    RegisterAdministrationMessages(Beam::Store(m_clientHandler.GetSlots()));
    Beam::Services::AddMessageSlot<RiskParametersMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind(&AdministrationClient::OnRiskParametersMessage, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Beam::Services::AddMessageSlot<RiskStateMessage>(
      Beam::Store(m_clientHandler.GetSlots()),
      std::bind(&AdministrationClient::OnRiskStateMessage, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  template<typename ServiceProtocolClientBuilderType>
  AdministrationClient<ServiceProtocolClientBuilderType>::
      ~AdministrationClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  std::string AdministrationClient<ServiceProtocolClientBuilderType>::
      LoadOrganizationName() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadOrganizationNameService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Beam::ServiceLocator::DirectoryEntry>
      AdministrationClient<ServiceProtocolClientBuilderType>::
      LoadAccountsByRoles(AccountRoles roles) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountsByRolesService>(roles);
  }

  template<typename ServiceProtocolClientBuilderType>
  Beam::ServiceLocator::DirectoryEntry AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadAdministratorsRootEntry() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAdministratorsRootEntryService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  Beam::ServiceLocator::DirectoryEntry AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadServicesRootEntry() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadServicesRootEntryService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  Beam::ServiceLocator::DirectoryEntry AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadTradingGroupsRootEntry() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadTradingGroupsRootEntryService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  bool AdministrationClient<ServiceProtocolClientBuilderType>::
      CheckAdministrator(const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<CheckAdministratorService>(account);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountRoles AdministrationClient<ServiceProtocolClientBuilderType>::
      LoadAccountRoles(const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountRolesService>(account);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountRoles AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadAccountRoles(
      const Beam::ServiceLocator::DirectoryEntry& parent,
      const Beam::ServiceLocator::DirectoryEntry& child) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadSupervisedAccountRolesService>(
      parent, child);
  }

  template<typename ServiceProtocolClientBuilderType>
  Beam::ServiceLocator::DirectoryEntry AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadTradingGroupEntry(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountTradingGroupEntryService>(
      account);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountIdentity AdministrationClient<ServiceProtocolClientBuilderType>::
      LoadIdentity(const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountIdentityService>(account);
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::StoreIdentity(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const AccountIdentity& identity) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreAccountIdentityService>(account,
      identity);
  }

  template<typename ServiceProtocolClientBuilderType>
  TradingGroup AdministrationClient<ServiceProtocolClientBuilderType>::
      LoadTradingGroup(const Beam::ServiceLocator::DirectoryEntry& directory) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadTradingGroupService>(directory);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadManagedTradingGroups(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadManagedTradingGroupsService>(
      account);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadAdministrators() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAdministratorsService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadServices() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadServicesService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  MarketDataService::EntitlementDatabase AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadEntitlements() {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadEntitlementsService>(0);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Beam::ServiceLocator::DirectoryEntry> AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadEntitlements(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountEntitlementsService>(
      account);
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::
      StoreEntitlements(const Beam::ServiceLocator::DirectoryEntry& account,
      const std::vector<Beam::ServiceLocator::DirectoryEntry>& entitlements) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreEntitlementsService>(account,
      entitlements);
  }

  template<typename ServiceProtocolClientBuilderType>
  const Beam::Publisher<RiskService::RiskParameters>&
      AdministrationClient<ServiceProtocolClientBuilderType>::
      GetRiskParametersPublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_riskParameterPublishers.GetOrInsert(account,
      [&] {
        auto publisher = std::make_shared<RiskParameterPublisher>();
        m_tasks.Push(
          [=] {
            try {
              auto client = m_clientHandler.GetClient();
              auto parameters =
                client->template SendRequest<MonitorRiskParametersService>(
                account);
              publisher->Push(parameters);
            } catch(const std::exception&) {
              publisher->Break(std::current_exception());
            }
          });
        return publisher;
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::
      StoreRiskParameters(const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreRiskParametersService>(account,
      riskParameters);
  }

  template<typename ServiceProtocolClientBuilderType>
  const Beam::Publisher<RiskService::RiskState>&
      AdministrationClient<ServiceProtocolClientBuilderType>::
      GetRiskStatePublisher(
      const Beam::ServiceLocator::DirectoryEntry& account) {
    return *m_riskStatePublishers.GetOrInsert(account,
      [&] {
        auto publisher = std::make_shared<RiskStatePublisher>();
        m_tasks.Push(
          [=] {
            try {
              auto client = m_clientHandler.GetClient();
              auto state = client->template SendRequest<
                MonitorRiskStateService>(account);
              publisher->Push(state);
            } catch(const std::exception&) {
              publisher->Break(std::current_exception());
            }
          });
        return publisher;
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::StoreRiskState(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskState& riskState) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<StoreRiskStateService>(account, riskState);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountModificationRequest AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadAccountModificationRequest(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadAccountModificationRequestService>(
      id);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<AccountModificationRequest::Id> AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id id, int maxCount) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadAccountModificationRequestIdsService>(account, id, maxCount);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<AccountModificationRequest::Id>
      AdministrationClient<ServiceProtocolClientBuilderType>::
      LoadManagedAccountModificationRequestIds(
      const Beam::ServiceLocator::DirectoryEntry& account,
      AccountModificationRequest::Id id, int maxCount) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadManagedAccountModificationRequestIdsService>(account, id, maxCount);
  }

  template<typename ServiceProtocolClientBuilderType>
  EntitlementModification AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadEntitlementModification(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadEntitlementModificationService>(id);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountModificationRequest AdministrationClient<
      ServiceProtocolClientBuilderType>::SubmitAccountModificationRequest(
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Beam::ServiceLocator::DirectoryEntry& submissionAccount,
      const EntitlementModification& modification, const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      SubmitEntitlementModificationRequestService>(account, submissionAccount,
      modification, comment);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountModificationRequest::Update AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadAccountModificationRequestStatus(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      LoadAccountModificationRequestStatusService>(id);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountModificationRequest::Update AdministrationClient<
      ServiceProtocolClientBuilderType>::ApproveAccountModificationRequest(
      AccountModificationRequest::Id id,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      ApproveAccountModificationRequestService>(id, account, comment);
  }

  template<typename ServiceProtocolClientBuilderType>
  AccountModificationRequest::Update AdministrationClient<
      ServiceProtocolClientBuilderType>::RejectAccountModificationRequest(
      AccountModificationRequest::Id id,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const Message& comment) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      RejectAccountModificationRequestService>(id, account, comment);
  }

  template<typename ServiceProtocolClientBuilderType>
  Message AdministrationClient<ServiceProtocolClientBuilderType>::LoadMessage(
      Message::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMessageService>(id);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<Message::Id> AdministrationClient<
      ServiceProtocolClientBuilderType>::LoadMessageIds(
      AccountModificationRequest::Id id) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadMessageIdsService>(id);
  }

  template<typename ServiceProtocolClientBuilderType>
  Message AdministrationClient<ServiceProtocolClientBuilderType>::
      SendAccountModificationRequestMessage(AccountModificationRequest::Id id,
      const Message& message) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<
      SendAccountModificationRequestMessageService>(id, message);
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_clientHandler.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_tasks.Break();
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::OnReconnect(
      const std::shared_ptr<ServiceProtocolClient>& client) {
    m_tasks.Push(
      [=] {
        RecoverRiskParameters(*client);
        RecoverRiskState(*client);
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::
      RecoverRiskParameters(ServiceProtocolClient& client) {
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      std::shared_ptr<RiskParameterPublisher>>> currentRiskParameterEntries;
    m_riskParameterPublishers.With(
      [&] (const std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
          std::shared_ptr<RiskParameterPublisher>>& riskParameterPublishers) {
        for(const auto& publisher : riskParameterPublishers) {
          currentRiskParameterEntries.emplace_back(
            publisher.first, publisher.second);
        }
      });
    for(const auto& entry : currentRiskParameterEntries) {
      const auto& publisher = std::get<1>(entry);
      try {
        auto parameters =
          client.template SendRequest<MonitorRiskParametersService>(
          std::get<0>(entry));
        bool publish;
        publisher->WithSnapshot(
          [&] (boost::optional<const RiskService::RiskParameters&> snapshot) {
            if(!snapshot.is_initialized() || *snapshot != parameters) {
              publish = true;
            } else {
              publish = false;
            }
          });
        if(publish) {
          publisher->Push(parameters);
        }
      } catch(const std::exception&) {
        publisher->Break(std::current_exception());
      }
    }
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::RecoverRiskState(
      ServiceProtocolClient& client) {
    std::vector<std::tuple<Beam::ServiceLocator::DirectoryEntry,
      std::shared_ptr<RiskStatePublisher>>> currentRiskStateEntries;
    m_riskStatePublishers.With(
      [&] (const std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
          std::shared_ptr<RiskStatePublisher>>& riskStatePublishers) {
        for(const auto& publisher : riskStatePublishers) {
          currentRiskStateEntries.emplace_back(
            publisher.first, publisher.second);
        }
      });
    for(const auto& entry : currentRiskStateEntries) {
      const auto& publisher = std::get<1>(entry);
      try {
        auto state = client.template SendRequest<MonitorRiskStateService>(
          std::get<0>(entry));
        bool publish;
        publisher->WithSnapshot(
          [&] (boost::optional<const RiskService::RiskState&> snapshot) {
            if(!snapshot.is_initialized() || *snapshot != state) {
              publish = true;
            } else {
              publish = false;
            }
          });
        if(publish) {
          publisher->Push(state);
        }
      } catch(const std::exception&) {
        publisher->Break(std::current_exception());
      }
    }
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::
      OnRiskParametersMessage(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      const RiskService::RiskParameters& riskParameters) {
    m_tasks.Push(
      [=] {
        auto publisher = m_riskParameterPublishers.FindValue(account);
        if(!publisher.is_initialized()) {
          return;
        }
        (*publisher)->Push(riskParameters);
      });
  }

  template<typename ServiceProtocolClientBuilderType>
  void AdministrationClient<ServiceProtocolClientBuilderType>::
      OnRiskStateMessage(ServiceProtocolClient& client,
      const Beam::ServiceLocator::DirectoryEntry& account,
      RiskService::RiskState riskState) {
    m_tasks.Push(
      [=] {
        auto publisher = m_riskStatePublishers.FindValue(account);
        if(!publisher.is_initialized()) {
          return;
        }
        (*publisher)->Push(riskState);
      });
  }
}
}

#endif
