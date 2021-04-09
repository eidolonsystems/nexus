#include "Spire/Styles/Stylist.hpp"
#include <deque>
#include <QApplication>
#include <QEvent>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  std::unordered_map<QWidget*, Stylist*> stylists;
}

struct Stylist::StyleEventFilter : QObject {
  Stylist* m_stylist;

  StyleEventFilter(Stylist& stylist)
      : QObject(stylist.m_widget),
        m_stylist(&stylist) {
    connect(qApp, &QApplication::focusChanged, this,
      &StyleEventFilter::on_focus_changed);
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if(event->type() == QEvent::FocusIn) {
      m_stylist->match(Focus());
    } else if(event->type() == QEvent::FocusOut) {
      m_stylist->unmatch(Focus());
    } else if(event->type() == QEvent::Enter) {
      m_stylist->match(Hover());
    } else if(event->type() == QEvent::Leave) {
      m_stylist->unmatch(Hover());
    } else if(event->type() == QEvent::EnabledChange) {
      if(!m_stylist->m_widget->isEnabled()) {
        m_stylist->match(Disabled());
      } else {
        m_stylist->unmatch(Disabled());
      }
    } else if(event->type() == QEvent::WindowActivate) {
      m_stylist->match(Active());
    } else if(event->type() == QEvent::WindowDeactivate) {
      m_stylist->unmatch(Active());
    }
    return QObject::eventFilter(watched, event);
  }

  void on_focus_changed(QWidget* old, QWidget* now) {
    auto proxy = m_stylist->m_widget->focusProxy();
    while(proxy) {
      if(proxy == now) {
        m_stylist->match(Focus());
        break;
      } else if(proxy == old) {
        m_stylist->unmatch(Focus());
        break;
      }
      proxy = proxy->focusProxy();
    }
  }
};

std::size_t Stylist::SelectorHash::operator ()(const Selector& selector) const {
  return selector.get_type().hash_code();
}

bool Stylist::SelectorEquality::operator ()(
    const Selector& left, const Selector& right) const {
  return left.is_match(right);
}

Stylist::Stylist(QWidget& widget)
  : Stylist({}, widget) {}

Stylist::Stylist(StyleSheet style, QWidget& widget)
    : m_widget(&widget),
      m_style(std::move(style)),
      m_visibility(VisibilityOption::VISIBLE) {
  stylists[m_widget] = this;
  m_widget->installEventFilter(new StyleEventFilter(*this));
}

Stylist::~Stylist() {
  for(auto dependent : m_dependents) {
    auto& stylist = find_stylist(*dependent);
    stylist.apply(*dependent, {});
  }
  while(!m_proxies.empty()) {
    remove_proxy(*m_proxies.front());
  }
  while(!m_principals.empty()) {
    find_stylist(*m_principals.front()).remove_proxy(*m_widget);
  }
}

const StyleSheet& Stylist::get_style() const {
  return m_style;
}

void Stylist::set_style(const StyleSheet& style) {
  m_style = style;
  m_enable_connections.clear();
  auto reach = build_reach(m_style, *m_widget);
  for(auto widget : reach) {
    if(widget != m_widget) {
      m_enable_connections.push_back(
        find_stylist(*widget).connect_enable_signal([=] { on_enable(); }));
    }
  }
  apply_rules();
}

bool Stylist::is_match(const Selector& selector) const {
  if(m_matching_selectors.find(selector) != m_matching_selectors.end()) {
    return true;
  }
  for(auto proxy : m_proxies) {
    if(find_stylist(*proxy).is_match(selector)) {
      return true;
    }
  }
  return false;
}

Block Stylist::compute_style() const {
  auto block = Block();
  for(auto& entry : m_blocks) {
    merge(block, entry->m_block);
  }
  for(auto principal : m_principals) {
    merge(block, find_stylist(*principal).compute_style());
  }
  return block;
}

void Stylist::add_proxy(QWidget& widget) {
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &widget);
  if(i == m_proxies.end()) {
    m_proxies.push_back(&widget);
    auto& stylist = find_stylist(widget);
    stylist.m_principals.push_back(m_widget);
    stylist.apply_proxy_styles();
  }
}

void Stylist::remove_proxy(QWidget& widget) {
  auto i = std::find(m_proxies.begin(), m_proxies.end(), &widget);
  if(i == m_proxies.end()) {
    return;
  }
  auto& stylist = find_stylist(**i);
  stylist.m_principals.erase(std::find(stylist.m_principals.begin(),
    stylist.m_principals.end(), m_widget));
  m_proxies.erase(i);
  stylist.apply_proxy_styles();
}

