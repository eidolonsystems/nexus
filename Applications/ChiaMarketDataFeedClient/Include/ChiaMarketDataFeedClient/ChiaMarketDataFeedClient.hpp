#ifndef NEXUS_CHIAMARKETDATAFEEDCLIENT_HPP
#define NEXUS_CHIAMARKETDATAFEEDCLIENT_HPP
#include <cstdint>
#include <string>
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "ChiaMarketDataFeedClient/ChiaConfiguration.hpp"
#include "ChiaMarketDataFeedClient/ChiaMessage.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolClient.hpp"
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolMessage.hpp"
#include "Nexus/MarketDataService/MarketDataFeedClient.hpp"
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \class ChiaMarketDataFeedClient
      \brief Parses packets from the CHIA market data feed.
      \tparam MarketDataFeedClientType The type of MarketDataFeedClient used to
              update the MarketDataServer.
      \tparam ProtocolClientType The type of client receiving CHIA messages.
   */
  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  class ChiaMarketDataFeedClient : private boost::noncopyable {
    public:

      //! The type of MarketDataFeedClient used to update the MarketDataServer.
      using MarketDataFeedClient =
        Beam::GetTryDereferenceType<MarketDataFeedClientType>;

      //! The type of client receiving CHIA messages.
      using ProtocolClient = Beam::GetTryDereferenceType<ProtocolClientType>;

      //! Constructs a ChiaMarketDataFeedClient.
      /*!
        \param config The configuration to use.
        \param marketDataFeedClient Initializes the MarketDataFeedClient.
        \param protocolClient The client receiving CHIA messages.
      */
      template<typename MarketDataFeedClientForward,
        typename ProtocolClientForward>
      ChiaMarketDataFeedClient(const ChiaConfiguration& config,
        MarketDataFeedClientForward&& marketDataFeedClient,
        ProtocolClientForward&& itchClient);

      ~ChiaMarketDataFeedClient();

      void Open();

      void Close();

    private:
      struct OrderEntry {
        Security m_security;
        Money m_price;

        OrderEntry() = default;
        OrderEntry(Security security, Money price);
      };
      ChiaConfiguration m_config;
      Beam::GetOptionalLocalPtr<MarketDataFeedClientType>
        m_marketDataFeedClient;
      Beam::GetOptionalLocalPtr<ProtocolClientType> m_protocolClient;
      std::unordered_map<std::string, OrderEntry> m_orderEntries;
      Beam::Routines::RoutineHandler m_readLoopRoutine;
      Beam::IO::OpenState m_openState;

      void Shutdown();
      char ParseChar(Beam::Out<const char*> cursor);
      std::int64_t ParseNumeric(int length, Beam::Out<const char*> cursor);
      std::string ParseAlphanumeric(int length,
        Beam::Out<const char*> cursor);
      Side ParseSide(Beam::Out<const char*> cursor);
      Money ParsePrice(bool isLongForm, Beam::Out<const char*> cursor);
      boost::posix_time::ptime ParseTimestamp(
        boost::posix_time::time_duration timestamp);
      void HandleAddOrderMessage(bool isLongForm, const ChiaMessage& message);
      void HandleOrderExecutionMessage(bool isLongForm,
        const ChiaMessage& message);
      void HandleOrderCancelMessage(bool isLongForm,
        const ChiaMessage& message);
      void HandleTradeMessage(bool isLongForm, const ChiaMessage& message);
      void Dispatch(const ChiaMessage& message);
      void ReadLoop();
  };

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      OrderEntry::OrderEntry(Security security, Money price)
      : m_security(std::move(security)),
        m_price(price) {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  template<typename MarketDataFeedClientForward, typename ProtocolClientForward>
  ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ChiaMarketDataFeedClient(const ChiaConfiguration& config,
      MarketDataFeedClientForward&& marketDataFeedClient,
      ProtocolClientForward&& protocolClient)
      : m_config(config),
        m_marketDataFeedClient{std::forward<MarketDataFeedClientForward>(
          marketDataFeedClient)},
        m_protocolClient{std::forward<ProtocolClientForward>(protocolClient)} {}

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ~ChiaMarketDataFeedClient() {
    Close();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_marketDataFeedClient->Open();
      m_protocolClient->Open();
      m_readLoopRoutine = Beam::Routines::Spawn(
        std::bind(&ChiaMarketDataFeedClient::ReadLoop, this));
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Shutdown() {
    m_protocolClient->Close();
    m_marketDataFeedClient->Close();
    m_readLoopRoutine.Wait();
    m_openState.SetClosed();
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  char ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseChar(Beam::Out<const char*> cursor) {
    auto value = **cursor;
    ++*cursor;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  std::int64_t ChiaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseNumeric(int length,
      Beam::Out<const char*> cursor) {
    while(**cursor == ' ' && length > 0) {
      ++*cursor;
      --length;
    }
    std::int64_t value = 0;
    while(length > 0) {
      value = value * 10 + (**cursor - '0');
      --length;
      ++*cursor;
    }
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  std::string ChiaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseAlphanumeric(int length,
      Beam::Out<const char*> cursor) {
    std::string value;
    for(auto i = 0; i < length; ++i) {
      if((*cursor)[i] == ' ') {
        break;
      }
      value += (*cursor)[i];
    }
    *cursor += length;
    return value;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Side ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParseSide(Beam::Out<const char*> cursor) {
    auto value = ParseChar(Beam::Store(cursor));
    auto side =
      [&] {
        if(value == 'B') {
          return Side::BID;
        } else if(value == 'S') {
          return Side::ASK;
        }
        return Side::NONE;
      }();
    return side;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  Money ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ParsePrice(bool isLongForm, Beam::Out<const char*> cursor) {
    auto PowerOfTen =
      [] (std::uint16_t exponent) {
        std::uint64_t result = 1;
        for(auto i = std::uint16_t{0}; i < exponent; ++i) {
          result *= 10;
        }
        return result;
      };
    auto value = 0;
    auto length =
      [&] {
        if(isLongForm) {
          return 19;
        }
        return 10;
      }();
    auto remainingLength = length;
    while(**cursor == ' ' && remainingLength > 0) {
      ++*cursor;
      --remainingLength;
    }
    while(remainingLength > 0) {
      value = 10 * value + (**cursor - '0');
      ++*cursor;
      --remainingLength;
    }
    auto decimalPlaces =
      [&] {
        if(isLongForm) {
          return 7;
        }
        return 4;
      }();
    auto multiplier = PowerOfTen(Money::DECIMAL_PLACES - decimalPlaces);
    auto price = Money::FromRepresentation(value * multiplier);
    return price;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  boost::posix_time::ptime ChiaMarketDataFeedClient<MarketDataFeedClientType,
      ProtocolClientType>::ParseTimestamp(
      boost::posix_time::time_duration timestamp) {
    return m_config.m_timeOrigin + timestamp;
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleAddOrderMessage(bool isLongForm, const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto side = ParseSide(Beam::Store(cursor));
    auto shares =
      [&] {
        if(isLongForm) {
          return ParseNumeric(10, Beam::Store(cursor));
        } else {
          return ParseNumeric(6, Beam::Store(cursor));
        }
      }();
    auto symbol = ParseAlphanumeric(6, Beam::Store(cursor));
    auto price = ParsePrice(isLongForm, Beam::Store(cursor));
    auto display = ParseChar(Beam::Store(cursor));
    if(display != 'Y') {
      return;
    }
    auto timestamp = ParseTimestamp(message.m_timestamp);
    Security security(symbol, m_config.m_primaryMarket, m_config.m_country);
    m_marketDataFeedClient->AddOrder(security, m_config.m_disseminatingMarket,
      m_config.m_mpid, false, orderReference, side, price, shares, timestamp);
    if(m_config.m_isTimeAndSaleFeed) {
      m_orderEntries[orderReference] = OrderEntry(security, price);
    }
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "," << ToString(side) << "," << shares << "," <<
        symbol << "," << price.ToString() << "\n";
      std::cout << std::flush;
    }
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleOrderExecutionMessage(bool isLongForm, const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto shares =
      [&] {
        if(isLongForm) {
          return ParseNumeric(10, Beam::Store(cursor));
        } else {
          return ParseNumeric(6, Beam::Store(cursor));
        }
      }();
    auto tradeReference = ParseNumeric(9, Beam::Store(cursor));
    auto contraOrderReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto timestamp = ParseTimestamp(message.m_timestamp);
    m_marketDataFeedClient->OffsetOrderSize(orderReference, -shares, timestamp);
    if(m_config.m_isTimeAndSaleFeed) {
      auto orderEntry = Beam::Lookup(m_orderEntries, orderReference);
      if(!orderEntry.is_initialized()) {
        return;
      }
      TimeAndSale::Condition condition;
      condition.m_code = "@";
      TimeAndSale timeAndSale(timestamp, orderEntry->m_price, shares,
        std::move(condition), m_config.m_mpid);
      m_marketDataFeedClient->PublishTimeAndSale(
        SecurityTimeAndSale(std::move(timeAndSale), orderEntry->m_security));
    }
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "," << shares << "," << tradeReference << "," <<
        contraOrderReference << "\n";
      std::cout << std::flush;
    }
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleOrderCancelMessage(bool isLongForm, const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto timestamp = ParseTimestamp(message.m_timestamp);
    m_marketDataFeedClient->DeleteOrder(orderReference, timestamp);
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "\n";
      std::cout << std::flush;
    }
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      HandleTradeMessage(bool isLongForm, const ChiaMessage& message) {
    auto cursor = message.m_data;
    auto orderReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto sideIndicator = ParseChar(Beam::Store(cursor));
    auto shares =
      [&] {
        if(isLongForm) {
          return ParseNumeric(10, Beam::Store(cursor));
        } else {
          return ParseNumeric(6, Beam::Store(cursor));
        }
      }();
    auto symbol = ParseAlphanumeric(6, Beam::Store(cursor));
    auto price = ParsePrice(isLongForm, Beam::Store(cursor));
    auto tradeReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto contraOrderReference = boost::lexical_cast<std::string>(
      ParseNumeric(9, Beam::Store(cursor)));
    auto tradeType = ParseChar(Beam::Store(cursor));
    auto timestamp = ParseTimestamp(message.m_timestamp);
    Security security(symbol, m_config.m_primaryMarket, m_config.m_country);
    TimeAndSale::Condition condition;
    condition.m_code = "@";
    TimeAndSale timeAndSale(timestamp, price, shares, std::move(condition),
      m_config.m_mpid);
    m_marketDataFeedClient->PublishTimeAndSale(
      SecurityTimeAndSale(std::move(timeAndSale), security));
    if(m_config.m_isLoggingMessages) {
      std::cout << timestamp << "," << message.m_type << "," <<
        orderReference << "," << shares << "," << symbol << "," <<
        tradeReference << "," << contraOrderReference << "\n";
      std::cout << std::flush;
    }
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      Dispatch(const ChiaMessage& message) {
    if(message.m_type == 'A') {
      HandleAddOrderMessage(false, message);
    } else if(message.m_type == 'a') {
      HandleAddOrderMessage(true, message);
    } else if(message.m_type == 'E') {
      HandleOrderExecutionMessage(false, message);
    } else if(message.m_type == 'e') {
      HandleOrderExecutionMessage(true, message);
    } else if(message.m_type == 'X') {
      HandleOrderCancelMessage(false, message);
    } else if(message.m_type == 'x') {
      HandleOrderCancelMessage(true, message);
    } else if(message.m_type == 'P' || message.m_type == 'M') {
      HandleTradeMessage(false, message);
    } else if(message.m_type == 'p' || message.m_type == 'm') {
      HandleTradeMessage(true, message);
    }
  }

  template<typename MarketDataFeedClientType, typename ProtocolClientType>
  void ChiaMarketDataFeedClient<MarketDataFeedClientType, ProtocolClientType>::
      ReadLoop() {
    while(true) {
      try {
        auto message = m_protocolClient->Read();
        Dispatch(message);
      } catch(const Beam::IO::EndOfFileException&) {
        break;
      }
    }
  }
}
}

#endif
