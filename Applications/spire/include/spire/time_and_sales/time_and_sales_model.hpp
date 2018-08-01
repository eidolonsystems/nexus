#ifndef SPIRE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TIME_AND_SALES_MODEL_HPP
#include <boost/noncopyable.hpp>
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "spire/spire/qt_promise.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"

namespace spire {

  //! Models the time and sales for a single security.
  class time_and_sales_model : private boost::noncopyable {
    public:

      //! Stores a single time and sale entry.
      struct entry {

        //! The time and sale.
        Nexus::SequencedTimeAndSale m_time_and_sale;

        //! The price range the entry belongs to.
        time_and_sales_properties::price_range m_price_range;
      };

      //! Signals a new time and sale.
      /*!
        \param e The new time and sale.
      */
      using time_and_sale_signal = Signal<void (const entry& e)>;

      //! Signals a change in the volume.
      /*!
        \param v The updated volume.
      */
      using volume_signal = Signal<void (Nexus::Quantity v)>;

      virtual ~time_and_sales_model() = default;

      //! Returns the security being modeled.
      virtual const Nexus::Security& get_security() const = 0;

      //! Returns the current volume.
      virtual Nexus::Quantity get_volume() const = 0;

      //! Loads a snapshot of values to display.
      /*!
        \param last The sequence number of the last entry to load,
               (use Beam::Queries::Sequence::Last() to load the most recent
               value).
        \param count The number of values to load.
        \return A list of time and sales no greater than <i>count</i> items
                where the last item's sequence number is no greater than
                <i>last</i>.
      */
      virtual QtPromise<std::vector<entry>> load_snapshot(
        Beam::Queries::Sequence last, int count) = 0;

      //! Connects a slot to the time and sale signal.
      virtual boost::signals2::connection connect_time_and_sale_signal(
        const time_and_sale_signal::slot_type& slot) const = 0;

      //! Connects a slot to the volume signal.
      virtual boost::signals2::connection connect_volume_signal(
        const volume_signal::slot_type& slot) const = 0;

    protected:

      //! Constructs a time and sales model.
      time_and_sales_model() = default;
  };
}

#endif
