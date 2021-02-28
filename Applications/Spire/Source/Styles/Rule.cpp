#include "Spire/Styles/Any.hpp"
#include "Spire/Styles/Rule.hpp"

using namespace Spire;
using namespace Spire::Styles;

Rule::Rule(Block block)
  : Rule(Any(), std::move(block)) {}

Rule::Rule(Selector selector, Block block)
  : m_selector(std::move(selector)),
    m_block(std::move(block)) {}

const Selector& Rule::get_selector() const {
  return m_selector;
}

const Block& Rule::get_block() const {
  return m_block;
}

Block& Rule::get_block() {
  return m_block;
}
