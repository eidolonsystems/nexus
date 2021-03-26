#ifndef SPIRE_LOCAL_SCALAR_VALUE_MODEL_HPP
#define SPIRE_LOCAL_SCALAR_VALUE_MODEL_HPP
#include <utility>
#include <boost/optional/optional.hpp>
#include "Spire/Ui/LocalValueModel.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/ValueModel.hpp"

namespace Spire {

  /**
   * Implements a ScalarValueModel by storing and updating a local value.
   * @param <T> The type of value to model.
   */
  template<typename T>
  class LocalScalarValueModel : public ScalarValueModel<T> {
    public:
      using Type = typename ScalarValueModel<T>::Type;

      using CurrentSignal = typename ScalarValueModel<T>::CurrentSignal;

      /** Constructs a default model. */
      LocalScalarValueModel() = default;

      /**
       * Constructs a model with an initial current value.
       * @param current The initial current value.
       */
      LocalScalarValueModel(Type current);

      /** Sets the minimum value assignable to current. */
      void set_minimum(const boost::optional<Type>& minimum);

      /** Sets the maximum value assignable to current. */
      void set_maximum(const boost::optional<Type>& maximum);

      /** Sets the increment. */
      void set_increment(const Type& increment);

      boost::optional<Type> get_minimum() const override;

      boost::optional<Type> get_maximum() const override;

      Type get_increment() const override;

      const Type& get_current() const override;

      QValidator::State set_current(const Type& value) override;

      boost::signals2::connection connect_current_signal(
        const typename CurrentSignal::slot_type& slot) const override;

    private:
      LocalValueModel<Type> m_model;
      boost::optional<Type> m_minimum;
      boost::optional<Type> m_maximum;
      Type m_increment;
  };

  template<typename T>
  LocalScalarValueModel<T>::LocalScalarValueModel(Type current)
    : m_model(std::move(current)) {}

  template<typename T>
  void LocalScalarValueModel<T>::set_minimum(
      const boost::optional<Type>& minimum) {
    m_minimum = minimum;
  }

  template<typename T>
  void LocalScalarValueModel<T>::set_maximum(
      const boost::optional<Type>& maximum) {
    m_maximum = maximum;
  }

  template<typename T>
  void LocalScalarValueModel<T>::set_increment(const Type& increment) {
    m_increment = increment;
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Type>
      LocalScalarValueModel<T>::get_minimum() const {
    return m_minimum;
  }

  template<typename T>
  boost::optional<typename LocalScalarValueModel<T>::Type>
      LocalScalarValueModel<T>::get_maximum() const {
    return m_maximum;
  }

  template<typename T>
  typename LocalScalarValueModel<T>::Type
      LocalScalarValueModel<T>::get_increment() const {
    return m_increment;
  }

  template<typename T>
  const typename LocalScalarValueModel<T>::Type&
      LocalScalarValueModel<T>::get_current() const {
    return m_model.get_current();
  }

  template<typename T>
  QValidator::State LocalScalarValueModel<T>::set_current(const Type& value) {
    using namespace std;
    if constexpr(std::numeric_limits<Type>::is_integer) {
      if(value != m_model.get_current() && (value % m_increment) != 0) {
        return QValidator::State::Invalid;
      }
    } else {
      if(value != m_model.get_current() && fmod(value, m_increment) != 0) {
        return QValidator::State::Invalid;
      }
    }
    auto& min = [&] () -> Type& {
      if(m_minimum) {
        return *m_minimum;
      }
      return value;
    }();
    auto& max = [&] () -> Type& {
      if(m_maximum) {
        return *m_maximum;
      }
      return value;
    }();
    if(value < min) {
    }
    if(value > max) {
      return QValidator::State::Invalid;
    }
    if(value < min) {
    }
    return m_model.set_current(value);
  }

  template<typename T>
  boost::signals2::connection LocalScalarValueModel<T>::connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const {
    return m_model.connect_current_signal(slot);
  }
}

#endif
