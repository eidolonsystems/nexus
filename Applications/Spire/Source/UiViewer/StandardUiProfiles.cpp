#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/ColorSelectorButton.hpp"
#include "Spire/Ui/CurrencyComboBox.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"
#include "Spire/UiViewer/UiProfile.hpp"

using namespace Nexus;
using namespace Spire;

UiProfile Spire::make_check_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_bool_property("checked"));
  properties.push_back(make_standard_qstring_property("text"));
  auto profile = UiProfile(QString::fromUtf8("CheckBox"), properties,
    [] (auto& profile) {
      auto& text = get<QString>("text", profile.get_properties());
      auto check_box = make_check_box(text.get());
      apply_widget_properties(check_box, profile.get_properties());
      auto& checked = get<bool>("checked", profile.get_properties());
      checked.connect_changed_signal([=] (auto value) {
        if(value) {
          check_box->setCheckState(Qt::Checked);
        } else {
          check_box->setCheckState(Qt::Unchecked);
        }
      });
      QObject::connect(check_box, &CheckBox::stateChanged, [&] (auto value) {
        checked.set(value == Qt::Checked);
      });
      QObject::connect(check_box, &CheckBox::stateChanged,
        profile.make_event_slot<int>(QString::fromUtf8("stateChanged")));
      return check_box;
    });
  return profile;
}

UiProfile Spire::make_color_selector_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qcolor_property("color"));
  auto profile = UiProfile(QString::fromUtf8("ColorSelectorButton"), properties,
    [] (auto& profile) {
      auto& color = get<QColor>("color", profile.get_properties());
      auto button = new ColorSelectorButton(color.get());
      button->setFixedSize(scale(100, 26));
      apply_widget_properties(button, profile.get_properties());
      color.connect_changed_signal([=] (const auto& value) {
        button->set_color(value);
      });
      button->connect_color_signal([&] (auto value) {
        color.set(value);
      });
      button->connect_color_signal(
        profile.make_event_slot<QColor>(QString::fromUtf8("ColorSignal")));
      return button;
    });
  return profile;
}

UiProfile Spire::make_currency_combo_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_currency_property("currency"));
  auto profile = UiProfile(QString::fromUtf8("CurrencyComboBox"), properties,
    [] (auto& profile) {
      auto combo_box = new CurrencyComboBox(GetDefaultCurrencyDatabase());
      combo_box->setFixedSize(scale(100, 26));
      apply_widget_properties(combo_box, profile.get_properties());
      auto& currency = get<CurrencyId>("currency", profile.get_properties());
      currency.set(combo_box->get_currency());
      currency.connect_changed_signal([=] (auto value) {
        combo_box->set_currency(value);
      });
      combo_box->connect_selected_signal([&] (auto value) {
        currency.set(value);
      });
      combo_box->connect_selected_signal(profile.make_event_slot<CurrencyId>(
        QString::fromUtf8("SelectedSignal")));
      return combo_box;
    });
  return profile;
}

UiProfile Spire::make_flat_button_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_qstring_property("label",
    QString::fromUtf8("Click me!")));
  auto profile = UiProfile(QString::fromUtf8("FlatButton"), properties,
    [] (auto& profile) {
      auto& label = get<QString>("label", profile.get_properties());
      auto button = make_flat_button(label.get());
      button->setFixedSize(scale(100, 26));
      apply_widget_properties(button, profile.get_properties());
      label.connect_changed_signal([=] (const auto& value) {
        button->set_label(value);
      });
      button->connect_clicked_signal(
        profile.make_event_slot(QString::fromUtf8("ClickedSignal")));
      return button;
    });
  return profile;
}
