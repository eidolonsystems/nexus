#ifndef NEXUS_PUREFEETABLE_HPP
#define NEXUS_PUREFEETABLE_HPP
#include <array>
#include <unordered_set>
#include <Beam/Utilities/YamlConfig.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/FeeHandling/FeeHandling.hpp"
#include "Nexus/FeeHandling/LiquidityFlag.hpp"
#include "Nexus/OrderExecutionService/ExecutionReport.hpp"

namespace Nexus {

  /*! \struct PureFeeTable
      \brief Stores the table of fees used by Pure on TSX.
   */
  struct PureFeeTable {

    /*! \enum PriceClass
        \brief Enumerates the types of price classes.
     */
    enum class PriceClass : int {

      //! Unknown.
      NONE = -1,

      //! Price >= $1.00 and designated.
      DESIGNATED,

      //! Price >= $1.00.
      DEFAULT,

      //! Price >= $0.10 & < $1.00.
      SUB_DOLLAR,

      //! Price < $0.10.
      SUB_DIME
    };

    //! The number of price classes enumerated.
    static const std::size_t PRICE_CLASS_COUNT = 4;

    //! The TSX Venture listed fee table.
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_tsxVentureListedFeeTable;

    //! The TSX listed fee table.
    std::array<std::array<Money, LIQUIDITY_FLAG_COUNT>, PRICE_CLASS_COUNT>
      m_tsxListedFeeTable;

    //! The fee used for odd-lots.
    Money m_oddLot;

    //! The cap on TSX Venture listed sub-dime trades.
    Money m_tsxVentureListedSubDimeCap;

    //! The set of Securities part of Pure's designated program.
    std::unordered_set<Security> m_designatedSecurities;
  };

  //! Parses a PureFeeTable from a YAML configuration.
  /*!
    \param config The configuration to parse the PureFeeTable from.
    \return The PureFeeTable represented by the <i>config</i>.
  */
  inline PureFeeTable ParsePureFeeTable(const YAML::Node& config,
      const MarketDatabase& marketDatabase) {
    PureFeeTable feeTable;
    ParseFeeTable(config, "tsx_venture_listed_fee_table",
      Beam::Store(feeTable.m_tsxVentureListedFeeTable));
    ParseFeeTable(config, "tsx_listed_fee_table",
      Beam::Store(feeTable.m_tsxListedFeeTable));
    feeTable.m_oddLot = Beam::Extract<Money>(config, "odd_lot");
    feeTable.m_tsxVentureListedSubDimeCap = Beam::Extract<Money>(config,
      "tsx_venture_sub_dime_cap");
    auto designatedSecuritiesPath = Beam::Extract<std::string>(config,
      "designated_securities_path");
    YAML::Node designatedSecuritiesConfig;
    Beam::LoadFile(designatedSecuritiesPath,
      Beam::Store(designatedSecuritiesConfig));
    auto symbols = designatedSecuritiesConfig.FindValue("symbols");
    if(symbols == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error{
        "PURE designated symbols not found."});
    }
    for(auto& symbol : *symbols) {
      auto security = ParseSecurity(Beam::Extract<std::string>(symbol),
        marketDatabase);
      feeTable.m_designatedSecurities.insert(security);
    }
    return feeTable;
  }

  //! Looks up a fee on a TSX listed Security.
  /*!
    \param feeTable The PureFeeTable used to lookup the fee.
    \param liquidityFlag The trade's LiquidityFlag.
    \param priceClass The trade's PriceClass.
    \return The fee corresponding to the specified <i>liquidityFlag</i> and
            <i>priceClass</i>.
  */
  inline Money LookupTsxListedFee(const PureFeeTable& feeTable,
      LiquidityFlag liquidityFlag, PureFeeTable::PriceClass priceClass) {
    return feeTable.m_tsxListedFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  //! Looks up a fee on a TSX Venture listed Security.
  /*!
    \param feeTable The PureFeeTable used to lookup the fee.
    \param liquidityFlag The trade's LiquidityFlag.
    \param priceClass The trade's PriceClass.
    \return The fee corresponding to the specified <i>liquidityFlag</i> and
            <i>priceClass</i>.
  */
  inline Money LookupTsxVentureListedFee(const PureFeeTable& feeTable,
      LiquidityFlag liquidityFlag, PureFeeTable::PriceClass priceClass) {
    return feeTable.m_tsxVentureListedFeeTable[static_cast<int>(priceClass)][
      static_cast<int>(liquidityFlag)];
  }

  //! Calculates the fee on a trade executed on PURE.
  /*!
    \param feeTable The PureFeeTable used to calculate the fee.
    \param security The Security that was traded.
    \param executionReport The ExecutionReport to calculate the fee for.
    \return The fee calculated for the specified trade.
  */
  inline Money CalculateFee(const PureFeeTable& feeTable,
      const Security& security,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_lastQuantity == 0) {
      return Money::ZERO;
    } else if(executionReport.m_lastQuantity < 100) {
      return executionReport.m_lastQuantity * feeTable.m_oddLot;
    }
    auto priceClass = [&] {
      if(executionReport.m_lastPrice < 10 * Money::CENT) {
        return PureFeeTable::PriceClass::SUB_DIME;
      } else if(executionReport.m_lastPrice < Money::ONE) {
        return PureFeeTable::PriceClass::SUB_DOLLAR;
      } else if(feeTable.m_designatedSecurities.find(security) !=
          feeTable.m_designatedSecurities.end()) {
        return PureFeeTable::PriceClass::DESIGNATED;
      } else {
        return PureFeeTable::PriceClass::DEFAULT;
      }
    }();
    auto liquidityFlag = [&] {
      if(executionReport.m_liquidityFlag.size() == 1) {
        if(executionReport.m_liquidityFlag[0] == 'P') {
          return LiquidityFlag::PASSIVE;
        } else if(executionReport.m_liquidityFlag[0] == 'A') {
          return LiquidityFlag::ACTIVE;
        } else {
          std::cout << "Unknown liquidity flag [PURE]: \"" <<
            executionReport.m_liquidityFlag << "\"\n";
          return LiquidityFlag::ACTIVE;
        }
      } else {
        std::cout << "Unknown liquidity flag [PURE]: \"" <<
          executionReport.m_liquidityFlag << "\"\n";
        return LiquidityFlag::ACTIVE;
      }
    }();
    auto fee = [&] {
      if(security.GetMarket() == DefaultMarkets::TSX()) {
        return LookupTsxListedFee(feeTable, liquidityFlag, priceClass);
      } else if(security.GetMarket() == DefaultMarkets::TSXV()) {
        return LookupTsxVentureListedFee(feeTable, liquidityFlag, priceClass);
      } else {
        std::cout << "Unknown market [PURE]: \"" << security.GetMarket() <<
          "\"\n";
        return LookupTsxVentureListedFee(feeTable, liquidityFlag, priceClass);
      }
    }();
    if(priceClass == PureFeeTable::PriceClass::SUB_DIME &&
        security.GetMarket() == DefaultMarkets::TSXV()) {
      if(fee >= Money::ZERO) {
        return std::min(executionReport.m_lastQuantity * fee,
          feeTable.m_tsxVentureListedSubDimeCap);
      } else {
        return std::max(executionReport.m_lastQuantity * fee,
          -feeTable.m_tsxVentureListedSubDimeCap);
      }
    }
    return executionReport.m_lastQuantity * fee;
  }
}

#endif
