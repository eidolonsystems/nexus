#ifndef NEXUS_SUBMISSION_RESTRICTION_PERIOD_COMPLIANCE_RULE_HPP
#define NEXUS_SUBMISSION_RESTRICTION_PERIOD_COMPLIANCE_RULE_HPP
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Compliance {

  /**
   * Rejects submissions made during a specified time period.
   * @param C The type of TimeClient used to check the time of an order
   *        submission.
   */
  template<typename C>
  class SubmissionRestrictionPeriodComplianceRule : public ComplianceRule {
    public:

      /**
       * The type of TimeClient used to check the time of an order submission.
       */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /**
       * Constructs a SubmissionRestrictionPeriodComplianceRule.
       * @param parameters The list of parameters used by this rule.
       * @param timeClient Initializes the TimeClient used to check order
       *        submissions.
       */
      template<typename CF>
      SubmissionRestrictionPeriodComplianceRule(
        const std::vector<ComplianceParameter>& parameters, CF&& timeClient);

      /**
       * Constructs a SubmissionRestrictionPeriodComplianceRule.
       * @param symbols The set of Securities this rule applies to.
       * @param startPeriod The beginning of the period to restrict submissions.
       * @param endPeriod The end of the period to restrict submissions.
       * @param timeClient Initializes the TimeClient used to check order
       *        submissions.
       */
      template<typename CF>
      SubmissionRestrictionPeriodComplianceRule(SecuritySet symbols,
        boost::posix_time::time_duration startPeriod,
        boost::posix_time::time_duration endPeriod, CF&& timeClient);

      void Submit(const OrderExecutionService::Order& order) override;

    private:
      SecuritySet m_symbols;
      boost::posix_time::time_duration m_startPeriod;
      boost::posix_time::time_duration m_endPeriod;
      Beam::GetOptionalLocalPtr<C> m_timeClient;
  };

  /**
   * Builds a ComplianceRuleSchema representing a
   * SubmissionRestrictionPeriodComplianceRule.
   */
  inline ComplianceRuleSchema
      BuildSubmissionRestrictionPeriodComplianceRuleSchema() {
    auto symbols = std::vector<ComplianceValue>();
    symbols.push_back(Security());
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("symbols", symbols);
    parameters.emplace_back("start_period", boost::posix_time::time_duration());
    parameters.emplace_back("end_period", boost::posix_time::time_duration());
    auto schema = ComplianceRuleSchema("submission_restriction_period",
      parameters);
    return schema;
  }

  template<typename C>
  template<typename CF>
  SubmissionRestrictionPeriodComplianceRule<C>::
      SubmissionRestrictionPeriodComplianceRule(
      const std::vector<ComplianceParameter>& parameters, CF&& timeClient)
      : m_timeClient(std::forward<CF>(timeClient)) {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "symbols") {
        for(auto& security : boost::get<std::vector<ComplianceValue>>(
            parameter.m_value)) {
          m_symbols.Add(std::move(boost::get<Security>(security)));
        }
      } else if(parameter.m_name == "start_period") {
        m_startPeriod = boost::get<boost::posix_time::time_duration>(
          parameter.m_value);
      } else if(parameter.m_name == "end_period") {
        m_endPeriod = boost::get<boost::posix_time::time_duration>(
          parameter.m_value);
      }
    }
  }

  template<typename C>
  template<typename CF>
  SubmissionRestrictionPeriodComplianceRule<C>::
      SubmissionRestrictionPeriodComplianceRule(SecuritySet symbols,
      boost::posix_time::time_duration startPeriod,
      boost::posix_time::time_duration endPeriod, CF&& timeClient)
    : m_symbols(std::move(symbols)),
      m_startPeriod(startPeriod),
      m_endPeriod(endPeriod),
      m_timeClient(std::forward<CF>(timeClient)) {}

  template<typename C>
  void SubmissionRestrictionPeriodComplianceRule<C>::Submit(
      const OrderExecutionService::Order& order) {
    auto& security = order.GetInfo().m_fields.m_security;
    if(!m_symbols.Contains(security)) {
      return;
    }
    auto time = m_timeClient->GetTime();
    if(m_startPeriod > m_endPeriod) {
      if(time.time_of_day() >= m_startPeriod ||
          time.time_of_day() <= m_endPeriod) {
        throw ComplianceCheckException(
          "Submissions not permitted at this time.");
      }
    } else {
      if(time.time_of_day() >= m_startPeriod &&
          time.time_of_day() <= m_endPeriod) {
        throw ComplianceCheckException(
          "Submissions not permitted at this time.");
      }
    }
  }
}

#endif
