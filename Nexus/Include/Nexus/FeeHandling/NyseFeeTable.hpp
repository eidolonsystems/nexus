#ifndef NEXUS_NYSEFEETABLE_HPP
#define NEXUS_NYSEFEETABLE_HPP
#include <array>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/rational.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

namespace Nexus {

  /*! \struct NyseFeeTable
      \brief Stores the table of fees used by NYSE.
   */
  struct NyseFeeTable {

    /*! \enum Type
        \brief Enumerates the types of trades.
     */
    enum class Type : int {

      //! Unknown.
      NONE = -1,

      //! Active.
      ACTIVE = 0,

      //! Passive.
      PASSIVE
    };

    //! The number of trade types enumerated.
    static const std::size_t TYPE_COUNT = 2;

    /*! \enum Category
        \brief Enumerates the category of trades.
     */
    enum class Category : int {

      //! Unknown.
      NONE = -1,

      //! Default category.
      DEFAULT = 0,

      //! Hidden order.
      HIDDEN,

      //! Cross order.
      CROSS,

      //! On open.
      ON_OPEN,

      //! On close.
      ON_CLOSE,

      //! Retail order.
      RETAIL
    };

    //! The number of trade categories enumerated.
    static const std::size_t CATEGORY_COUNT = 6;

    //! The fee table.
    std::array<std::array<Money, TYPE_COUNT>, CATEGORY_COUNT> m_feeTable;

    //! The sub-dollar rates.
    std::array<boost::rational<int>, TYPE_COUNT> m_subDollarTable;
  };

  //! Parses a NyseFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the NyseFeeTable from.
    \return The NyseFeeTable represented by the <i>config</i>.
  */
  inline NyseFeeTable ParseNyseFeeTable(const YAML::Node& config) {
    NyseFeeTable feeTable;
    ParseFeeTable(config, "fee_table", Beam::Store(feeTable.m_feeTable));
    ParseFeeTable(config, "sub_dollar_table",
      Beam::Store(feeTable.m_subDollarTable));
    return feeTable;
  }

  //! Looks up a fee.
  /*!
    \param feeTable The NyseFeeTable used to lookup the fee.
    \param type The trade's type.
    \param category The trade's Category.
    \return The fee corresponding to the specified <i>type</i> and
            <i>category</i>.
  */
  inline Money LookupFee(const NyseFeeTable& feeTable, NyseFeeTable::Type type,
      NyseFeeTable::Category category) {
    return feeTable.m_feeTable[static_cast<int>(category)][
      static_cast<int>(type)];
  }

  //! Tests if an OrderFields represents a hidden liquidity provider.
  /*!
    \param fields The OrderFields to test.
    \return <code>true</code> iff the <i>order</i> counts as a hidden liquidity
            provider.
  */
  inline bool IsNyseHiddenLiquidityProvider(
      const OrderExecutionService::OrderFields& fields) {
    return fields.m_type == OrderType::PEGGED &&
      OrderExecutionService::HasField(fields, Tag{18, "M"});
  }

  //! Calculates the fee on a trade executed on NYSE.
  /*!
    \param feeTable The NyseFeeTable used to calculate the fee.
    \param fields The OrderFields used to place the Order.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const NyseFeeTable& feeTable,
      const OrderExecutionService::OrderFields& fields,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    }
    auto isHidden = IsNyseHiddenLiquidityProvider(fields);
    NyseFeeTable::Type type = NyseFeeTable::Type::ACTIVE;
    NyseFeeTable::Category category = NyseFeeTable::Category::DEFAULT;
    if(executionReport.m_liquidityFlag.size() == 1) {
      if(executionReport.m_liquidityFlag[0] == '1') {
        type = NyseFeeTable::Type::ACTIVE;
        if(isHidden) {
          category = NyseFeeTable::Category::HIDDEN;
        }
      } else if(executionReport.m_liquidityFlag[0] == '2') {
        type = NyseFeeTable::Type::PASSIVE;
        if(isHidden) {
          category = NyseFeeTable::Category::HIDDEN;
        }
      } else if(executionReport.m_liquidityFlag[0] == '3') {
        type = NyseFeeTable::Type::ACTIVE;
      } else if(executionReport.m_liquidityFlag[0] == '5') {
        type = NyseFeeTable::Type::PASSIVE;
        category = NyseFeeTable::Category::ON_OPEN;
      } else if(executionReport.m_liquidityFlag[0] == '6') {
        type = NyseFeeTable::Type::ACTIVE;
        category = NyseFeeTable::Category::ON_CLOSE;
      } else if(executionReport.m_liquidityFlag[0] == '7') {
        type = NyseFeeTable::Type::ACTIVE;
        category = NyseFeeTable::Category::ON_CLOSE;
      } else if(executionReport.m_liquidityFlag[0] == '8') {
        type = NyseFeeTable::Type::PASSIVE;
        category = NyseFeeTable::Category::RETAIL;
      } else if(executionReport.m_liquidityFlag[0] == '9') {
        type = NyseFeeTable::Type::ACTIVE;
        category = NyseFeeTable::Category::RETAIL;
      } else {
        std::cout << "Unknown liquidity flag [NYSE]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
      }
    } else {
      std::cout << "Unknown liquidity flag [NYSE]: \"" <<
        executionReport.m_liquidityFlag << "\"\n";
    }
    if(executionReport.m_lastPrice < Money::ONE) {
      auto rate = feeTable.m_subDollarTable[static_cast<int>(type)];
      return rate *
        (executionReport.m_lastQuantity * executionReport.m_lastPrice);
    }
    auto fee = LookupFee(feeTable, type, category);
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
