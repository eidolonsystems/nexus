#include "spire/book_view/labeled_data_widget.hpp"
#include <QHBoxLayout>
#include "spire/spire/dimensions.hpp"

using namespace spire;

labeled_data_widget::labeled_data_widget(const QString& label_text,
    const QString& data_text, QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_label = new QLabel(label_text, this);
  m_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;)").arg(scale_height(10)));
  layout->addWidget(m_label);
  layout->addSpacing(scale_width(4));
  m_data_label = new QLabel(data_text, this);
  m_data_label->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(10)));
  layout->addWidget(m_data_label);
  layout->addStretch(1);
}

void labeled_data_widget::set_label_text(const QString& text) {
  m_label->setText(text);
}

void labeled_data_widget::set_data_text(const QString& text) {
  m_data_label->setText(text);
}
