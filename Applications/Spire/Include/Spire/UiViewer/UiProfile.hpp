#ifndef SPIRE_UI_PROFILE_HPP
#define SPIRE_UI_PROFILE_HPP
#include <any>
#include <functional>
#include <memory>
#include <vector>
#include <QWidget>
#include "Spire/UiViewer/UiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Stores the information needed to display a single widget in the UI Viewer.
  class UiProfile {
    public:

      //! Provides a generic signal to indicate a change in the widget.
      /*!
        \param name The name of the event.
        \param arguments The arguments passed to the event.
      */
      using EventSignal = Signal<
        void (const QString& name, const std::vector<std::any>& arguments)>;

      //! Constructs the profile for a widget.
      /*!
        \param name The name of the widget.
        \param properties The widget's properties.
        \param factory The function used to construct the widget.
      */
      UiProfile(QString name,
        std::vector<std::shared_ptr<UiProperty>> properties,
        std::function<QWidget* (UiProfile&)>);

      UiProfile(UiProfile&&) = default;

      //! Returns the name of the widget.
      const QString& get_name() const;

      //! Returns the widget's properties.
      const std::vector<std::shared_ptr<UiProperty>>& get_properties() const;

      //! Returns the widget to display.
      QWidget* get_widget();

      //! Returns a slot that is used to report updates to the displayed widget.
      template<typename... Args>
      std::function<void (const Args&...)> make_event_slot(const QString& name);

      //! Resets this profile.
      void reset();

      //! Connects a slot to the EventSignal.
      boost::signals2::connection connect_event_signal(
        const EventSignal::slot_type& slot) const;

      UiProfile& operator =(UiProfile&&) = default;

    private:
      mutable std::shared_ptr<EventSignal> m_event_signal;
      QString m_name;
      std::vector<std::shared_ptr<UiProperty>> m_properties;
      std::function<QWidget* (UiProfile&)> m_factory;
      QWidget* m_widget;

      UiProfile(const UiProfile&) = delete;
      UiProfile& operator =(const UiProfile&) = delete;
  };

  template<typename... Args>
  std::function<void (const Args&...)> UiProfile::make_event_slot(
      const QString& name) {
    return [m_event_signal = m_event_signal, name] (const auto&... args) {
      auto values = std::vector<std::any>();
      (values.push_back(args), ...);
      (*m_event_signal)(name, values);
    };
  }
}

#endif
