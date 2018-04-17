#include "spire/time_and_sales/time_and_sales_properties.hpp"

using namespace spire;

time_and_sales_properties::time_and_sales_properties() {
  set_text_color(price_range::UNKNOWN, QColor(0, 0, 0));
  set_band_color(price_range::UNKNOWN, QColor(235, 235, 235));
  set_text_color(price_range::ABOVE_ASK, QColor(0, 0, 0));
  set_band_color(price_range::ABOVE_ASK, QColor(154, 255, 150));
  set_text_color(price_range::AT_ASK, QColor(0, 0, 0));
  set_band_color(price_range::AT_ASK, QColor(154, 255, 150));
  set_text_color(price_range::INSIDE, QColor(0, 0, 0));
  set_band_color(price_range::INSIDE, QColor(255, 255, 255));
  set_text_color(price_range::AT_BID, QColor(0, 0, 0));
  set_band_color(price_range::AT_BID, QColor(234, 135, 99));
  set_text_color(price_range::BELOW_BID, QColor(0, 0, 0));
  set_band_color(price_range::BELOW_BID, QColor(234, 135, 99));
  m_show_columns.fill(true);
  set_show_column(columns::TIME_COLUMN, false);
  set_show_column(columns::CONDITION_COLUMN, false);
  m_show_grid = false;
  m_font = QFont("Roboto", 11);
}

const QColor& time_and_sales_properties::get_text_color(
    price_range index) const noexcept {
  return m_text_colors[static_cast<int>(index)];
}

void time_and_sales_properties::set_text_color(price_range index,
    const QColor& color) noexcept {
  m_text_colors[static_cast<int>(index)] = color;
}

const QColor& time_and_sales_properties::get_band_color(
    price_range index) const noexcept {
  return m_band_colors[static_cast<int>(index)];
}

void time_and_sales_properties::set_band_color(price_range index,
    const QColor& color) noexcept {
  m_band_colors[static_cast<int>(index)] = color;
}

bool time_and_sales_properties::get_show_column(columns column) const noexcept {
  return m_show_columns[static_cast<int>(column)];
}

void time_and_sales_properties::set_show_column(
    columns column, bool show) noexcept {
  m_show_columns[static_cast<int>(column)] = show;
}
