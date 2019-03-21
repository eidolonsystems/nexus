#ifndef SPIRE_QT_PROMISE_HPP
#define SPIRE_QT_PROMISE_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/noncopyable.hpp>
#include <QApplication>
#include "spire/spire/chained_qt_promise.hpp"
#include "spire/spire/qt_promise_imp.hpp"
#include "spire/spire/spire.hpp"

namespace Spire {

  /*! \brief Performs an asyncronous computation and signals the result within a
             Qt-thread in a thread-safe manner.
      \tparam T The type of value to compute.
  */
  template<typename T>
  class QtPromise : private boost::noncopyable {
    public:

      //! The type of value to compute.
      using Type = T;

      //! Constructs an empty promise.
      QtPromise() = default;

      //! Constructs a Qt promise.
      /*!
        \param executor The callable performing the computation.
        \param launch_policy Specifies how the executor should be invoked.
      */
      template<typename Executor>
      explicit QtPromise(Executor&& executor, LaunchPolicy launch_policy =
        LaunchPolicy::DEFERRED);

      QtPromise(QtPromise&& other);

      ~QtPromise();

      //! Assigns a function to be called when the computation completes.
      /*!
        \param continuation The function to call when the computation completes.
      */
      template<typename F>
      std::enable_if_t<std::is_same_v<std::invoke_result_t<F, Beam::Expect<T>>,
        void>> then(F&& continuation);

      //! Assigns a function to be called when the computation completes.
      /*!
        \param continuation The function to call when the computation completes.
      */
      template<typename F>
      std::enable_if_t<!std::is_same_v<std::invoke_result_t<F, Beam::Expect<T>>,
        void>, QtPromise<promise_executor_result_t<F, Beam::Expect<T>>>> then(
        F&& continuation);

      //! Disconnects from this promise, upon disconnection the callback
      //! will not be called when the executor completes its computation.
      void disconnect();

      //! Disconnects this promise and then moves another promise into this.
      QtPromise& operator =(QtPromise&& other);

    private:
      template<typename> friend class QtPromise;
      std::shared_ptr<details::BaseQtPromiseImp<Type>> m_imp;

      template<typename U, typename F>
      QtPromise(QtPromise<U> promise, F&& continuation);
  };

  template<typename Executor>
  QtPromise(Executor&&) -> QtPromise<promise_executor_result_t<Executor>>;

  template<typename Executor>
  QtPromise(Executor&&, LaunchPolicy) ->
    QtPromise<promise_executor_result_t<Executor>>;

  template<typename U, typename F>
  QtPromise(QtPromise<U>, F&&) -> QtPromise<promise_executor_result_t<F>>;

  QtPromise() -> QtPromise<void>;

  //! Returns a promise that signals the result only when all provided
  //! promises have completed, or throws an exception if any provided
  //! promise throws an exception.
  /*
    \param promises The promises to be executed.
  */
  template<typename T>
  QtPromise<std::vector<T>> all(std::vector<QtPromise<T>> promises) {
    if(promises.empty()) {
      return QtPromise(
        [] {
          return std::vector<T>();
        });
    }
    auto completed_promises = std::make_shared<std::vector<T>>();
    auto promise = std::move(promises.front());
    for(auto i = std::size_t(0); i < promises.size() - 1; ++i) {
      promise = promise.then(
        [=, p = std::move(promises[i + 1])]
        (auto&& result) mutable {
          completed_promises->push_back(std::move(result.Get()));
          return std::move(p);
        });
    }
    return promise.then(
      [=] (auto&& result) {
        completed_promises->push_back(std::move(result.Get()));
        return std::move(*completed_promises);
      });
  }

  //! Waits for a promise to complete and returns its result.
  /*!
    \param promise The promise to wait for.
    \return The result of the promise's execution.
  */
  template<typename T>
  T wait(QtPromise<T>& promise) {
    auto future = std::optional<Beam::Expect<T>>();
    promise.then(
      [&] (auto result) {
        future.emplace(std::move(result));
      });
    while(!future.has_value()) {
      QApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
    return std::move(future->Get());
  }

  //! Waits for a promise to complete and returns its result.
  /*!
    \param promise The promise to wait for.
    \return The result of the promise's execution.
  */
  template<typename T>
  T wait(QtPromise<T> promise) {
    return static_cast<T (*)(QtPromise<T>&)>(wait)(promise);
  }

  template<typename T>
  template<typename Executor>
  QtPromise<T>::QtPromise(Executor&& executor, LaunchPolicy launch_policy) {
    auto imp = std::make_shared<details::qt_promise_imp<Executor>>(
      std::forward<Executor>(executor), launch_policy);
    imp->bind(imp);
    m_imp = std::move(imp);
  }

  template<typename T>
  QtPromise<T>::QtPromise(QtPromise&& other)
      : m_imp(std::move(other.m_imp)) {}

  template<typename T>
  QtPromise<T>::~QtPromise() {
    disconnect();
  }

  template<typename T>
  template<typename F>
  std::enable_if_t<std::is_same_v<std::invoke_result_t<F, Beam::Expect<T>>,
      void>> QtPromise<T>::then(F&& continuation) {
    m_imp->then(std::forward<F>(continuation));
  }

  template<typename T>
  template<typename F>
  std::enable_if_t<!std::is_same_v<std::invoke_result_t<F, Beam::Expect<T>>,
      void>, QtPromise<promise_executor_result_t<F, Beam::Expect<T>>>>
      QtPromise<T>::then(F&& continuation) {
    return QtPromise<promise_executor_result_t<F, Beam::Expect<T>>>(
      std::move(*this), std::forward<F>(continuation));
  }

  template<typename T>
  void QtPromise<T>::disconnect() {
    if(m_imp == nullptr) {
      return;
    }
    m_imp->disconnect();
    m_imp = nullptr;
  }

  template<typename T>
  QtPromise<T>& QtPromise<T>::operator =(QtPromise&& other) {
    disconnect();
    m_imp = std::move(other.m_imp);
    return *this;
  }

  template<typename T>
  template<typename U, typename F>
  QtPromise<T>::QtPromise(QtPromise<U> promise, F&& continuation) {
    auto chain = make_chained_qt_promise(std::move(promise),
      std::forward<F>(continuation));
    chain->bind(chain);
    m_imp = std::move(chain);
  }
}

#endif
