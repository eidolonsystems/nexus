#ifndef NEXUS_EVALUATORTRANSLATOR_HPP
#define NEXUS_EVALUATORTRANSLATOR_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include <Beam/Queries/FunctionEvaluatorNode.hpp>
#include <Beam/Queries/MemberAccessEvaluatorNode.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/FunctionEvaluators.hpp"
#include "Nexus/Queries/Queries.hpp"
#include "Nexus/Queries/StandardDataTypes.hpp"

namespace Nexus {
namespace Queries {

  /*! \class EvaluatorTranslator
      \brief Translates an Expression into an EvaluatorNode.
   */
  class EvaluatorTranslator :
      public Beam::Queries::EvaluatorTranslator<QueryTypes>,
      public ExpressionVisitor {
    public:

      //! Constructs an EvaluatorTranslator.
      EvaluatorTranslator();

      //! Constructs an EvaluatorTranslator maintaining a set of live Orders.
      /*!
        \param liveOrders The set of live Orders.
      */
      EvaluatorTranslator(Beam::Ref<
        const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>>
        liveOrders);

      virtual std::unique_ptr<Beam::Queries::EvaluatorTranslator<QueryTypes>>
        NewTranslator() const override;

      virtual void Visit(const Beam::Queries::FunctionExpression& expression)
        override;

      virtual void Visit(
        const Beam::Queries::MemberAccessExpression& expression) override;

    private:
      const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>*
        m_liveOrders;

      void TranslateSecurityMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateTimeAndSaleMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateOrderFieldsMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
      void TranslateOrderInfoMemberAccessExpression(
        const Beam::Queries::MemberAccessExpression& expression);
  };

  inline EvaluatorTranslator::EvaluatorTranslator()
      : m_liveOrders{nullptr} {}

  inline EvaluatorTranslator::EvaluatorTranslator(Beam::Ref<
      const Beam::SynchronizedUnorderedSet<OrderExecutionService::OrderId>>
      liveOrders)
      : m_liveOrders{liveOrders.Get()} {}

  inline std::unique_ptr<Beam::Queries::EvaluatorTranslator<QueryTypes>>
      EvaluatorTranslator::NewTranslator() const {
    if(m_liveOrders == nullptr) {
      return std::make_unique<EvaluatorTranslator>();
    } else {
      return std::make_unique<EvaluatorTranslator>(Beam::Ref(*m_liveOrders));
    }
  }

