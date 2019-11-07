#include <catch.hpp>
#include "Spire/OrderImbalanceIndicator/FilteredOrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/LocalOrderImbalanceIndicatorModel.hpp"
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using boost::posix_time::ptime;
using boost::posix_time::from_time_t;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make(const std::string& symbol, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, 0), Side::BID, 100,
      Money(1 * Money::ONE), timestamp);
  }

  auto make(const std::string& symbol, const std::string& market, Side side,
      Quantity size, double ref_price, const ptime& timestamp) {
    return OrderImbalance(Security(symbol, market, 0), side, size,
      Money(Quantity(ref_price)), timestamp);
  }

  const auto a = make("A", "TSX", Side::BID, 100, 1.0, from_time_t(100));
  const auto b = make("B", "TSX", Side::BID, 1000, 10.0, from_time_t(200));
  const auto c = make("C", "TSX", Side::ASK, 10000, 100.0, from_time_t(300));
  const auto d = make("D", "NYSE", Side::ASK, 100000, 1000.0,
    from_time_t(400));
  const auto e = make("E", "NYSE", Side::ASK, 1000000, 10000.0,
    from_time_t(500));
}

auto make_local_model() {
    auto model = std::make_shared<LocalOrderImbalanceIndicatorModel>();
    model->insert(a);
    model->insert(b);
    model->insert(c);
    model->insert(d);
    model->insert(e);
    return model;
}

TEST_CASE("test_unfiltered_subscribing",
    "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model = FilteredOrderImbalanceIndicatorModel(make_local_model(), {});
    auto [connection1, promise1] = model.subscribe(from_time_t(100),
      from_time_t(300), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({a, b, c}));
    auto [connection2, promise2] = model.subscribe(from_time_t(200),
      from_time_t(400), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({b, c, d}));
    auto [connection3, promise3] = model.subscribe(from_time_t(300),
      from_time_t(500), [] (auto& i) {});
    auto data3 = wait(std::move(promise3));
    REQUIRE(data3 == std::vector<OrderImbalance>({c, d, e}));
  }, "test_unfiltered_subscribing");
}

TEST_CASE("test_list_filter", "[FilteredOrderImbalanceIndicatorModel]") {
  run_test([] {
    auto model1 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_list_filter({"A"})});
    auto [connection1, promise1] = model1.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data1 = wait(std::move(promise1));
    REQUIRE(data1 == std::vector<OrderImbalance>({a}));
    auto model2 = FilteredOrderImbalanceIndicatorModel(make_local_model(),
      {make_list_filter({"A", "C", "E"})});
    auto [connection2, promise2] = model2.subscribe(from_time_t(0),
      from_time_t(500), [] (auto& i) {});
    auto data2 = wait(std::move(promise2));
    REQUIRE(data2 == std::vector<OrderImbalance>({a, c, e}));
  }, "test_list_filter");
}