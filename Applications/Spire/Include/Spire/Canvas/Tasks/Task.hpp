#ifndef SPIRE_TASK_HPP
#define SPIRE_TASK_HPP
#include <ostream>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Tasks/Executor.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
namespace Details {
  BEAM_ENUM(TaskState,

    //! The Task is initializing, no sub-Tasks may be executed.
    INITIALIZING,

    //! Task is active, sub-Tasks may be executed.
    ACTIVE,

    //! Task is pending cancel, no new sub-Tasks may be executed.
    PENDING_CANCEL,

    //! Task is canceled.
    CANCELED,

    //! Task encountered an error.
    FAILED,

    //! The Task has expired.
    EXPIRED,

    //! Task has completed.
    COMPLETE);
}

  /** Executes and manages an asynchronous operation and sub-Tasks. */
  class Task {
    public:

      /** Enumerates Task states. */
      using State = Details::TaskState;

      /** Records a change in a Task's State. */
      struct StateEntry {

        /** The State of the Task. */
        State m_state;

        /** A message describing the change in State. */
        std::string m_message;

        /** Constructs an uninitialized StateEntry. */
        StateEntry() = default;

        /**
         * Constructs a StateEntry with an empty message.
         * @param state The Task's State.
         */
        explicit StateEntry(State state);

        /**
         * Constructs a StateEntry with an empty message.
         * @param state The Task's State.
         */
        explicit StateEntry(State::Type state);

        /**
         * Constructs a StateEntry.
         * @param state The Task's State.
         * @param message A message describing the change in State.
         */
        StateEntry(State state, const std::string& message);
      };

      using Type = StateEntry;

      /**
       * Constructs a Task.
       * @param node The CanvasNode to translate and execute.
       * @param executingAccount The account used to submit orders.
       * @param userProfile The user's profile.
       */
      Task(const CanvasNode& node,
        Beam::ServiceLocator::DirectoryEntry executingAccount,
        Beam::Ref<UserProfile> userProfile);

      /** Returns the CanvasNode used to execute this task. */
      const CanvasNode& GetNode() const;

      /** Returns the context used to translate the node into a reactor. */
      CanvasNodeTranslationContext& GetContext();

      /** Returns the context used to translate the node into a reactor. */
      const CanvasNodeTranslationContext& GetContext() const;

      /** Returns the executor used by the translated reactor. */
      Executor& GetExecutor();

      /** Returns the executor used by the translated reactor. */
      const Executor& GetExecutor() const;

      /** Returns a unique id for this Task. */
      int GetId() const;

      /** Executes this Task. */
      void Execute();

      /** Cancels this Task. */
      void Cancel();

      /** Returns the Publisher indicating the State of this Task. */
      const Beam::Publisher<StateEntry>& GetPublisher() const;

    private:
      std::unique_ptr<CanvasNode> m_node;
      CanvasNodeTranslationContext m_context;
      bool m_isRunning;
      Executor m_executor;
      int m_id;
      Beam::SequencePublisher<StateEntry> m_publisher;

      Task(const Task&) = delete;
      Task(Task&&) = delete;
      Task& operator =(const Task&) = delete;
      Task& operator =(Task&&) = delete;
  };

  /** Streams a string representation of a Task's State. */
  std::ostream& operator <<(std::ostream& o, Task::State state);

  /**
   * Tests if a Task's State represents a terminal state.
   * A terminal state is one of CANCELED, FAILED, EXPIRED, or COMPLETE.
   */
  bool IsTerminal(Task::State state);
}

namespace Beam {
  template<>
  struct EnumeratorCount<Spire::Task::State> :
    std::integral_constant<int, 7> {};
}

#endif
