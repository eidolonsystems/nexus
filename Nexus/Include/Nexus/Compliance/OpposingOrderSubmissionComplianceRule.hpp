#ifndef NEXUS_OPPOSINGORDERSUBMISSIONCOMPLIANCERULE_HPP
#define NEXUS_OPPOSINGORDERSUBMISSIONCOMPLIANCERULE_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/TaggedQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/MapComplianceRule.hpp"
#include "Nexus/Compliance/SecurityFilterComplianceRule.hpp"
#include "Nexus/Compliance/TimeFilterComplianceRule.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class OpposingOrderSubmissionComplianceRule
      \brief Prevents opposing orders from being submitted after a cancellation.
      \tparam TimeClientType The type of TimeClient used to determine how much
              time has elapsed since the last cancellation.
   */
  template<typename TimeClientType>
  class OpposingOrderSubmissionComplianceRule : public ComplianceRule {
    public:

      //! The type of TimeClient used to determine how much time has elapsed
      //! since the last cancellation.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! Constructs an OpposingOrderSubmissionComplianceRule.
      /*!
        \param timeout The amount of time to restrict submissions after a
               cancel.
        \param offset The offset from the submission price to restrict
               submissions.
        \param timeClient Initializes the TimeClient.
      */
      template<typename TimeClientForward>
      OpposingOrderSubmissionComplianceRule(
        boost::posix_time::time_duration timeout, Money offset,
        TimeClientForward&& timeClient);

      virtual void Add(const OrderExecutionService::Order& order);

      virtual void Submit(const OrderExecutionService::Order& order);

    private:
      boost::posix_time::time_duration m_timeout;
      Money m_offset;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::TaggedQueue<const OrderExecutionService::Order*,
        OrderExecutionService::ExecutionReport> m_executionReportQueue;
      boost::posix_time::ptime m_lastAskCancelTime;
      Money m_askPrice;
      boost::posix_time::ptime m_lastBidCancelTime;
      Money m_bidPrice;

      Money GetSubmissionPrice(const OrderExecutionService::Order& order);
      bool TestSubmissionPriceInRange(
        const OrderExecutionService::Order& order);
  };

  template<typename TimeClient>
  OpposingOrderSubmissionComplianceRule(boost::posix_time::time_duration, Money,
    TimeClient&& timeClient) -> OpposingOrderSubmissionComplianceRule<
    std::decay_t<TimeClient>>;

  //! Builds a ComplianceRuleSchema representing an
  //! OpposingOrderSubmissionComplianceRule.
  inline ComplianceRuleSchema
      BuildOpposingOrderSubmissionComplianceRuleSchema() {
    std::vector<ComplianceParameter> parameters;
    std::vector<ComplianceValue> symbols;
    symbols.push_back(Security{});
    parameters.emplace_back("symbols", symbols);
    parameters.emplace_back("start_period", boost::posix_time::time_duration{});
    parameters.emplace_back("end_period", boost::posix_time::time_duration{});
    parameters.emplace_back("timeout", Quantity{0});
    parameters.emplace_back("offset", Money::ZERO);
    ComplianceRuleSchema schema{"opposing_order_submission", parameters};
    return schema;
  }

  //! Builds an OpposingOrderSubmissionComplianceRule from a list of
  //! ComplianceParameters.
  /*!
    \param parameters The list of ComplianceParameters used to build the rule.
    \param timeClient Initializes the TimeClient.
  */
  template<typename TimeClient>
  std::unique_ptr<ComplianceRule> MakeOpposingOrderSubmissionComplianceRule(
      const std::vector<ComplianceParameter>& parameters,
      const TimeClient& timeClient) {
    SecuritySet symbols;
    boost::posix_time::time_duration startPeriod;
    boost::posix_time::time_duration endPeriod;
    boost::posix_time::time_duration timeout;
    Money offset;
    for(auto& parameter : parameters) {
      if(parameter.m_name == "symbols") {
        for(auto& security : boost::get<std::vector<ComplianceValue>>(
            parameter.m_value)) {
          symbols.Add(std::move(boost::get<Security>(security)));
        }
      } else if(parameter.m_name == "start_period") {
        startPeriod = boost::get<boost::posix_time::time_duration>(
          parameter.m_value);
      } else if(parameter.m_name == "end_period") {
        endPeriod = boost::get<boost::posix_time::time_duration>(
          parameter.m_value);
      } else if(parameter.m_name == "timeout") {
        timeout = boost::posix_time::seconds(
          static_cast<int>(boost::get<Quantity>(parameter.m_value)));
      } else if(parameter.m_name == "offset") {
        offset = boost::get<Money>(parameter.m_value);
      }
    }
    auto mapRule = MakeMapSecurityComplianceRule({},
      [=] (const ComplianceRuleSchema&) {
        return std::make_unique<OpposingOrderSubmissionComplianceRule<
          TimeClient>>(timeout, offset, timeClient);
      });
    auto timeFilter = std::make_unique<TimeFilterComplianceRule<TimeClient>>(
      startPeriod, endPeriod, timeClient, std::move(mapRule));
    auto symbolFilter = std::make_unique<SecurityFilterComplianceRule>(
      std::move(symbols), std::move(timeFilter));
    return std::move(symbolFilter);
  }

  template<typename TimeClientType>
  template<typename TimeClientForward>
  OpposingOrderSubmissionComplianceRule<TimeClientType>::
      OpposingOrderSubmissionComplianceRule(
      boost::posix_time::time_duration timeout, Money offset,
      TimeClientForward&& timeClient)
      : m_timeout{timeout},
        m_offset{offset},
        m_timeClient{std::forward<TimeClientForward>(timeClient)},
        m_lastAskCancelTime{boost::posix_time::min_date_time},
        m_askPrice{std::numeric_limits<Money>::max()},
        m_lastBidCancelTime{boost::posix_time::min_date_time},
        m_bidPrice{Money::ZERO} {}

  template<typename TimeClientType>
  void OpposingOrderSubmissionComplianceRule<TimeClientType>::Add(
      const OrderExecutionService::Order& order) {
    if(order.GetInfo().m_fields.m_type != OrderType::LIMIT &&
        order.GetInfo().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    order.GetPublisher().Monitor(m_executionReportQueue.GetSlot(&order));
  }

  template<typename TimeClientType>
  void OpposingOrderSubmissionComplianceRule<TimeClientType>::Submit(
      const OrderExecutionService::Order& order) {
    if(order.GetInfo().m_fields.m_type != OrderType::LIMIT &&
        order.GetInfo().m_fields.m_type != OrderType::MARKET) {
      return;
    }
    auto time = m_timeClient->GetTime();
    while(!m_executionReportQueue.IsEmpty()) {
      auto executionReport = m_executionReportQueue.Top();
      m_executionReportQueue.Pop();
      if(executionReport.m_value.m_status == OrderStatus::CANCELED) {
        auto side = executionReport.m_key->GetInfo().m_fields.m_side;
        auto submissionPrice = GetSubmissionPrice(*executionReport.m_key);
        if(side == Side::ASK) {
          if((time - m_lastAskCancelTime) > m_timeout) {
            m_askPrice = std::numeric_limits<Money>::max();
          }
          if(executionReport.m_value.m_timestamp >= m_lastAskCancelTime) {
            if(submissionPrice <= m_askPrice) {
              m_lastAskCancelTime = executionReport.m_value.m_timestamp;
              m_askPrice = submissionPrice;
            }
          }
        } else {
          if((time - m_lastBidCancelTime) > m_timeout) {
            m_bidPrice = Money::ZERO;
          }
          if(executionReport.m_value.m_timestamp >= m_lastBidCancelTime) {
            if(submissionPrice >= m_bidPrice) {
              m_lastBidCancelTime = executionReport.m_value.m_timestamp;
              m_bidPrice = submissionPrice;
            }
          }
        }
      }
    }
    auto& lastCancelTime = Pick(order.GetInfo().m_fields.m_side,
      m_lastBidCancelTime, m_lastAskCancelTime);
    if(TestSubmissionPriceInRange(order) &&
        lastCancelTime >= (time - m_timeout)) {
      BOOST_THROW_EXCEPTION(ComplianceCheckException{
        "Opposing order can not be submitted yet."});
    }
    order.GetPublisher().Monitor(m_executionReportQueue.GetSlot(&order));
  }

  template<typename TimeClientType>
  Money OpposingOrderSubmissionComplianceRule<TimeClientType>::
      GetSubmissionPrice(const OrderExecutionService::Order& order) {
    if(order.GetInfo().m_fields.m_type == OrderType::LIMIT) {
      return order.GetInfo().m_fields.m_price;
    } else if(order.GetInfo().m_fields.m_side == Side::ASK) {
      return Money::ZERO;
    } else {
      return std::numeric_limits<Money>::max();
    }
  }

  template<typename TimeClientType>
  bool OpposingOrderSubmissionComplianceRule<TimeClientType>::
      TestSubmissionPriceInRange(const OrderExecutionService::Order& order) {
    auto price = GetSubmissionPrice(order);
    if(order.GetInfo().m_fields.m_side == Side::ASK) {
      return price <= m_bidPrice + m_offset;
    } else {
      return price >= m_askPrice - m_offset;
    }
  }
}
}

#endif