void Stylist::match(const Selector& selector) {
  if(m_matching_selectors.insert(selector).second) {
    m_enable_signal();
    apply_rules();
    for(auto principal : m_principals) {
      auto& stylist = find_stylist(*principal);
      stylist.apply_rules();
    }
  }
}

void Stylist::unmatch(const Selector& selector) {
  if(m_matching_selectors.erase(selector) != 0) {
    m_enable_signal();
    apply_rules();
    for(auto principal : m_principals) {
      auto& stylist = find_stylist(*principal);
      stylist.apply_rules();
    }
  }
}

connection Stylist::connect_style_signal(
    const StyleSignal::slot_type& slot) const {
  return m_style_signal.connect(slot);
}

void Stylist::apply(const QWidget& source, Block block) {
  auto i = m_source_to_block.find(&source);
  if(i == m_source_to_block.end()) {
    auto entry = std::make_shared<BlockEntry>();
    entry->m_source = &source;
    entry->m_priority = 0;
    m_blocks.push_back(entry);
    i = m_source_to_block.insert(i, std::pair(&source, std::move(entry)));
  }
  i->second->m_block = std::move(block);
  apply_proxy_styles();
}

void Stylist::apply_rules() {
  auto blocks = std::unordered_map<Stylist*, Block>();
  auto principals = std::deque<QWidget*>();
  principals.push_back(m_widget);
  while(!principals.empty()) {
    auto principal = principals.front();
    auto& stylist = find_stylist(*principal);
    principals.pop_front();
    principals.insert(principals.end(), stylist.m_principals.begin(),
      stylist.m_principals.end());
    for(auto& rule : stylist.m_style.get_rules()) {
      auto selection = select(rule.get_selector(), *m_widget);
      for(auto& selected : selection) {
        merge(blocks[&find_stylist(*selected)], rule.get_block());
      }
    }
  }
  auto previous_dependents = std::move(m_dependents);
  for(auto& block : blocks) {
    m_dependents.insert(block.first->m_widget);
    block.first->apply(*m_widget, std::move(block.second));
  }
  for(auto previous_dependent : previous_dependents) {
    if(m_dependents.find(previous_dependent) == m_dependents.end()) {
      auto& stylist = find_stylist(*previous_dependent);
      stylist.apply(*m_widget, {});
    }
  }
  for(auto proxy : m_proxies) {
    find_stylist(*proxy).apply_rules();
  }
}

void Stylist::apply_style() {
  m_style_signal();
  auto style = compute_style();
  auto visibility_option = [&] {
    if(auto visibility = Spire::Styles::find<Visibility>(style)) {
      return visibility->get_expression().as<VisibilityOption>();
    }
    return VisibilityOption::VISIBLE;
  }();
  if(visibility_option != m_visibility) {
    if(visibility_option == VisibilityOption::VISIBLE) {
      m_widget->show();
    } else if(visibility_option == VisibilityOption::NONE) {
      auto size = m_widget->sizePolicy();
      size.setRetainSizeWhenHidden(false);
      m_widget->setSizePolicy(size);
      m_widget->hide();
    } else if(visibility_option == VisibilityOption::INVISIBLE) {
      auto size = m_widget->sizePolicy();
      size.setRetainSizeWhenHidden(true);
      m_widget->setSizePolicy(size);
      m_widget->hide();
    }
    m_visibility = visibility_option;
  }
}

void Stylist::apply_proxy_styles() {
  apply_style();
  for(auto proxy : m_proxies) {
    find_stylist(*proxy).apply_proxy_styles();
  }
}

connection Stylist::connect_enable_signal(
    const EnableSignal::slot_type& slot) const {
  return m_enable_signal.connect(slot);
}

void Stylist::on_enable() {
  apply_rules();
}

const Stylist& Spire::Styles::find_stylist(const QWidget& widget) {
  return find_stylist(const_cast<QWidget&>(widget));
}

Stylist& Spire::Styles::find_stylist(QWidget& widget) {
  auto stylist = stylists.find(&widget);
  if(stylist == stylists.end()) {
    auto entry = new Stylist(widget);
    stylist = stylists.insert(std::pair(&widget, entry)).first;
    QObject::connect(&widget, &QObject::destroyed, [=] (QObject*) {
      delete entry;
    });
  }
  return *stylist->second;
}

const StyleSheet& Spire::Styles::get_style(const QWidget& widget) {
  return find_stylist(widget).get_style();
}

void Spire::Styles::set_style(QWidget& widget, const StyleSheet& style) {
  find_stylist(widget).set_style(style);
}

void Spire::Styles::proxy_style(QWidget& source, QWidget& destination) {
  find_stylist(source).add_proxy(destination);
}
