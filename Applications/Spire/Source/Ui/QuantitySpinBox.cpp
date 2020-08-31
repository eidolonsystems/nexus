#include "Spire/Ui/QuantitySpinBox.hpp"
#include <QHBoxLayout>

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_quantity(RealSpinBox::Real value) {
    if(auto quantity = Quantity::FromValue(value.str(
        std::numeric_limits<Quantity>::digits10, std::ios_base::dec))) {
      return *quantity;
    }
    throw std::runtime_error(R"(QuantitySpinBox: failed to convert Real to
      Quantity.")");
  }
}

QuantitySpinBox::QuantitySpinBox(Quantity value, QWidget* parent)
    : QAbstractSpinBox(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_spin_box = new RealSpinBox(to_real(value), this);
  m_spin_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusProxy(m_spin_box);
  layout->addWidget(m_spin_box);
  m_spin_box->connect_change_signal([=] (auto value) {
    m_change_signal(to_quantity(value));
  });
  connect(m_spin_box, &RealSpinBox::editingFinished, this,
    &QuantitySpinBox::editingFinished);
  m_locale.setNumberOptions(m_locale.numberOptions().setFlag(
    QLocale::OmitGroupSeparator, true));
}

connection QuantitySpinBox::connect_change_signal(
    const ChangeSignal::slot_type& slot) const {
  return m_change_signal.connect(slot);
}

void QuantitySpinBox::set_minimum(Quantity minimum) {
  m_spin_box->set_minimum(to_real(minimum));
}

void QuantitySpinBox::set_maximum(Quantity maximum) {
  m_spin_box->set_maximum(to_real(maximum));
}

Quantity QuantitySpinBox::get_step() const {
  return to_quantity(m_spin_box->get_step());
}

void QuantitySpinBox::set_step(Quantity step) {
  m_spin_box->set_step(to_real(step));
}

Quantity QuantitySpinBox::get_value() const {
  return to_quantity(m_spin_box->get_value());
}

void QuantitySpinBox::set_value(Quantity value) {
  m_spin_box->set_value(to_real(value));
}

RealSpinBox::Real QuantitySpinBox::to_real(Nexus::Quantity value) {
  return m_item_delegate.displayText(
    QVariant::fromValue(value), m_locale).toStdString().c_str();
}