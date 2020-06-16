#include "Spire/Ui/FontSelectorWidget.hpp"
#include <vector>
#include <QFontDatabase>
#include <QGridLayout>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  auto BUTTON_SIZE() {
    static auto button_size = scale(20, 20);
    return button_size;
  }

  auto HORIZONTAL_SPACING() {
    static auto spacing = scale_width(8);
    return spacing;
  }

  auto VERTICAL_SPACING() {
    static auto spacing = scale_height(8);
    return spacing;
  }
}

FontSelectorWidget::FontSelectorWidget(const QFont& current_font,
    QWidget* parent)
    : QWidget(parent),
      m_current_font(current_font) {
  auto layout = new QGridLayout(this);
  layout->setHorizontalSpacing(HORIZONTAL_SPACING());
  layout->setVerticalSpacing(VERTICAL_SPACING());
  auto fonts = QFontDatabase().families();
  auto font_list = [&] {
    auto list = std::vector<QString>();
    for(auto& font : fonts) {
      list.push_back(font);
    }
    return list;
  }();
  m_font_list = new DropDownMenu(font_list, this);
  m_font_list->set_current_text(m_current_font.family());
  m_font_list->setFixedSize(scale(162, 26));
  m_font_list->connect_selected_signal([=] (const auto& font) {
    on_font_selected(font);
  });
  layout->addWidget(m_font_list, 0, 0, 1, 6);
  m_style_list = new DropDownMenu({}, this);
  update_style_list();
  m_style_list->set_current_text(m_current_font.styleName());
  qDebug() << m_current_font.styleName();
  m_style_list->setFixedSize(scale(104, 26));
  layout->addWidget(m_style_list, 1, 0, 1, 4);
  m_size_list = new DropDownMenu({"6", "7", "8", "9", "10", "11", "12", "14",
    "16", "18", "20", "22", "24", "26", "28", "36", "48", "72"}, this);
  m_size_list->setFixedSize(scale(50, 26));
  layout->addWidget(m_size_list, 1, 4, 1, 2);
  m_bold_button = new FlatButton(this);
  m_bold_button->setFixedSize(BUTTON_SIZE());
  layout->addWidget(m_bold_button, 2, 0);
  m_italics_button = new FlatButton(this);
  m_italics_button->setFixedSize(BUTTON_SIZE());
  layout->addWidget(m_italics_button, 2, 1);
  m_underline_button = new FlatButton(this);
  m_underline_button->setFixedSize(BUTTON_SIZE());
  layout->addWidget(m_underline_button, 2, 2);
  m_strikethrough_button = new FlatButton(this);
  m_strikethrough_button->setFixedSize(BUTTON_SIZE());
  layout->addWidget(m_strikethrough_button, 2, 3);
}

void FontSelectorWidget::update_style_list() {
  auto styles = QFontDatabase().styles(m_current_font.family());
  auto style_list = [&] {
    auto list = std::vector<QString>();
    for(auto& style : styles) {
      list.push_back(style);
    }
    return list;
  }();
  m_style_list->set_items(style_list);
}

void FontSelectorWidget::on_font_selected(const QString& family) {
  m_current_font.setFamily(family);
  update_style_list();
}

void FontSelectorWidget::on_size_selected(const QString& size) {

}

void FontSelectorWidget::on_style_selected(const QString& style) {

}
