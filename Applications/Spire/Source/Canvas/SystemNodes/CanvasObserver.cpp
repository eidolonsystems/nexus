#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include <Aspen/Aspen.hpp>
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/Translation.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

namespace {
  constexpr auto UPDATE_INTERVAL = 100;

  struct ObserverTranslator {
    template<typename T>
    static Aspen::Box<void> Template(const Translation& translation,
        const std::function<void (const any& value)>& callback) {
      return Aspen::box(Aspen::lift(
        [=] (const T& value) {
          callback(value);
        }, translation.Extract<Aspen::SharedBox<T>>()));
    }

    using SupportedTypes = ValueTypes;
  };
}

CanvasObserver::CanvasObserver(std::shared_ptr<Task> task,
    const CanvasNode& observer)
    : m_task(std::move(task)),
      m_isTranslated(false) {
  auto children = std::vector<CustomNode::Child>();
  children.push_back(CustomNode::Child("target", m_task->GetNode().GetType()));
  children.push_back(CustomNode::Child("observer", observer.GetType()));
  auto customObserver = Refresh(std::make_unique<CustomNode>(
    "Observer", std::move(children))->Replace(
    "target", CanvasNode::Clone(m_task->GetNode()))->Replace("observer",
    CanvasNode::Clone(observer)));
  auto validationErrors = Validate(*customObserver);
  if(!validationErrors.empty()) {
    return;
  }
  auto& taskNode = customObserver->GetChildren().front();
  for(auto& child : BreadthFirstView(*customObserver->FindChild("observer"))) {
    if(auto reference = dynamic_cast<const ReferenceNode*>(&child)) {
      auto& referent = *reference->FindReferent();
      if(IsParent(taskNode, referent) || &referent == &taskNode) {
        auto path = GetPath(taskNode, referent);
        auto& dependency = *m_task->GetNode().FindNode(path);
        m_dependencies.push_back(&dependency);
        m_remainingDependencies.push_back(&dependency);
      }
    }
  }
  m_observer = std::move(customObserver);
  auto translatedNode = PreprocessTranslation(*m_observer);
  if(translatedNode != nullptr) {
    m_observer = std::move(translatedNode);
  }
  QObject::connect(&m_updateTimer, &QTimer::timeout,
    std::bind(&CanvasObserver::OnUpdateTimer, this));
  m_updateTimer.start(UPDATE_INTERVAL);
}

const any& CanvasObserver::GetValue() const {
  return m_value;
}

connection CanvasObserver::ConnectUpdateSignal(
    const UpdateSignal::slot_type& slot) const {
  return m_updateSignal.connect(slot);
}

void CanvasObserver::Translate() {
  if(m_isTranslated) {
    return;
  }
  try {
    bool monitorTranslated = false;
    while(!m_remainingDependencies.empty()) {
      auto dependency = m_remainingDependencies.back();
      if(m_task->GetContext().FindSubTranslation(
          *dependency).is_initialized()) {
        m_remainingDependencies.pop_back();
      } else {
        break;
      }
    }
    if(m_remainingDependencies.empty()) {
      auto context = CanvasNodeTranslationContext(
        Ref(m_task->GetContext().GetUserProfile()), Ref(m_task->GetExecutor()),
        m_task->GetContext().GetExecutingAccount());
      for(const auto& rootDependency : m_dependencies) {
        auto monitorDependency = &*m_observer->GetChildren().front().FindNode(
          GetFullName(*rootDependency));
        Mirror(*rootDependency, m_task->GetContext(), *monitorDependency,
          Store(context));
      }
      auto observer = Spire::Translate(context,
        m_observer->GetChildren().back());
      auto reactor = Instantiate<ObserverTranslator>(observer.GetTypeInfo())(
        observer, m_callbacks.MakeSlot([=] (const any& value) {
          OnReactorUpdate(value);
        }));
      m_task->GetExecutor().Add(std::move(reactor));
      monitorTranslated = true;
      m_isTranslated = true;
    }
  } catch(const std::exception&) {
    return;
  }
}

void CanvasObserver::OnReactorUpdate(const any& value) {
  m_tasks.Push(
    [=] {
      m_value = value;
      m_updateSignal(m_value);
    });
}

void CanvasObserver::OnUpdateTimer() {
  Translate();
  HandleTasks(m_tasks);
}
