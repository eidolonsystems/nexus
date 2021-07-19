#include "Spire/Ui/QuantityBox.hpp"

using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct QuantityToDecimalModel : ToDecimalModel<Quantity> {
    using ToDecimalModel<Quantity>::ToDecimalModel;

    Decimal get_increment() const override {
      return Decimal("0.000001");
    }
  };
}

QuantityBox::QuantityBox(QHash<Qt::KeyboardModifier, Quantity> modifiers,
  QWidget* parent)
  : QuantityBox(std::make_shared<LocalOptionalQuantityModel>(),
      std::move(modifiers), parent) {}

QuantityBox::QuantityBox(std::shared_ptr<OptionalQuantityModel> model,
    QHash<Qt::KeyboardModifier, Quantity> modifiers, QWidget* parent)
    : DecimalBoxAdaptor(model, std::make_shared<QuantityToDecimalModel>(model),
        std::move(modifiers), parent) {
  auto style = get_style(get_decimal_box());
  style.get(Any()).set(TrailingZeros(0));
  set_style(get_decimal_box(), std::move(style));
}