#include <deque>
#include <doctest/doctest.h>
#include "Spire/Ui/ArrayTableModel.hpp"
#include "Spire/Ui/SortedTableModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  bool test_comparator(const std::any& lhs, const std::any& rhs) {
    if(lhs.type() == typeid(int)) {
      return std::any_cast<int>(lhs) < std::any_cast<int>(rhs);
    } else if(lhs.type() == typeid(std::string)) {
      return std::any_cast<std::string>(lhs) <
        std::any_cast<std::string>(rhs);
    } else if(lhs.type() == typeid(float)) {
      return std::any_cast<float>(lhs) < std::any_cast<float>(rhs);
    }
    return false;
  }

  template<typename T>
  std::vector<T> column_span(const TableModel& model, int column) {
    auto span = std::vector<T>();
    for(auto i = 0; i != model.get_row_size(); ++i) {
      span.push_back(model.get<T>(i, column));
    }
    return span;
  }
}

TEST_SUITE("SortedTableModel") {
  TEST_CASE("sort") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source,
      {{0, SortedTableModel::Ordering::ASCENDING}}, test_comparator);
    auto order = sorted_model.get_column_order();
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{1, 2, 4, 9});
    order.front() = cycle(order.front());
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{9, 4, 2, 1});
    order.front() = cycle(order.front());
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 2, 9, 1});
    order.front() = cycle(order.front());
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{1, 2, 4, 9});
  }

  TEST_CASE("sort_constructor_without_order_comparator") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 2, 9, 1});
  }

  TEST_CASE("sort_constructor_without_comparator") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source,
      {{0, SortedTableModel::Ordering::DESCENDING}});
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{9, 4, 2, 1});
  }

  TEST_CASE("sort_constructor_without_order") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4});
    source->push({2});
    source->push({9});
    source->push({1});
    auto sorted_model = SortedTableModel(source, test_comparator);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 2, 9, 1});
  }

  TEST_CASE("sort_multiple_columns") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::DESCENDING});
    order.push_back({2, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Joe", "Jack"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{4.1f, 9.2f, 3.7f, 7.8f, 7.8f, 1.3f});
    order.clear();
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({2, SortedTableModel::Ordering::DESCENDING});
    sorted_model.set_column_order(order);
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "Jack", "Joe", "John", "Liam"});
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 9, 9, 4, 6});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 1.3f, 7.8f, 3.7f, 7.8f});
  }

  TEST_CASE("sort_cycle") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::NONE});
    order.push_back({1, SortedTableModel::Ordering::NONE});
    order.push_back({2, SortedTableModel::Ordering::NONE});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 2, 9, 2, 6, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"John", "Bob", "Jack", "Bob", "Liam", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{3.7f, 9.2f, 1.3f, 4.1f, 7.8f, 7.8f});
    std::transform(order.begin(), order.end(), order.begin(), cycle);
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{4.1f, 9.2f, 3.7f, 7.8f, 1.3f, 7.8f});
    std::transform(order.begin(), order.end(), order.begin(), cycle);
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{9, 9, 6, 4, 2, 2});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Joe", "Jack", "Liam", "John", "Bob", "Bob"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{7.8f, 1.3f, 7.8f, 3.7f, 9.2f, 4.1f});
    std::transform(order.begin(), order.end(), order.begin(), cycle);
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 2, 9, 2, 6, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"John", "Bob", "Jack", "Bob", "Liam", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{3.7f, 9.2f, 1.3f, 4.1f, 7.8f, 7.8f});
  }

  TEST_CASE("sort_adjust_order1") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    order.push_back({2, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{4.1f, 9.2f, 3.7f, 7.8f, 1.3f, 7.8f});
    adjust({1, SortedTableModel::Ordering::DESCENDING}, order);
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{6, 4, 9, 9, 2, 2});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Liam", "John", "Joe", "Jack", "Bob", "Bob"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{7.8f, 3.7f, 7.8f, 1.3f, 4.1f, 9.2f});
  }

  TEST_CASE("sort_adjust_order2") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 7.8f, 1.3f, 7.8f});
    adjust({2, SortedTableModel::Ordering::ASCENDING}, order);
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{9, 4, 2, 6, 9, 2});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Jack", "John", "Bob", "Liam", "Joe", "Bob"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{1.3f, 3.7f, 4.1f, 7.8f, 7.8f, 9.2f});
    adjust({0, SortedTableModel::Ordering::DESCENDING}, order);
    sorted_model.set_column_order(order);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{9, 9, 6, 4, 2, 2});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Jack", "Joe", "Liam", "John", "Bob", "Bob"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{1.3f, 7.8f, 7.8f, 3.7f, 4.1f, 9.2f});
  }

  TEST_CASE("push") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    auto operations = std::deque<TableModel::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 7.8f, 1.3f, 7.8f});
    source->push({2, std::string("Tom"), 3.0f});
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 2);
    REQUIRE(column_span<int>(sorted_model, 0) ==
      std::vector{2, 2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{
        "Bob", "Bob", "Tom", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.0f, 3.7f, 7.8f, 1.3f, 7.8f});
    source->push({7, std::string("Smith"), 4.4f});
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 5);
    REQUIRE(column_span<int>(sorted_model, 0) ==
      std::vector{2, 2, 2, 4, 6, 7, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{
        "Bob", "Bob", "Tom", "John", "Liam", "Smith", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.0f, 3.7f, 7.8f, 4.4f, 1.3f, 7.8f});
  }

  TEST_CASE("insert") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    auto operations = std::deque<TableModel::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 7.8f, 1.3f, 7.8f});
    source->insert({2, std::string("Tom"), 3.0f}, 4);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 2);
    REQUIRE(column_span<int>(sorted_model, 0) ==
      std::vector{2, 2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{
        "Bob", "Bob", "Tom", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.0f, 3.7f, 7.8f, 1.3f, 7.8f});
    source->insert({7, std::string("Smith"), 4.4f}, 0);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 5);
    REQUIRE(column_span<int>(sorted_model, 0) ==
      std::vector{2, 2, 2, 4, 6, 7, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{
        "Bob", "Bob", "Tom", "John", "Liam", "Smith", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.0f, 3.7f, 7.8f, 4.4f, 1.3f, 7.8f});
  }

  TEST_CASE("add_without_order") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::NONE});
    order.push_back({1, SortedTableModel::Ordering::NONE});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    auto operations = std::deque<TableModel::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    source->push({1, std::string("Tom"), 3.0f});
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 3);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 2, 9, 1});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"John", "Bob", "Jack", "Tom"});
    REQUIRE(sorted_model.get_row_size() == 4);
    source->insert({7, std::string("Smith"), 4.4f}, 1);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 1);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 7, 2, 9, 1});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"John", "Smith", "Bob", "Jack", "Tom"});
    REQUIRE(sorted_model.get_row_size() == 5);
    source->remove(2);
    order.clear();
    sorted_model.set_column_order(order);
    operations.clear();
    source->insert({0, std::string("Liam"), 1.4f}, 3);
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    add_operation = get<TableModel::AddOperation>(&operation);
    REQUIRE(add_operation);
    REQUIRE(add_operation->m_index == 3);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{4, 7, 9, 0, 1});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"John", "Smith", "Jack", "Liam", "Tom"});
    REQUIRE(sorted_model.get_row_size() == 5);
  }

  TEST_CASE("remove") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    auto operations = std::deque<TableModel::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { operations.push_back(operation); });
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 7.8f, 1.3f, 7.8f});
    source->remove(2);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto remove_operation = get<TableModel::RemoveOperation>(&operation);
    REQUIRE(remove_operation);
    REQUIRE(remove_operation->m_index == 4);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 7.8f, 7.8f});
  }

  TEST_CASE("update") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({2, std::string("Bob"), 4.1f});
    source->push({6, std::string("Liam"), 7.8f});
    source->push({9, std::string("Joe"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::ASCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    auto operations = std::deque<TableModel::Operation>();
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) {
        visit(operation, [&] (const auto& operation) {
          operations.push_back(operation);
        });
      });
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 7.8f, 1.3f, 7.8f});
    source->set(4, 2, 10.0f);
    REQUIRE(operations.size() == 1);
    auto operation = operations.front();
    operations.pop_front();
    auto update_operation = get<TableModel::UpdateOperation>(&operation);
    REQUIRE(update_operation);
    REQUIRE(update_operation->m_row == 3);
    REQUIRE(update_operation->m_column == 2);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Bob", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{9.2f, 4.1f, 3.7f, 10.0f, 1.3f, 7.8f});
    source->set(3, 1, std::string("Ava"));
    REQUIRE(operations.size() == 2);
    operation = operations.front();
    operations.pop_front();
    auto move_operation = get<TableModel::MoveOperation>(&operation);
    REQUIRE(move_operation);
    REQUIRE(move_operation->m_source == 1);
    REQUIRE(move_operation->m_destination == 0);
    operation = operations.front();
    operations.pop_front();
    update_operation = get<TableModel::UpdateOperation>(&operation);
    REQUIRE(update_operation);
    REQUIRE(update_operation->m_row == 0);
    REQUIRE(update_operation->m_column == 1);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{2, 2, 4, 6, 9, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Ava", "Bob", "John", "Liam", "Jack", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{4.1f, 9.2f, 3.7f, 10.0f, 1.3f, 7.8f});
    source->set(2, 0, 0);
    REQUIRE(operations.size() == 2);
    operation = operations.front();
    operations.pop_front();
    move_operation = get<TableModel::MoveOperation>(&operation);
    REQUIRE(move_operation);
    REQUIRE(move_operation->m_source == 4);
    REQUIRE(move_operation->m_destination == 0);
    operation = operations.front();
    operations.pop_front();
    update_operation = get<TableModel::UpdateOperation>(&operation);
    REQUIRE(update_operation);
    REQUIRE(update_operation->m_row == 0);
    REQUIRE(update_operation->m_column == 0);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{0, 2, 2, 4, 6, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Jack", "Ava", "Bob", "John", "Liam", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{1.3f, 4.1f, 9.2f, 3.7f, 10.f, 7.8f});
    source->set(1, 1, std::string("Tom"));
    REQUIRE(operations.size() == 1);
    operation = operations.front();
    operations.pop_front();
    update_operation = get<TableModel::UpdateOperation>(&operation);
    REQUIRE(update_operation);
    REQUIRE(update_operation->m_row == 2);
    REQUIRE(update_operation->m_column == 1);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{0, 2, 2, 4, 6, 9});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Jack", "Ava", "Tom", "John", "Liam", "Joe"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{1.3f, 4.1f, 9.2f, 3.7f, 10.0f, 7.8f});
  }

  TEST_CASE("multiple_operations") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({4, std::string("John"), 3.7f});
    source->push({2, std::string("Bob"), 9.2f});
    source->push({9, std::string("Jack"), 1.3f});
    source->push({6, std::string("Liam"), 7.8f});
    auto order = std::vector<SortedTableModel::ColumnOrder>();
    order.push_back({0, SortedTableModel::Ordering::DESCENDING});
    order.push_back({1, SortedTableModel::Ordering::ASCENDING});
    order.push_back({2, SortedTableModel::Ordering::ASCENDING});
    auto sorted_model = SortedTableModel(source, order, test_comparator);
    auto signal_count = 0;
    sorted_model.connect_operation_signal(
      [&] (const auto& operation) { ++signal_count; });
    source->push({2, std::string("Ava"), 4.1f});
    source->set(3, 0, 1);
    source->insert({6, std::string("Tom"), 11.1f}, 2);
    source->set(5, 1, std::string("Bob"));
    REQUIRE(signal_count == 4);
    REQUIRE(column_span<int>(sorted_model, 0) == std::vector{9, 6, 4, 2, 2, 1});
    REQUIRE(column_span<std::string>(sorted_model, 1) ==
      std::vector<std::string>{"Jack", "Tom", "John", "Bob", "Bob", "Liam"});
    REQUIRE(column_span<float>(sorted_model, 2) ==
      std::vector{1.3f, 11.1f, 3.7f, 4.1f, 9.2f, 7.8f});
  }
}
