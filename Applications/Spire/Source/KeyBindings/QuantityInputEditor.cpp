#include "Spire/KeyBindings/QuantityInputEditor.hpp"
#include <QIntValidator>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

QuantityInputEditor::QuantityInputEditor(int initial_value, QWidget* parent)
    : QLineEdit(parent),
      m_initial_value(initial_value) {
  setValidator(new QIntValidator(0, std::numeric_limits<int>::max(), this));
  setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    color: #000000;
    font-family: Roboto;
    font-size: %1px;
    padding-left: %2px;
  )").arg(scale_height(12)).arg(scale_width(6)));
}

void QuantityInputEditor::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Delete) {
    setText("0");
    emit editingFinished();
    return;
  } else if(event->key() == Qt::Key_Enter ||
      event->key() == Qt::Key_Return) {
    if(text().isEmpty()) {
      setText(QString::number(m_initial_value));
    }
    emit editingFinished();
    return;
  }
  insert(event->text());
}