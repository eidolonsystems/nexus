#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include "spire/spire/resources.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/title_bar.hpp"
#include "spire/ui/ui.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

class security_info_widget : public QWidget {
  public:
    security_info_widget(const QString& security_name,
      const QString& company_name, const QString& icon_path, QWidget* parent = nullptr);

  private:
    QLabel* m_security_name_label;
    QLabel* m_company_name_label;
    QLabel* m_icon_label;
};

security_info_widget::security_info_widget(const QString& security_name,
    const QString& company_name, const QString& icon_path, QWidget* parent)
    : QWidget(parent) {
  //setStyleSheet("background-color: green;");
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(8), scale_height(6), scale_width(8),
    scale_height(6));
  layout->setSpacing(scale_height(3));
  auto top_line_layout = new QHBoxLayout();
  top_line_layout->setMargin(0);
  top_line_layout->setSpacing(0);
  m_security_name_label = new QLabel(security_name, this);
  m_security_name_label->setStyleSheet(R"(
    background-color: transparent;
    color: #333333;
    font-family: Roboto;
    font-size: 18px;
  )");
  top_line_layout->addWidget(m_security_name_label);
  m_icon_label = new QLabel(this);
  m_icon_label->setFixedSize(scale(14, 14));
  m_icon_label->setStyleSheet("background-color: transparent;");

  m_icon_label->setPixmap(QPixmap::fromImage(
    QImage(icon_path).scaled(scale(14, 8), Qt::KeepAspectRatio,
      Qt::SmoothTransformation)));
  top_line_layout->addWidget(m_icon_label);
  layout->addLayout(top_line_layout);
  m_company_name_label = new QLabel(company_name, this);
  m_company_name_label->setStyleSheet(R"(
    background-color: transparent;
    color: #8C8C8C;
    font-family: Roboto;
    font-size: 18px;
  )");
  layout->addWidget(m_company_name_label);
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();

  auto window = new QWidget();
  auto layout = new QHBoxLayout(window);
  layout->setMargin(25);
  auto search_widget = new QWidget(window);
  search_widget->setStyleSheet("border: 1px solid #4B23A0;");
  search_widget->setFixedSize(scale(180, 30));
  layout->addWidget(search_widget);
  auto line_edit = new QLineEdit(search_widget);
  line_edit->setStyleSheet(R"(
    QLineEdit {
      background-color: white;
      border: none;
      font-family: Roboto;
      font-size: 18px;
      padding: 13px 0px 13px 12px;
    })");
  auto label = new QLabel(window);
  label->setPixmap(QPixmap::fromImage(imageFromSvg(":/icons/search.svg", scale(10, 10))));
  label->setStyleSheet(R"(
    background-color: white;
    border: none;
    padding: 13px 12px 13px 0px;
  )");
  auto search_layout = new QHBoxLayout(search_widget);
  search_layout->setMargin(1);
  search_layout->setSpacing(0);
  search_layout->addWidget(line_edit);
  search_layout->addWidget(label);


  window->show();

  //auto results_widget = new QListWidget();
  ////auto results_layout = new QVBoxLayout(results_widget);
  ////results_layout->setMargin(0);
  ////results_layout->setSpacing(0);
  //results_widget->setSizeAdjustPolicy(QListWidget::AdjustToContents);
  //results_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  //results_widget->setObjectName("results_widget");
  //results_widget->setFixedSize(search_widget->width(), scale_height(200));
  //results_widget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  //results_widget->setStyleSheet(R"(
  //  #results_widget {
  //    border-left: 1px solid red;
  //    border-top: none;
  //    border-right: 1px solid red;
  //    border-bottom 1px solid red;
  //    outline: 0;
  //  }

  //  #results_widget::item:selected {
  //    background-color: transparent;
  //  }

  //  QScrollBar::sub-line:vertical {
  //    border: none;
  //    background: none;
  //  }

  //  QScrollBar::add-line:vertical {
  //    border: none;
  //    background: none;
  //  })");

  //for(int i = 0; i < 10; ++i) {
  //  auto item = new QListWidgetItem(results_widget);
  //  //item->setFix
  //  auto test = new security_info_widget("KEN.TSX",
  //  "KenCorp", ":/icons/canada.png");
  //  test->setFixedSize(scale(166, 400));
  //  //results_layout->addWidget(test);
  //  results_widget->setItemWidget(item, test);
  //}

  auto results_widget = new QWidget();
  results_widget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  results_widget->setFixedSize(search_widget->width(), scale_height(200));
  auto results_layout = new QHBoxLayout(results_widget);
  results_layout->setMargin(0);
  results_layout->setSpacing(0);



  auto inner_results_widget = new QWidget(results_widget);
  inner_results_widget->setStyleSheet("background-color: white");
  //inner_results_widget->setObjectName("temp_name");
  //inner_results_widget->setStyleSheet(R"(
  //  #temp_name {
  //    background-color: #FFFFFF;
  //    border-bottom: 1px solid #A0A0A0;
  //    border-left: 1px solid #A0A0A0;
  //    border-right: 1px solid #A0A0A0;
  //    border-top: none;
  //  })");
  auto scroller = new QScrollArea(results_widget);
  scroller->setObjectName("temp_name");
  scroller->setFrameShape(QFrame::NoFrame);
  scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroller->setStyleSheet(R"(
    #temp_name {
      background-color: #FFFFFF;
      border-bottom: 1px solid #A0A0A0;
      border-left: 1px solid #A0A0A0;
      border-right: 1px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
      width: 18px;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
    }

    QScrollBar::sub-line:vertical {
      border: none;
      background: none;
    }

    QScrollBar::add-line:vertical {
      border: none;
      background: none;
    })");
  results_layout->addWidget(scroller);

  auto inner_results_layout = new QVBoxLayout(inner_results_widget);
  inner_results_layout->setMargin(0);
  inner_results_layout->setSpacing(0);

  for(int i = 0; i < 10; ++i) {
    auto test = new security_info_widget("KEN.TSX",
    QString("KenCorp%1").arg(i), ":/icons/canada.png");
    test->setFixedSize(scale(166, 40));
    test->setStyleSheet(":hover { background-color: #F2F2FF; }");
    inner_results_layout->addWidget(test);
  }

  scroller->setWidget(inner_results_widget);

  auto pos = window->mapToGlobal(search_widget->geometry().topLeft());
  results_widget->move(pos.x(), pos.y() + search_widget->height());
  results_widget->show();

  application->exec();
}
