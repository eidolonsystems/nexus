#ifndef SPIRE_STYLES_SELECTOR_HPP
#define SPIRE_STYLES_SELECTOR_HPP
#include <any>
#include <typeindex>
#include <type_traits>
#include <Beam/Utilities/Functional.hpp>
#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/StateSelector.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Defines what widget a style rule applies to. */
  class Selector {
    public:

      /** Constructs a Selector for any component. */
      Selector(Any any);

      /**
       * Constructs a Selector for a StateSelector.
       * @param state The state to represent.
       */
      template<typename T>
      Selector(StateSelector<T> state);

      /** Returns the underlying selector's type. */
      std::type_index get_type() const;

      /** Casts the underlying selector to a specified type. */
      template<typename U>
      const U& as() const;

      /**
       * Applies a callable to the underlying selector.
       * @param f The callable to apply.
       */
      template<typename F>
      decltype(auto) visit(F&& f) const;

      template<typename F, typename... G>
      decltype(auto) visit(F&& f, G&&... g) const;

    private:
      template<typename T>
      struct TypeExtractor {};
      template<typename T>
      struct TypeExtractor<Beam::TypeSequence<T>> {
        using type = std::decay_t<T>;
      };
      template<typename T, typename U>
      struct TypeExtractor<Beam::TypeSequence<T, U>> {
        using type = std::decay_t<U>;
      };
      std::any m_selector;
  };

  template<typename T>
  Selector::Selector(StateSelector<T> state)
    : m_selector(std::move(selector)) {}

  template<typename U>
  const U& Selector::as() const {
    return std::any_cast<const U&>(m_property);
  }

  template<typename F>
  decltype(auto) Selector::visit(F&& f) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_property.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_property));
    }
    throw std::bad_any_cast();
  }

  template<typename F, typename... G>
  decltype(auto) Selector::visit(F&& f, G&&... g) const {
    using Parameter = typename TypeExtractor<
      Beam::GetFunctionParameters<std::decay_t<F>>>::type;
    if(m_property.type() == typeid(Parameter)) {
      return std::forward<F>(f)(std::any_cast<const Parameter&>(m_property));
    }
    return visit(std::forward<G>(g)...);
  }
}

#endif
