#ifndef NEXUS_BACKTESTERTIMER_HPP
#define NEXUS_BACKTESTERTIMER_HPP
#include <Beam/Queues/MultiQueueWriter.hpp>
#include <Beam/Threading/Timer.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Backtester/Backtester.hpp"
#include "Nexus/Backtester/BacktesterEvent.hpp"
#include "Nexus/Backtester/BacktesterEventHandler.hpp"

namespace Nexus {

  /*! \class BacktesterTimer
      \brief Implements a Timer used by the backtester.
   */
  class BacktesterTimer : private boost::noncopyable {
    public:

      //! Constructs a BacktesterTimer.
      /*!
        \param interval The time interval before expiring.
        \param eventHandler The event handler to publish timer events to.
      */
      BacktesterTimer(boost::posix_time::time_duration interval,
        Beam::RefType<BacktesterEventHandler> eventHandler);

      ~BacktesterTimer();

      void Start();

      void Cancel();

      void Wait();

      const Beam::Publisher<Beam::Threading::Timer::Result>&
        GetPublisher() const;

    private:
      friend class TimerBacktesterEvent;
      boost::posix_time::time_duration m_interval;
      BacktesterEventHandler* m_eventHandler;
      std::shared_ptr<TimerBacktesterEvent> m_expireEvent;
      std::shared_ptr<TimerBacktesterEvent> m_cancelEvent;
      Beam::MultiQueueWriter<Beam::Threading::Timer::Result> m_publisher;
  };

  /*! \class TimerBacktesterEvent
      \brief Represents a Timer event.
   */
  class TimerBacktesterEvent : public BacktesterEvent {
    public:

      //! Constructs a TimerBacktesterEvent.
      /*!
        \param timer The BacktesterTimer that produced the event.
        \param timestamp The time this event is to be executed.
        \param result The Timer result.
      */
      TimerBacktesterEvent(BacktesterTimer& timer,
        boost::posix_time::ptime timestamp,
        Beam::Threading::Timer::Result result);

      void Cancel();

      virtual void Execute() override;

    private:
      BacktesterTimer* m_timer;
      Beam::Threading::Timer::Result m_result;
  };

  inline BacktesterTimer::BacktesterTimer(
      boost::posix_time::time_duration interval,
      Beam::RefType<BacktesterEventHandler> eventHandler)
      : m_interval{interval},
        m_eventHandler{eventHandler.Get()} {}

  inline BacktesterTimer::~BacktesterTimer() {
    Cancel();
  }

  inline void BacktesterTimer::Start() {
    if(m_expireEvent != nullptr) {
      return;
    }
    m_expireEvent = std::make_shared<TimerBacktesterEvent>(*this,
      m_eventHandler->GetTime() + m_interval,
      Beam::Threading::Timer::Result::EXPIRED);
    m_eventHandler->Add(m_expireEvent);
  }

  inline void BacktesterTimer::Cancel() {
    if(m_expireEvent == nullptr) {
      return;
    } else if(m_cancelEvent == nullptr) {
      m_expireEvent->Cancel();
      m_cancelEvent = std::make_shared<TimerBacktesterEvent>(*this,
        boost::posix_time::neg_infin,
        Beam::Threading::Timer::Result::CANCELED);
      m_eventHandler->Add(m_cancelEvent);
    }
    auto cancelEvent = m_cancelEvent;
    cancelEvent->Wait();
  }

  inline void BacktesterTimer::Wait() {
    if(m_expireEvent == nullptr) {
      return;
    }
    auto event =
      [&] {
        if(m_cancelEvent != nullptr &&
            m_cancelEvent->GetTimestamp() < m_expireEvent->GetTimestamp()) {
          return m_cancelEvent;
        } else {
          return m_expireEvent;
        }
      }();
    event->Wait();
  }

  inline const Beam::Publisher<Beam::Threading::Timer::Result>&
      BacktesterTimer::GetPublisher() const {
    return m_publisher;
  }

  inline TimerBacktesterEvent::TimerBacktesterEvent(BacktesterTimer& timer,
      boost::posix_time::ptime timestamp,
      Beam::Threading::Timer::Result result)
      : BacktesterEvent{timestamp},
        m_timer{&timer},
        m_result{result} {}

  inline void TimerBacktesterEvent::Cancel() {
    m_result = Beam::Threading::Timer::Result::NONE;
  }

  inline void TimerBacktesterEvent::Execute() {
    if(m_result != Beam::Threading::Timer::Result::NONE) {
      m_timer->m_expireEvent.reset();
      m_timer->m_cancelEvent.reset();
      m_timer->m_publisher.Push(m_result);
    }
    Complete();
  }
}

namespace Beam {
  template<>
  struct ImplementsConcept<Nexus::BacktesterTimer, Threading::Timer> :
    std::true_type {};
}

#endif
