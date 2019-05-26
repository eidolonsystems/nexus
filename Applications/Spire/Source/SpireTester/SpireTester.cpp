#include "Spire/SpireTester/SpireTester.hpp"
#include <QDebug>

void Spire::print_test_name(const QString& name) {
  qDebug() << "\n**************************************************";
  qDebug() << name << " PASSED";
  qDebug() << "**************************************************\n";
}