  inline void EvaluatorTranslator::Visit(
      const Beam::Queries::FunctionExpression& expression) {
    if(expression.GetName() == Beam::Queries::ADDITION_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(Beam::Queries::ExpressionTranslationException(
          "Invalid parameters."));
      }
      const auto& leftExpression = *expression.GetParameters()[0];
      const auto& rightExpression = *expression.GetParameters()[1];
      std::function<Beam::Queries::BaseEvaluatorNode*
        (std::vector<std::unique_ptr<Beam::Queries::BaseEvaluatorNode>>)>
        translator;
      try {
        translator = Beam::Instantiate<
          Beam::Queries::FunctionEvaluatorNodeTranslator<
          AdditionExpressionTranslator>>(
          leftExpression.GetType()->GetNativeType(),
          rightExpression.GetType()->GetNativeType());
      } catch(const std::exception&) {
        Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
        return;
      }
      leftExpression.Apply(*this);
      std::vector<std::unique_ptr<Beam::Queries::BaseEvaluatorNode>> parameters;
      parameters.push_back(std::move(GetEvaluator()));
      rightExpression.Apply(*this);
      parameters.push_back(std::move(GetEvaluator()));
      std::unique_ptr<Beam::Queries::BaseEvaluatorNode> evaluator(
        translator(std::move(parameters)));
      SetEvaluator(std::move(evaluator));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::Visit(
      const Beam::Queries::MemberAccessExpression& expression) {
    if(expression.GetExpression()->GetType() == SecurityType()) {
      TranslateSecurityMemberAccessExpression(expression);
      return;
    } else if(expression.GetExpression()->GetType() == TimeAndSaleType()) {
      TranslateTimeAndSaleMemberAccessExpression(expression);
      return;
    } else if(expression.GetExpression()->GetType() == OrderFieldsType()) {
      TranslateOrderFieldsMemberAccessExpression(expression);
      return;
    } else if(expression.GetExpression()->GetType() == OrderInfoType()) {
      TranslateOrderInfoMemberAccessExpression(expression);
      return;
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateSecurityMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto securityExpression = Beam::UniqueStaticCast<
      Beam::Queries::EvaluatorNode<Security>>(GetEvaluator());
    if(expression.GetName() == "symbol") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (Security security) {
          return security.GetSymbol();
        }, std::move(securityExpression)));
    } else if(expression.GetName() == "market") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (Security security) {
          return static_cast<std::string>(security.GetMarket().GetData());
        }, std::move(securityExpression)));
    } else if(expression.GetName() == "country") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [] (Security security) {
          return security.GetCountry();
        }, std::move(securityExpression)));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateTimeAndSaleMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto timeAndSaleExpression = Beam::UniqueStaticCast<
      Beam::Queries::EvaluatorNode<TimeAndSale>>(GetEvaluator());
    if(expression.GetName() == "timestamp") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<boost::posix_time::ptime,
        TimeAndSale>>(std::move(timeAndSaleExpression),
        &TimeAndSale::m_timestamp));
    } else if(expression.GetName() == "price") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Money, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_price));
    } else if(expression.GetName() == "size") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<Quantity, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_size));
    } else if(expression.GetName() == "market_center") {
      SetEvaluator(std::make_unique<
        Beam::Queries::MemberAccessEvaluatorNode<std::string, TimeAndSale>>(
        std::move(timeAndSaleExpression), &TimeAndSale::m_marketCenter));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateOrderFieldsMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto orderFieldsExpression = Beam::UniqueStaticCast<
      Beam::Queries::EvaluatorNode<OrderExecutionService::OrderFields>>(
      GetEvaluator());
    if(expression.GetName() == "security") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        Security, OrderExecutionService::OrderFields>>(
        std::move(orderFieldsExpression),
        &OrderExecutionService::OrderFields::m_security));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }

  inline void EvaluatorTranslator::TranslateOrderInfoMemberAccessExpression(
      const Beam::Queries::MemberAccessExpression& expression) {
    expression.GetExpression()->Apply(*this);
    auto orderInfoExpression = Beam::UniqueStaticCast<
      Beam::Queries::EvaluatorNode<OrderExecutionService::OrderInfo>>(
      GetEvaluator());
    if(expression.GetName() == "fields") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        OrderExecutionService::OrderFields, OrderExecutionService::OrderInfo>>(
        std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_fields));
    } else if(expression.GetName() == "order_id") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        OrderExecutionService::OrderId, OrderExecutionService::OrderInfo>>(
        std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_orderId));
    } else if(expression.GetName() == "shorting_flag") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        bool, OrderExecutionService::OrderInfo>>(std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_shortingFlag));
    } else if(expression.GetName() == "timestamp") {
      SetEvaluator(std::make_unique<Beam::Queries::MemberAccessEvaluatorNode<
        boost::posix_time::ptime, OrderExecutionService::OrderInfo>>(
        std::move(orderInfoExpression),
        &OrderExecutionService::OrderInfo::m_timestamp));
    } else if(expression.GetName() == "is_live") {
      SetEvaluator(Beam::Queries::MakeFunctionEvaluatorNode(
        [liveOrders = m_liveOrders] (
            const OrderExecutionService::OrderInfo& orderInfo) {
          if(liveOrders == nullptr) {
            return false;
          }
          return liveOrders->Contains(orderInfo.m_orderId);
        }, std::move(orderInfoExpression)));
    } else {
      Beam::Queries::EvaluatorTranslator<QueryTypes>::Visit(expression);
    }
  }
}
}

#endif
