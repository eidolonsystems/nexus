#ifndef SPIRE_STYLES_FUNCTION_DEFINITION_EXPRESSION_HPP
#define SPIRE_STYLES_FUNCTION_DEFINITION_EXPRESSION_HPP
#include <tuple>
#include <utility>
#include "Spire/Styles/Expression.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {

  /**
   * Provides a helper template class for an Expression representing a function.
   * @param <I> A unique struct ID/tag used to differentiate function
   *            definitions.
   * @param <T> The type the function evaluates to.
   * @param <A> The type of the function's arguments.
   */
  template<typename I, typename T, typename... A>
  class FunctionDefinitionExpression {
    public:

      /** The type the expression evaluates to. */
      using Type = T;

      /** The tuple of arguments. */
      using Arguments = std::tuple<Expression<A>...>;

      /**
       * Constructs a FunctionDefinitionExpression.
       * @param arguments The arguments used to evaluate the function.
       */
      explicit FunctionDefinitionExpression(Expression<A>... arguments);

      /** Returns the tuple of arguments. */
      const Arguments& get_arguments() const;

      bool operator ==(const FunctionDefinitionExpression& expression) const;

      bool operator !=(const FunctionDefinitionExpression& expression) const;

    private:
      Arguments m_arguments;
  };

  template<typename I, typename T, typename... A>
  FunctionDefinitionExpression<I, T, A...>::FunctionDefinitionExpression(
    Expression<A>... arguments)
    : m_arguments(std::move(arguments)...) {}

  template<typename I, typename T, typename... A>
  const typename FunctionDefinitionExpression<I, T, A...>::Arguments&
      FunctionDefinitionExpression<I, T, A...>::get_arguments() const {
    return m_arguments;
  }

  template<typename I, typename T, typename... A>
  bool FunctionDefinitionExpression<I, T, A...>::operator ==(
      const FunctionDefinitionExpression& expression) const {
    return m_arguments == expression.m_arguments;
  }

  template<typename I, typename T, typename... A>
  bool FunctionDefinitionExpression<I, T, A...>::operator !=(
      const FunctionDefinitionExpression& expression) const {
    return !(*this == expression);
  }
}

#endif
