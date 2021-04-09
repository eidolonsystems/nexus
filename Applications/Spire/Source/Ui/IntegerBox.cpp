#include "Spire/Ui/IntegerBox.hpp"
#include <QHBoxLayout>
#include "Spire/Ui/LocalScalarValueModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

struct IntegerBox::IntegerToDecimalModel : DecimalBox::DecimalModel {
  mutable CurrentSignal m_current_signal;
  std::shared_ptr<IntegerModel> m_model;
  DecimalBox::Decimal m_current;
  scoped_connection m_current_connection;

  IntegerToDecimalModel(std::shared_ptr<IntegerModel> model)
    : m_model(std::move(model)),
      m_current(m_model->get_current()),
      m_current_connection(m_model->connect_current_signal(
        [=] (auto current) {
          on_current(current);
        })) {}

  optional<DecimalBox::Decimal> get_minimum() const {
    if(auto minimum = m_model->get_minimum()) {
      return DecimalBox::Decimal(*minimum);
    }
    return none;
  }

  optional<DecimalBox::Decimal> get_maximum() const {
    if(auto maximum = m_model->get_maximum()) {
      return DecimalBox::Decimal(*maximum);
    }
    return none;
  }

  DecimalBox::Decimal get_increment() const {
    return DecimalBox::Decimal(1);
  }

  QValidator::State get_state() const override {
    return m_model->get_state();
  }

  const DecimalBox::Decimal& get_current() const {
    return m_current;
  }

  QValidator::State set_current(const DecimalBox::Decimal& value) override {
    auto state = m_model->set_current(value.convert_to<int>());
    if(state == QValidator::State::Invalid) {
      return QValidator::State::Invalid;
    }
    m_current = value;
    m_current_signal(m_current);
    return state;
  }

  connection connect_current_signal(
      const typename CurrentSignal::slot_type& slot) const override {
    return m_current_signal.connect(slot);
  }

  void on_current(int current) {
    m_current = current;
    m_current_signal(m_current);
  }
};

IntegerBox::IntegerBox(QHash<Qt::KeyboardModifier, int> modifiers,
  QWidget* parent)
  : IntegerBox(std::make_shared<LocalScalarValueModel<int>>(),
      std::move(modifiers), parent) {}

IntegerBox::IntegerBox(std::shared_ptr<IntegerModel> model,
    QHash<Qt::KeyboardModifier, int> modifiers, QWidget* parent)
    : StyledWidget(parent),
      m_model(std::move(model)),
      m_adaptor_model(std::make_shared<IntegerToDecimalModel>(m_model)),
      m_submission(m_model->get_current()) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  auto adapted_modifiers = QHash<Qt::KeyboardModifier, DecimalBox::Decimal>();
  for(auto modifier = modifiers.begin();
      modifier != modifiers.end(); ++modifier) {
    adapted_modifiers.insert(modifier.key(), modifier.value());
  }
  m_decimal_box = new DecimalBox(m_adaptor_model, adapted_modifiers, this);
  add_proxy(*m_decimal_box);
  setFocusProxy(m_decimal_box);
  layout->addWidget(m_decimal_box);
  m_submit_connection = m_decimal_box->connect_submit_signal(
    [=] (const auto& submission) { on_submit(submission); });
  m_reject_connection = m_decimal_box->connect_reject_signal(
    [=] (const auto& value) { on_reject(value); });
}

const std::shared_ptr<IntegerModel>& IntegerBox::get_model() const {
  return m_model;
}

void IntegerBox::set_placeholder(const QString& value) {
  m_decimal_box->set_placeholder(value);
}

bool IntegerBox::is_read_only() const {
  return m_decimal_box->is_read_only();
}

void IntegerBox::set_read_only(bool is_read_only) {
  m_decimal_box->set_read_only(is_read_only);
}

bool IntegerBox::is_warning_displayed() const {
  return m_decimal_box->is_warning_displayed();
}

void IntegerBox::set_warning_displayed(bool is_displayed) {
  m_decimal_box->set_warning_displayed(is_displayed);
}

connection IntegerBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

connection IntegerBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

void IntegerBox::on_submit(const DecimalBox::Decimal& submission) {
  m_submission = m_model->get_current();
  m_submit_signal(m_submission);
}

void IntegerBox::on_reject(const DecimalBox::Decimal& value) {
  m_reject_signal(value.convert_to<int>());
}
