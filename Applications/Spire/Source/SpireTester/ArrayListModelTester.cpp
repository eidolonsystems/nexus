#include <deque>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  template<typename... F>
  decltype(auto) test_operation(
      const ListModel::Operation& operation, F&&... f) {
    return visit(
      operation, std::forward<F>(f)..., [] (const auto&) { REQUIRE(false); });
  }
}

TEST_SUITE("ArrayListModel") {
  TEST_CASE("push") {
    auto model = ArrayListModel();
    REQUIRE(model.get_size() == 0);
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
      }));
    REQUIRE_NOTHROW(model.push(3));
    REQUIRE(operations.size() == 1);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get<int>(0) == 3);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == model.get_size() - 1);
    });
    REQUIRE_NOTHROW(model.push(2));
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get<int>(1) == 2);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == model.get_size() - 1);
    });
  }

  TEST_CASE("remove") {
    auto model = ArrayListModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.remove(0));
    auto operations = std::deque<ListModel::Operation>();
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
      });
    REQUIRE_NOTHROW(model.push(1));
    REQUIRE(operations.size() == 1);
    REQUIRE_NOTHROW(model.push(3));
    REQUIRE(operations.size() == 2);
    REQUIRE_NOTHROW(model.push(5));
    REQUIRE(operations.size() == 3);
    REQUIRE(model.get_size() == 3);
    operations.clear();
    REQUIRE_NOTHROW(model.remove(0));
    REQUIRE(operations.size() == 1);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get<int>(0) == 3);
    REQUIRE(model.get<int>(1) == 5);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::RemoveOperation& operation) {
      REQUIRE(operation.m_index == 0);
    });
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        REQUIRE(false);
      });
    REQUIRE_THROWS(model.remove(2));
  }

  TEST_CASE("move") {
    auto model = ArrayListModel();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        REQUIRE(false);
      }));
    REQUIRE_THROWS(model.move(1, 3));
    auto operations = std::deque<ListModel::Operation>();
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
      });
    REQUIRE_NOTHROW(model.push(1));
    REQUIRE(operations.size() == 1);
    REQUIRE_NOTHROW(model.push(4));
    REQUIRE(operations.size() == 2);
    REQUIRE_NOTHROW(model.push(7));
    REQUIRE(operations.size() == 3);
    REQUIRE_NOTHROW(model.push(10));
    REQUIRE(operations.size() == 4);
    REQUIRE(model.get_size() == 4);
    operations.clear();
    REQUIRE_THROWS(model.move(0, 4));
    REQUIRE(operations.empty());
    REQUIRE_THROWS(model.move(5, 1));
    REQUIRE(operations.empty());
    REQUIRE_NOTHROW(model.move(2, 2));
    REQUIRE(operations.empty());
    REQUIRE(model.get<int>(0) == 1);
    REQUIRE(model.get<int>(1) == 4);
    REQUIRE(model.get<int>(2) == 7);
    REQUIRE(model.get<int>(3) == 10);
    REQUIRE_NOTHROW(model.move(0, 3));
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 7);
    REQUIRE(model.get<int>(2) == 10);
    REQUIRE(model.get<int>(3) == 1);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::MoveOperation& operation) {
      REQUIRE(operation.m_source == 0);
      REQUIRE(operation.m_destination == 3);
    });
    REQUIRE_NOTHROW(model.move(3, 1));
    REQUIRE(model.get<int>(0) == 4);
    REQUIRE(model.get<int>(1) == 1);
    REQUIRE(model.get<int>(2) == 7);
    REQUIRE(model.get<int>(3) == 10);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::MoveOperation& operation) {
      REQUIRE(operation.m_source == 3);
      REQUIRE(operation.m_destination == 1);
    });
  }

  TEST_CASE("insert") {
    auto model = ArrayListModel();
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
      }));
    REQUIRE_THROWS(model.insert(1, 1));
    REQUIRE(model.get_size() == 0);
    REQUIRE(operations.empty());
    REQUIRE_NOTHROW(model.insert(1, 0));
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get<int>(0) == 1);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == 0);
    });
    REQUIRE_NOTHROW(model.insert(2, 0));
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 1);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == 0);
    });
    REQUIRE_THROWS(model.insert(3, -1));
    REQUIRE(model.get_size() == 2);
    REQUIRE(operations.empty());
    REQUIRE_NOTHROW(model.insert(3, 1));
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get<int>(0) == 2);
    REQUIRE(model.get<int>(1) == 3);
    REQUIRE(model.get<int>(2) == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == 1);
    });
  }

  TEST_CASE("update") {
    auto model = ArrayListModel();
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    REQUIRE(model.set(2, 0) == QValidator::State::Invalid);
    REQUIRE(model.get_size() == 0);
    REQUIRE(operations.empty());
    REQUIRE_NOTHROW(model.push(1));
    REQUIRE(operations.size() == 1);
    REQUIRE_NOTHROW(model.push(4));
    REQUIRE(operations.size() == 2);
    REQUIRE_NOTHROW(model.push(7));
    REQUIRE(operations.size() == 3);
    operations.clear();
    REQUIRE(model.set(0, 0) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get<int>(0) == 0);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::UpdateOperation& operation) {
      REQUIRE(operation.m_index == 0);
    });
    REQUIRE(model.set(2, 10) == QValidator::State::Acceptable);
    REQUIRE(model.get_size() == 3);
    REQUIRE(model.get<int>(2) == 10);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::UpdateOperation& operation) {
      REQUIRE(operation.m_index == 2);
    });
  }
  
  TEST_CASE("transactions") {
    auto model = ArrayListModel();
    auto operations = std::deque<ListModel::Operation>();
    auto connection = scoped_connection(model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
        auto transaction = get<ListModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 5);
        auto& operations = transaction->m_operations;
        REQUIRE(get<ListModel::AddOperation>(&operations[0]));
        REQUIRE(get<ListModel::UpdateOperation>(&operations[1]));
        REQUIRE(get<ListModel::AddOperation>(&operations[2]));
        REQUIRE(get<ListModel::RemoveOperation>(&operations[3]));
        REQUIRE(get<ListModel::AddOperation>(&operations[4]));
      }));
    model.transact([&] {
      model.push(1);
      model.transact([&] {
        model.set(0, 10);
        model.transact([&] {
          model.push(9);
          model.remove(1);
        });
        model.push(8);
      });
    });
    REQUIRE(operations.size() == 1);
    operations.clear();
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
        connection.disconnect();
        model.transact([&] {
          model.push(7);
        });
        auto transaction = get<ListModel::Transaction>(&operation);
        REQUIRE(transaction != nullptr);
        REQUIRE(transaction->m_operations.size() == 2);
        auto& operations = transaction->m_operations;
        REQUIRE(get<ListModel::AddOperation>(&operations[0]));
        REQUIRE(get<ListModel::AddOperation>(&operations[1]));
      });
    model.transact([&] {
      model.push(1);
      model.push(4);
    });
    REQUIRE(operations.size() == 1);
    operations.clear();
    connection = model.connect_operation_signal(
      [&] (const ListModel::Operation& operation) {
        operations.push_back(operation);
      });
    model.transact([&] {
      model.push(1);
    });
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    test_operation(operation, [&] (const ListModel::AddOperation& operation) {
      REQUIRE(operation.m_index == model.get_size() - 1);
    });
  }
}
