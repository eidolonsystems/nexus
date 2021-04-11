#ifndef SPIRE_STATE_SELECTOR_HPP
#define SPIRE_STATE_SELECTOR_HPP
#include <utility>
#include "Spire/Styles/Styles.hpp"
#include "Spire/Styles/Stylist.hpp"

namespace Spire::Styles {

  /**
   * Selects a widget that is in a given state.
   * @param <T> The type of data associated with the state.
   * @param <G> A unique type tag.
   */
  template<typename T, typename G>
  class StateSelector {
    public:

      /** The type of data associated with the state. */
      using Type = T;

      /** A unique tag used to identify the state. */
      using Tag = G;

      /**
       * Constructs a StateSelector.
       * @param data The data associated with the state.
       */
      explicit StateSelector(Type data);

      /** Returns the associated data. */
      const Type& get_data() const;

      bool operator ==(const StateSelector& selector) const;

      bool operator !=(const StateSelector& selector) const;

    private:
      Type m_data;
  };

  template<typename G>
  class StateSelector<void, G> {
    public:
      using Tag = G;

      bool operator ==(const StateSelector& selector) const;

      bool operator !=(const StateSelector& selector) const;
  };

  template<typename T, typename G>
  std::vector<Stylist*> select(
      const StateSelector<T, G>& selector, Stylist& source) {
    if(source.is_match(selector)) {
      return {&source};
    }
    return {};
  }

  template<typename T, typename G>
  StateSelector<T, G>::StateSelector(Type data)
    : m_data(std::move(data)) {}

  template<typename T, typename G>
  const typename StateSelector<T, G>::Type&
      StateSelector<T, G>::get_data() const {
    return m_data;
  }

  template<typename T, typename G>
  bool StateSelector<T, G>::operator ==(const StateSelector& selector) const {
    return m_data == selector.m_data;
  }

  template<typename T, typename G>
  bool StateSelector<T, G>::operator !=(const StateSelector& selector) const {
    return !(*this == selector);
  }

  template<typename G>
  bool StateSelector<void, G>::operator ==(
      const StateSelector& selector) const {
    return true;
  }

  template<typename G>
  bool StateSelector<void, G>::operator !=(
      const StateSelector& selector) const {
    return !(*this == selector);
  }
}

#endif