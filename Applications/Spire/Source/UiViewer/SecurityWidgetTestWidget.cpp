#include "Spire/UiViewer/SecurityWidgetTestWidget.hpp"
#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/FlatButton.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Spire;

SecurityWidgetTestWidget::SecurityWidgetTestWidget(QWidget* parent)
    : QWidget(parent),
      m_security_widget(nullptr) {
  auto container = new QWidget(this);
  m_layout = new QGridLayout(container);
  m_layout->addWidget(make_parameters_label(this), 1, 0, 1, 2);
  auto info_label = new QLabel(tr("Click SecurityWidget to set focus."));
  info_label->setAlignment(Qt::AlignCenter);
  m_layout->addWidget(info_label, 2, 0, 1, 2);
  m_dark_theme_check_box = make_check_box(tr("Dark Theme"), this);
  m_layout->addWidget(m_dark_theme_check_box, 3, 0);
  auto reset_button = make_flat_button(tr("Reset"), this);
  reset_button->setFixedSize(scale(100, 26));
  reset_button->connect_clicked_signal([=] { on_reset_button(); });
  m_layout->addWidget(reset_button, 3, 1);
  m_status_label = new QLabel(this);
  m_layout->addWidget(m_status_label, 0, 2, 1, 2,
    Qt::AlignTop | Qt::AlignLeft);
  m_status_label->setFixedWidth(scale_width(100));
  on_reset_button();
}

void SecurityWidgetTestWidget::on_reset_button() {
  delete_later(m_security_widget);
  auto theme = [&] {
    if(m_dark_theme_check_box->isChecked()) {
      return SecurityWidget::Theme::DARK;
    }
    return SecurityWidget::Theme::LIGHT;
  }();
  m_security_widget = new SecurityWidget(
    Ref(LocalSecurityInputModel::get_test_model()), theme, this);
  m_security_widget->setMinimumHeight(scale_height(200));
  m_security_widget->connect_change_security_signal(
    [=] (const auto& security){
      on_security_selected(security);
    });
  m_layout->addWidget(m_security_widget, 0, 0, 1, 2);
  m_status_label->setText("");
}

void SecurityWidgetTestWidget::on_security_selected(const Security& security) {
  m_status_label->setText(m_item_delegate.displayText(
    QVariant::fromValue(security)));
}
