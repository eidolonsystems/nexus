#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Spire;

namespace {
  auto nextId = 0;
}

Task::StateEntry::StateEntry(State state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State::Type state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State state, const std::string& message)
  : m_state(state),
    m_message(message) {}

Task::Task(const CanvasNode& node, DirectoryEntry executingAccount,
  Ref<UserProfile> userProfile)
  : m_node(
      [&] {
        auto n = PreprocessTranslation(node);
        if(n == nullptr) {
          return CanvasNode::Clone(node);
        }
        return n;
      }()),
    m_context(Ref(userProfile), Ref(m_executor), std::move(executingAccount)),
    m_id(++nextId),
    m_isRunning(false) {}

const CanvasNode& Task::GetNode() const {
  return *m_node;
}

CanvasNodeTranslationContext& Task::GetContext() {
  return m_context;
}

const CanvasNodeTranslationContext& Task::GetContext() const {
  return m_context;
}

Executor& Task::GetExecutor() {
  return m_executor;
}

const Executor& Task::GetExecutor() const {
  return m_executor;
}

int Task::GetId() const {
  return m_id;
}

void Task::Execute() {
  if(m_isRunning) {
    return;
  }
  m_isRunning = true;
  auto translation = Translate(m_context, *m_node).Extract<Aspen::Box<void>>();
  m_executor.Add(std::move(translation));
  m_executor.Open();
}

void Task::Cancel() {}

const Publisher<Task::StateEntry>& Task::GetPublisher() const {
  return m_publisher;
}

std::ostream& Spire::operator <<(std::ostream& s, Task::State state) {
  if(state == Task::State::NONE) {
    s << "None";
  } else if(state == Task::State::INITIALIZING) {
    s << "Initializing";
  } else if(state == Task::State::ACTIVE) {
    s << "Active";
  } else if(state == Task::State::PENDING_CANCEL) {
    s << "Pending Cancel";
  } else if(state == Task::State::CANCELED) {
    s << "Canceled";
  } else if(state == Task::State::FAILED) {
    s << "Failed";
  } else if(state == Task::State::EXPIRED) {
    s << "Expired";
  } else if(state == Task::State::COMPLETE) {
    s << "Complete";
  }
  return s;
}

bool Spire::IsTerminal(Task::State state) {
  return state == Task::State::CANCELED || state == Task::State::FAILED ||
    state == Task::State::EXPIRED || state == Task::State::COMPLETE;
}
