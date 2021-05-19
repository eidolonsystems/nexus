#ifndef SPIRE_STYLES_EXPRESSION_HPP
#define SPIRE_STYLES_EXPRESSION_HPP
#include <any>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Spire/Styles/ConstantExpression.hpp"
#include "Spire/Styles/Evaluator.hpp"
#include "Spire/Styles/Styles.hpp"

namespace Spire::Styles {
  template<typename T, typename = void>
  struct is_expression_t : std::false_type {};

  template<typename T>
  struct is_expression_t<T, std::void_t<decltype(
    make_evaluator(std::declval<T>(), std::declval<const Stylist&>()))>> :
      std::true_type {};

  template<typename T>
  constexpr auto is_expression_v = is_expression_t<T>::value;

  /**
   * Represents an expression performed on a style.
   * @param <T> The type the expression evaluates to.
   */
  template<typename T>
  class Expression {
    public:

      /** The type the expression evaluates to. */
      using Type = T;

      /**
       * Constructs an Expression evaluating to a constant.
       * @param constant The constant to represent.
       */
      Expression(Type constant);

      template<typename E, typename = std::enable_if_t<is_expression_v<E>>>
      Expression(E expression);

      /** Casts the underlying expression to a specified type. */
      template<typename U>
      const U& as() const;

      bool operator ==(const Expression& expression) const;

      bool operator !=(const Expression& expression) const;

    private:
      template<typename U>
      friend Evaluator<typename Expression<U>::Type>
        make_evaluator(const Expression<U>& expression, const Stylist& stylist);
      std::any m_expression;
      std::function<bool (const Expression&, const Expression&)> m_is_equal;
      std::function<Evaluator<Type> (const Expression&, const Stylist&)>
        m_make_evaluator;

      Evaluator<Type> make_evaluator(const Stylist& stylist) const;
  };

  /**
   * Returns a function that can be used to evaluate an Expression.
   * @param expression The expression to evaluate.
   * @param stylist The Stylist is used to provide context to the evaluator.
   * @return A function that can be used to evaluate the <i>expression</i>.
   */
  template<typename T>
  Evaluator<typename Expression<T>::Type> make_evaluator(
      const Expression<T>& expression, const Stylist& stylist) {
    return expression.make_evaluator(stylist);
  }

  /**
   * Provides a helper template class for a generic expression definition.
   * @param <I> A unique struct ID/tag used to differentiate definitions.
   * @param <T> The type the expression evaluates to.
   * @param <A> The type of the expression's arguments.
   */
  template<typename I, typename T, typename... A>
  class ExpressionDefinition {
    public:

      /** The type the expression evaluates to. */
      using Type = T;

      /** The tuple of arguments. */
      using Arguments = std::tuple<Expression<A>...>;

      /**
       * Constructs an ExpressionDefinition.
       * @param arguments The arguments used to evaluate the expression.
       */
      explicit ExpressionDefinition(Expression<A>... arguments);

      /** Returns the tuple of arguments. */
      const Arguments& get_arguments() const;

      bool operator ==(const ExpressionDefinition& expression) const;

      bool operator !=(const ExpressionDefinition& expression) const;

    private:
      Arguments m_arguments;
  };

  template<typename T>
  Expression<T>::Expression(Type value)
    : Expression(ConstantExpression(std::move(value))) {}

  template<typename T>
  template<typename E, typename>
  Expression<T>::Expression(E expression)
    : m_expression(std::move(expression)),
      m_is_equal([] (const Expression& left, const Expression& right) {
        return left.m_expression.type() == right.m_expression.type() &&
          left.as<E>() == right.as<E>();
      }),
      m_make_evaluator(
        [] (const Expression& expression, const Stylist& stylist) {
          return Spire::Styles::make_evaluator(expression.as<E>(), stylist);
        }) {}

  template<typename T>
  template<typename U>
  const U& Expression<T>::as() const {
    return std::any_cast<const U&>(m_expression);
  }

  template<typename T>
  bool Expression<T>::operator ==(const Expression& expression) const {
    return m_is_equal(*this, expression);
  }

  template<typename T>
  bool Expression<T>::operator !=(const Expression& expression) const {
    return !(*this == expression);
  }

  template<typename T>
  Evaluator<typename Expression<T>::Type>
      Expression<T>::make_evaluator(const Stylist& stylist) const {
    return m_make_evaluator(*this, stylist);
  }

  template<typename I, typename T, typename... A>
  ExpressionDefinition<I, T, A...>::ExpressionDefinition(
    Expression<A>... arguments)
    : m_arguments(std::move(arguments)...) {}

  template<typename I, typename T, typename... A>
  const typename ExpressionDefinition<I, T, A...>::Arguments&
      ExpressionDefinition<I, T, A...>::get_arguments() const {
    return m_arguments;
  }

  template<typename I, typename T, typename... A>
  bool ExpressionDefinition<I, T, A...>::operator ==(
      const ExpressionDefinition& expression) const {
    return m_arguments == expression.m_arguments;
  }

  template<typename I, typename T, typename... A>
  bool ExpressionDefinition<I, T, A...>::operator !=(
      const ExpressionDefinition& expression) const {
    return !(*this == expression);
  }
}

#endif
