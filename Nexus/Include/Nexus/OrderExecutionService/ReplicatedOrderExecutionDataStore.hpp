#ifndef NEXUS_REPLICATEDORDEREXECUTIONDATASTORE_HPP
#define NEXUS_REPLICATEDORDEREXECUTIONDATASTORE_HPP
#include <iostream>
#include <vector>
#include <boost/atomic/atomic.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/OrderExecutionService/VirtualOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class ReplicatedOrderExecutionDataStore
      \brief Duplicates an OrderExecutionDataStore across multiple instances.
   */
  class ReplicatedOrderExecutionDataStore : private boost::noncopyable {
    public:

      //! Constructs an empty ReplicatedOrderExecutionDataStore.
      /*!
        \param primaryDataStore The primary data store to access.
        \param duplicateDataStores The data stores to replicate the primary to.
      */
      ReplicatedOrderExecutionDataStore(
        std::unique_ptr<VirtualOrderExecutionDataStore> primaryDataStore,
        std::vector<std::unique_ptr<VirtualOrderExecutionDataStore>>
        duplicateDataStores);

      ~ReplicatedOrderExecutionDataStore();

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Open();

      void Close();

    private:
      std::unique_ptr<VirtualOrderExecutionDataStore> m_primaryDataStore;
      std::vector<std::unique_ptr<VirtualOrderExecutionDataStore>>
        m_duplicateDataStores;
      boost::atomic<std::size_t> m_nextDataStore;
  };

  inline ReplicatedOrderExecutionDataStore::ReplicatedOrderExecutionDataStore(
      std::unique_ptr<VirtualOrderExecutionDataStore> primaryDataStore,
      std::vector<std::unique_ptr<VirtualOrderExecutionDataStore>>
      duplicateDataStores)
      : m_primaryDataStore{std::move(primaryDataStore)},
        m_duplicateDataStores(std::move(duplicateDataStores)),
        m_nextDataStore{0} {}

  inline ReplicatedOrderExecutionDataStore::
      ~ReplicatedOrderExecutionDataStore() {
    Close();
  }

  inline std::vector<SequencedOrderRecord>
      ReplicatedOrderExecutionDataStore::LoadOrderSubmissions(
      const AccountQuery& query) {
    if(m_duplicateDataStores.empty()) {
      return m_primaryDataStore->LoadOrderSubmissions(query);
    }
    auto index = ++m_nextDataStore;
    index = index % m_duplicateDataStores.size();
    return m_duplicateDataStores[index]->LoadOrderSubmissions(query);
  }

  inline std::vector<SequencedExecutionReport>
      ReplicatedOrderExecutionDataStore::LoadExecutionReports(
      const AccountQuery& query) {
    if(m_duplicateDataStores.empty()) {
      return m_primaryDataStore->LoadExecutionReports(query);
    }
    auto index = ++m_nextDataStore;
    index = index % m_duplicateDataStores.size();
    return m_duplicateDataStores[index]->LoadExecutionReports(query);
  }

  inline void ReplicatedOrderExecutionDataStore::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_primaryDataStore->Store(orderInfo);
    for(auto& dataStore : m_duplicateDataStores) {
      dataStore->Store(orderInfo);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_primaryDataStore->Store(executionReport);
    for(auto& dataStore : m_duplicateDataStores) {
      dataStore->Store(executionReport);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::Open() {
    m_primaryDataStore->Open();
    auto i = m_duplicateDataStores.begin();
    while(i != m_duplicateDataStores.end()) {
      try {
        (*i)->Open();
        ++i;
      } catch(const std::exception& e) {
        std::cerr << "Failed to open replicated data store: " << e.what() <<
          "\n";
        i = m_duplicateDataStores.erase(i);
      }
    }
  }

  inline void ReplicatedOrderExecutionDataStore::Close() {
    for(auto& dataStore : m_duplicateDataStores) {
      dataStore->Close();
    }
    m_primaryDataStore->Close();
  }
}
}

#endif
