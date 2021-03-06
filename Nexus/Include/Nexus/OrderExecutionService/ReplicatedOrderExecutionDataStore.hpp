#ifndef NEXUS_REPLICATED_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_REPLICATED_ORDER_EXECUTION_DATA_STORE_HPP
#include <iostream>
#include <vector>
#include <boost/atomic/atomic.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreBox.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Duplicates an OrderExecutionDataStore across multiple instances. */
  class ReplicatedOrderExecutionDataStore {
    public:

      /**
       * Constructs an empty ReplicatedOrderExecutionDataStore.
       * @param primaryDataStore The primary data store to access.
       * @param duplicateDataStores The data stores to replicate the primary to.
       */
      ReplicatedOrderExecutionDataStore(
        OrderExecutionDataStoreBox primaryDataStore,
        std::vector<OrderExecutionDataStoreBox> duplicateDataStores);

      ~ReplicatedOrderExecutionDataStore();

      boost::optional<SequencedAccountOrderRecord> LoadOrder(OrderId id);

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Close();

    private:
      OrderExecutionDataStoreBox m_primaryDataStore;
      std::vector<OrderExecutionDataStoreBox> m_duplicateDataStores;
      boost::atomic<std::size_t> m_nextDataStore;

      ReplicatedOrderExecutionDataStore(
        const ReplicatedOrderExecutionDataStore&) = delete;
      ReplicatedOrderExecutionDataStore& operator =(
        const ReplicatedOrderExecutionDataStore&) = delete;
  };

  inline ReplicatedOrderExecutionDataStore::ReplicatedOrderExecutionDataStore(
    OrderExecutionDataStoreBox primaryDataStore,
    std::vector<OrderExecutionDataStoreBox> duplicateDataStores)
    : m_primaryDataStore(std::move(primaryDataStore)),
      m_duplicateDataStores(std::move(duplicateDataStores)),
      m_nextDataStore(0) {}

  inline ReplicatedOrderExecutionDataStore::
      ~ReplicatedOrderExecutionDataStore() {
    Close();
  }

  inline boost::optional<SequencedAccountOrderRecord>
      ReplicatedOrderExecutionDataStore::LoadOrder(OrderId id) {
    if(m_duplicateDataStores.empty()) {
      return m_primaryDataStore.LoadOrder(id);
    }
    auto index = ++m_nextDataStore;
    index = index % m_duplicateDataStores.size();
    return m_duplicateDataStores[index].LoadOrder(id);
  }

  inline std::vector<SequencedOrderRecord>
      ReplicatedOrderExecutionDataStore::LoadOrderSubmissions(
      const AccountQuery& query) {
    if(m_duplicateDataStores.empty()) {
      return m_primaryDataStore.LoadOrderSubmissions(query);
    }
    auto index = ++m_nextDataStore;
    index = index % m_duplicateDataStores.size();
    return m_duplicateDataStores[index].LoadOrderSubmissions(query);
  }

  inline std::vector<SequencedExecutionReport>
      ReplicatedOrderExecutionDataStore::LoadExecutionReports(
      const AccountQuery& query) {
    if(m_duplicateDataStores.empty()) {
      return m_primaryDataStore.LoadExecutionReports(query);
    }
    auto index = ++m_nextDataStore;
    index = index % m_duplicateDataStores.size();
    return m_duplicateDataStores[index].LoadExecutionReports(query);
  }

  inline void ReplicatedOrderExecutionDataStore::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_primaryDataStore.Store(orderInfo);
    for(auto& dataStore : m_duplicateDataStores) {
      dataStore.Store(orderInfo);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_primaryDataStore.Store(executionReport);
    for(auto& dataStore : m_duplicateDataStores) {
      dataStore.Store(executionReport);
    }
  }

  inline void ReplicatedOrderExecutionDataStore::Close() {
    for(auto& dataStore : m_duplicateDataStores) {
      dataStore.Close();
    }
    m_primaryDataStore.Close();
  }
}

#endif
