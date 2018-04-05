#include "spire/login/chroma_hash_widget.hpp"
#include <Beam/ServiceLocator/SessionEncryption.hpp>
#include <QHBoxLayout>

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace spire;

chroma_hash_widget::chroma_hash_widget(QWidget* parent)
    : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  for(auto i = 0; i < static_cast<int>(m_color_widgets.size()); ++i) {
    m_color_widgets[i] = new QWidget(this);
    m_color_widgets[i]->setStyleSheet("background-color: red;");
    layout->addWidget(m_color_widgets[i]);
  }
  set_text({});
}

void chroma_hash_widget::set_text(const QString& text) {
  const auto COLOR_LENGTH = 6;
  auto hash = ComputeSHA(text.toStdString());
  for(auto i = 0; i < COMPONENTS; ++i) {
    auto color_name = "#" + hash.substr(COLOR_LENGTH * i, COLOR_LENGTH);
    m_colors[i] = QColor(QString::fromStdString(color_name));
    m_color_widgets[i]->setStyleSheet(QString("background-color: %1;")
      .arg(m_colors[i].name()));
  }
}
