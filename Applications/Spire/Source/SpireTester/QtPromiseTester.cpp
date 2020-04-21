#include <doctest/doctest.h>
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

namespace {
  QtPromise<int> inc(int num, int end) {
    if(num != end) {
      return inc(num + 1, end);
    }
    return QtPromise<int>([=] { return num; });
  }
}

TEST_SUITE("QtPromise") {
  TEST_CASE("arbitrary_chaining") {
    run_test([] {
      auto end = 10;
      auto p = inc(1, end);
      auto r = wait(std::move(p));
      REQUIRE(r == 10);
    }, "arbitrary_chaining");
  }

  TEST_CASE("chaining_promise_then") {
    run_test([] {
      auto p = QtPromise(
        [] {
          return 123;
        }).then([] (auto result) {
          return QtPromise(
            [=] {
              return 2 * result.Get();
            }).then([] (auto result) {
              return 3 * result.Get();
            });
        }).then([] (auto result) {
          return 6 * result.Get();
        });
      auto r = wait(std::move(p));
      REQUIRE(r == 4428);
    }, "chaining_promise_then");
  }

  TEST_CASE("empty_promise") {
    run_test([] {
      auto promises = std::vector<QtPromise<std::vector<int>>>();
      auto result = wait(std::move(all(std::move(promises))));
      REQUIRE(result == std::vector<std::vector<int>>());
    }, "empty_promise");
  }

  TEST_CASE("single_promise") {
    run_test([] {
      auto promises = std::vector<QtPromise<int>>();
      promises.push_back(QtPromise([] {
        return 1;
      }));
      auto result = wait(std::move(all(std::move(promises))));
      REQUIRE(result == std::vector<int>{1});
    }, "single_promise");
  }

  TEST_CASE("multiple_promises") {
    run_test([] {
      auto promises = std::vector<QtPromise<int>>();
      promises.push_back(QtPromise([] {
        return 1;
      }));
      promises.push_back(QtPromise([] {
        return 2;
      }));
      promises.push_back(QtPromise([] {
        return 3;
      }));
      promises.push_back(QtPromise([] {
        return 4;
      }));
      auto all_promise = std::move(all(std::move(promises)));
      auto result = wait(std::move(all_promise));
      REQUIRE(result == std::vector<int>{1, 2, 3, 4});
    }, "multiple_promises");
  }

  TEST_CASE("move_only_type") {
    run_test([] {
      auto promises = std::vector<QtPromise<std::unique_ptr<int>>>();
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(1);
      }));
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(2);
      }));
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(3);
      }));
      promises.push_back(QtPromise([] {
        return std::make_unique<int>(4);
      }));
      auto expected_result = std::vector<std::unique_ptr<int>>();
      expected_result.push_back(std::make_unique<int>(1));
      expected_result.push_back(std::make_unique<int>(2));
      expected_result.push_back(std::make_unique<int>(3));
      expected_result.push_back(std::make_unique<int>(4));
      auto all_promise = all(std::move(promises));
      auto result = std::move(wait(std::move(all_promise)));
      REQUIRE(std::equal(result.begin(), result.end(), expected_result.begin(),
        expected_result.end(), [](const auto& lhs, const auto& rhs) {
          return *lhs == *rhs;
        }));
    }, "move_only_type");
  }

  TEST_CASE("void_all") {
    run_test([] {
      auto value = 0;
      auto promises = std::vector<QtPromise<void>>();
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      promises.push_back(QtPromise([&] {
        ++value;
      }));
      auto all_promise = all(std::move(promises));
      wait(std::move(all_promise));
      REQUIRE(value == 4);
    }, "void_all");
  }
}
