#ifndef SPIRE_REAL_SPIN_BOX_MODEL_HPP
#define SPIRE_REAL_SPIN_BOX_MODEL_HPP
#include <QLocale>
#include "Spire/Spire/SpinBoxModel.hpp"
#include "Spire/Ui/RealSpinBox.hpp"

namespace Spire {

  using Real = RealSpinBox::Real;

  //! Represents a RealSpinBox's data model.
  class RealSpinBoxModel {
    public:

      //! Returns an increment value.
      /*
        \param modifiers The modifiers to get the increment value of.
      */
      virtual Real get_increment(Qt::KeyboardModifiers modifiers) const = 0;

      //! Returns the initial value.
      virtual Real get_initial() const = 0;

      //! Returns the minimum acceptable value.
      virtual Real get_minimum() const = 0;

      //! Returns the maximum acceptable value.
      virtual Real get_maximum() const = 0;
  };

  //! Wraps a SpinBoxModel to provide a generic interface.
  template <typename T>
  class RealSpinBoxModelImpl : public RealSpinBoxModel {
    public:

      //! Constructs a RealSpinBoxModelImpl.
      /*
        \param model The source model.
      */
      RealSpinBoxModelImpl(std::shared_ptr<SpinBoxModel<T>> model)
          : m_model(std::move(model)) {
        m_locale.setNumberOptions(m_locale.numberOptions().setFlag(
          QLocale::OmitGroupSeparator, true));
      }

      Real get_increment(Qt::KeyboardModifiers modifiers) const override {
        return to_real(m_model->get_increment(modifiers));
      }

      Real get_initial() const override {
        return to_real(m_model->get_initial());
      }

      Real get_minimum() const override {
        return to_real(m_model->get_minimum());
      }

      Real get_maximum() const override {
        return to_real(m_model->get_maximum());
      }

    private:
      std::shared_ptr<SpinBoxModel<T>> m_model;
      CustomVariantItemDelegate m_item_delegate;
      QLocale m_locale;

      Real variant_to_real(const QVariant& value) const {
        return m_item_delegate.displayText(value,
          m_locale).toStdString().c_str();
      }

      Real to_real(std::int64_t value) const {
        return value;
      }

      Real to_real(double value) const {
        return static_cast<long double>(value);
      }

      Real to_real(Nexus::Quantity value) const {
        return variant_to_real(QVariant::fromValue<Nexus::Quantity>(value));
      }

      Real to_real(Nexus::Money value) const {
        return variant_to_real(QVariant::fromValue<Nexus::Money>(value));
      }
  };
}

#endif
