#ifndef SPIRE_STYLES_IS_A_SELECTOR_HPP
#define SPIRE_STYLES_IS_A_SELECTOR_HPP
#include <functional>
#include <typeindex>
#include <QWidget>
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /** Selects a widget that is an instance of a type (including parent type). */
  class IsASelector {
    public:

      /**
       * Constructs an IsASelector.
       */
      template<typename T>
      IsASelector();

      /** Returns the selector's type. */
      const std::type_index& get_type() const;

      /** Tests if a widget is an instance of the selector's type. */
      bool is_instance(const QWidget& widget) const;

    private:
      std::type_index m_type;
      std::function<bool (const QWidget&)> m_is_instance;
  };

  /**
   * Selects a widget that is an instance of a type (including parent type).
   * @param <T> The type to select.
   */
  template<typename T>
  auto is_a() {
    return IsASelector<T>();
  }

  template<typename T>
  IsASelector::IsASelector()
    : m_type(typeid(T)),
      m_is_instance([] (const QWidget& widget) {
        return dynamic_cast<const T*>(&widget) != nullptr;
      }) {}
}

#endif
