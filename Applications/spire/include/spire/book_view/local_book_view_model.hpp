#ifndef SPIRE_LOCAL_BOOK_VIEW_MODEL_HPP
#define SPIRE_LOCAL_BOOK_VIEW_MODEL_HPP
#include <unordered_map>
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/book_view_model.hpp"
#include "spire/spire/definitions.hpp"

namespace Spire {

  //! Implements a programmatic BookViewModel in memory.
  class LocalBookViewModel final : public BookViewModel {
    public:

      //! Constructs an empty model.
      /*!
        \param security The security to model.
        \param definitions The set of definitions to use.
      */
      LocalBookViewModel(Nexus::Security security, Definitions definitions);

      //! Updates the BBO.
      /*!
        \param bbo The updated BBO.
      */
      void update(const Nexus::BboQuote& bbo);

      //! Updates a book quote.
      /*!
        \param quote The book quote to update.
      */
      void update(const Nexus::BookQuote& quote);

      //! Updates a market quote.
      /*!
        \param quote The market quote to update.
      */
      void update(const Nexus::MarketQuote& quote);

      //! Updates the session's volume.
      /*!
        \param volume The session's volume.
      */
      void update_volume(Nexus::Quantity volume);

      //! Updates the high of the session.
      /*!
        \param high The updated high of the session.
      */
      void update_high(Nexus::Money high);

      //! Updates the low of the session.
      /*!
        \param low The updated low of the session.
      */
      void update_low(Nexus::Money low);

      //! Updates the session's opening price.
      /*!
        \param open The session's opening price.
      */
      void update_open(Nexus::Money open);

      //! Updates the session's closing price.
      /*!
        \param close The session's closing price.
      */
      void update_close(Nexus::Money close);

      const Nexus::Security& get_security() const override;

      const Nexus::BboQuote& get_bbo() const override;

      const std::vector<Nexus::BookQuote>& get_asks() const override;

      const std::vector<Nexus::BookQuote>& get_bids() const override;

      boost::optional<Nexus::Money> get_high() const override;

      boost::optional<Nexus::Money> get_low() const override;

      boost::optional<Nexus::Money> get_open() const override;

      boost::optional<Nexus::Money> get_close() const override;

      Nexus::Quantity get_volume() const override;

      QtPromise<void> load() override;

      boost::signals2::connection connect_bbo_slot(
        const BboSignal::slot_type& slot) const override;

      boost::signals2::connection connect_book_quote_slot(
        const BookQuoteSignal::slot_type& slot) const override;

      boost::signals2::connection connect_high_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_low_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_open_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_close_slot(
        const PriceSignal::slot_type& slot) const override;

      boost::signals2::connection connect_volume_slot(
        const QuantitySignal::slot_type& slot) const override;

    private:
      mutable BboSignal m_bbo_signal;
      mutable BookQuoteSignal m_book_quote_signal;
      mutable PriceSignal m_high_signal;
      mutable PriceSignal m_low_signal;
      mutable PriceSignal m_open_signal;
      mutable PriceSignal m_close_signal;
      mutable QuantitySignal m_volume_signal;
      Nexus::Security m_security;
      Definitions m_definitions;
      Nexus::BboQuote m_bbo;
      std::unordered_map<Nexus::MarketCode, Nexus::MarketQuote> m_market_quotes;
      std::vector<Nexus::BookQuote> m_asks;
      std::vector<Nexus::BookQuote> m_bids;
      boost::optional<Nexus::Money> m_high;
      boost::optional<Nexus::Money> m_low;
      boost::optional<Nexus::Money> m_open;
      boost::optional<Nexus::Money> m_close;
      Nexus::Quantity m_volume;
  };
}

#endif
