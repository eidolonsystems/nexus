#ifndef NEXUS_LOCALCOMPLIANCERULEDATASTORE_HPP
#define NEXUS_LOCALCOMPLIANCERULEDATASTORE_HPP
#include <memory>
#include <unordered_map>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleDataStore.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class LocalComplianceRuleDataStore
      \brief Implements a ComplianceRuleDataStore in memory.
   */
  class LocalComplianceRuleDataStore : private boost::noncopyable {
    public:

      //! Constructs a LocalComplianceRuleDataStore.
      LocalComplianceRuleDataStore() = default;

      ~LocalComplianceRuleDataStore();

      std::vector<ComplianceRuleEntry> LoadAllComplianceRuleEntries();

      ComplianceRuleId LoadNextComplianceRuleEntryId();

      boost::optional<ComplianceRuleEntry> LoadComplianceRuleEntry(
        ComplianceRuleId id);

      std::vector<ComplianceRuleEntry> LoadComplianceRuleEntries(
        const Beam::ServiceLocator::DirectoryEntry& directoryEntry);

      void Store(const ComplianceRuleEntry& entry);

      void Delete(ComplianceRuleId id);

      void Store(const ComplianceRuleViolationRecord& violationRecord);

      void Open();

      void Close();

    private:
      mutable boost::mutex m_mutex;
      std::unordered_map<ComplianceRuleId, std::shared_ptr<ComplianceRuleEntry>>
        m_entriesById;
      std::unordered_map<Beam::ServiceLocator::DirectoryEntry,
        std::vector<std::shared_ptr<ComplianceRuleEntry>>>
        m_entriesByDirectoryEntry;
      Beam::IO::OpenState m_openState;
  };

  inline LocalComplianceRuleDataStore::~LocalComplianceRuleDataStore() {
    Close();
  }

  inline std::vector<ComplianceRuleEntry> LocalComplianceRuleDataStore::
      LoadAllComplianceRuleEntries() {
    std::vector<ComplianceRuleEntry> entries;
    boost::lock_guard<boost::mutex> lock{m_mutex};
    entries.reserve(m_entriesById.size());
    std::transform(m_entriesById.begin(), m_entriesById.end(),
      std::back_inserter(entries),
      [] (auto& entry) {
        return *entry.second;
      });
    return entries;
  }

  inline ComplianceRuleId LocalComplianceRuleDataStore::
      LoadNextComplianceRuleEntryId() {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    ComplianceRuleId id = 1;
    for(auto& entry : m_entriesById) {
      id = std::max(entry.second->GetId() + 1, id);
    }
    return id;
  }

  inline boost::optional<ComplianceRuleEntry>
      LocalComplianceRuleDataStore::LoadComplianceRuleEntry(
      ComplianceRuleId id) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    auto entry = Beam::Retrieve(m_entriesById, id);
    if(!entry.is_initialized()) {
      return boost::none;
    }
    return **entry;
  }

  inline std::vector<ComplianceRuleEntry> LocalComplianceRuleDataStore::
      LoadComplianceRuleEntries(
      const Beam::ServiceLocator::DirectoryEntry& directoryEntry) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    auto entries = Beam::Retrieve(m_entriesByDirectoryEntry, directoryEntry);
    if(!entries.is_initialized()) {
      return {};
    }
    std::vector<ComplianceRuleEntry> result;
    result.reserve(entries->size());
    std::transform(entries->begin(), entries->end(),
      std::back_inserter(result),
      [] (auto& entry) {
        return *entry;
      });
    return result;
  }

  inline void LocalComplianceRuleDataStore::Store(
      const ComplianceRuleEntry& entry) {
    auto newEntry = std::make_shared<ComplianceRuleEntry>(entry);
    boost::lock_guard<boost::mutex> lock{m_mutex};
    auto& previousEntry = m_entriesById[entry.GetId()];
    if(previousEntry != nullptr) {
      auto& previousAccountEntries = m_entriesByDirectoryEntry[
        previousEntry->GetDirectoryEntry()];
      previousAccountEntries.erase(std::find_if(previousAccountEntries.begin(),
        previousAccountEntries.end(),
        [&] (auto& accountEntry) {
          return accountEntry->GetId() == entry.GetId();
        }));
    }
    previousEntry = newEntry;
    auto& accountEntries = m_entriesByDirectoryEntry[entry.GetDirectoryEntry()];
    accountEntries.push_back(newEntry);
  }

  inline void LocalComplianceRuleDataStore::Delete(ComplianceRuleId id) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    auto entryByIdIterator = m_entriesById.find(id);
    if(entryByIdIterator == m_entriesById.end()) {
      return;
    }
    auto& entry = entryByIdIterator->second;
    auto& accountEntries =
      m_entriesByDirectoryEntry[entry->GetDirectoryEntry()];
    accountEntries.erase(std::find_if(accountEntries.begin(),
      accountEntries.end(),
      [&] (auto& accountEntry) {
        return accountEntry->GetId() == id;
      }));
    m_entriesById.erase(entryByIdIterator);
  }

  inline void LocalComplianceRuleDataStore::Store(
    const ComplianceRuleViolationRecord& violationRecord) {}

  inline void LocalComplianceRuleDataStore::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalComplianceRuleDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }
}
}

#endif
