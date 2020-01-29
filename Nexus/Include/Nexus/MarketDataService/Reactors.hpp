#ifndef NEXUS_MARKET_DATA_SERVICE_REACTORS_HPP
#define NEXUS_MARKET_DATA_SERVICE_REACTORS_HPP
#include <utility>
#include <Aspen/Aspen.hpp>
#include <Beam/Reactors/QueryReactor.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"

namespace Nexus::MarketDataService {

  /**
   * Builds a reactor that queries BboQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto BboQuoteReactor(MarketDataClient& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<BboQuote>(
      [client = &client] (const auto& query, const auto& queue) {
        client->QueryBboQuotes(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Builds a reactor that queries the current BboQuote.
   * @param client The market data client to query.
   * @param security The security whose current BboQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto CurrentBboQuoteReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return BboQuoteReactor(client, Aspen::lift(
      &Beam::Queries::BuildCurrentQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries for real time BboQuotes.
   * @param client The market data client to query.
   * @param security The security whose real time BboQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto RealTimeBboQuoteReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return BboQuoteReactor(client, Aspen::lift(
      &Beam::Queries::BuildRealTimeQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries BookQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto BookQuoteReactor(MarketDataClient& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<BookQuote>(
      [client = &client] (const auto& query, const auto& queue) {
        client->QueryBookQuotes(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Builds a reactor that queries the current BookQuote.
   * @param client The market data client to query.
   * @param security The security whose current BookQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto CurrentBookQuoteReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return BookQuoteReactor(client, Aspen::lift(
      &Beam::Queries::BuildCurrentQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries for real time BookQuotes.
   * @param client The market data client to query.
   * @param security The security whose real time BookQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto RealTimeBookQuoteReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return BookQuoteReactor(client, Aspen::lift(
      &Beam::Queries::BuildRealTimeQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries MarketQuotes.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto MarketQuoteReactor(MarketDataClient& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<MarketQuote>(
      [client = &client] (const auto& query, const auto& queue) {
        client->QueryMarketQuotes(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Builds a reactor that queries the current MarketQuote.
   * @param client The market data client to query.
   * @param security The security whose current MarketQuotes are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto CurrentMarketQuoteReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return MarketQuoteReactor(client, Aspen::lift(
      &Beam::Queries::BuildCurrentQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries for real time MarketQuotes.
   * @param client The market data client to query.
   * @param security The security whose real time MarketQuotes are to be
   *        queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto RealTimeMarketQuoteReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return MarketQuoteReactor(client, Aspen::lift(
      &Beam::Queries::BuildRealTimeQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries TimeAndSales.
   * @param client The market data client to query.
   * @param query The reactor producing the queries to submit.
   */
  template<typename MarketDataClient, typename QueryReactor>
  auto TimeAndSalesReactor(MarketDataClient& client, QueryReactor&& query) {
    return Beam::Reactors::QueryReactor<TimeAndSale>(
      [client = &client] (const auto& query, const auto& queue) {
        client->QueryTimeAndSales(query, queue);
      },
      std::forward<QueryReactor>(query));
  }

  /**
   * Builds a reactor that queries the current TimeAndSales.
   * @param client The market data client to query.
   * @param security The security whose current TimeAndSales are to be queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto CurrentTimeAndSalesReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return TimeAndSalesReactor(client, Aspen::lift(
      &Beam::Queries::BuildCurrentQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }

  /**
   * Builds a reactor that queries for real time TimeAndSales.
   * @param client The market data client to query.
   * @param security The security whose real time TimeAndSales are to be
   *        queried.
   */
  template<typename MarketDataClient, typename SecurityReactor>
  auto RealTimeTimeAndSalesReactor(MarketDataClient& client,
      SecurityReactor&& security) {
    return TimeAndSalesReactor(client, Aspen::lift(
      &Beam::Queries::BuildRealTimeQuery<Security>,
      std::forward<SecurityReactor>(security)));
  }
}

#endif
