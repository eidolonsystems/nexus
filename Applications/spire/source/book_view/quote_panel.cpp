#include "spire/book_view/quote_panel.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include "spire/book_view/book_view_model.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Nexus;
using namespace Spire;

QuotePanel::QuotePanel(const BookViewModel& model, Side side,
    QWidget* parent)
    : QWidget(parent),
      m_side(side) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  m_indicator_widget = new QWidget(this);
  m_indicator_widget->setFixedHeight(scale_height(2));
  m_indicator_widget->setAutoFillBackground(true);
  set_indicator_color("#C8C8C8");
  layout->addWidget(m_indicator_widget);
  auto label_layout = new QHBoxLayout();
  label_layout->setContentsMargins({});
  label_layout->setSpacing(0);
  label_layout->addStretch(1);
  m_price_label = new QLabel(this);
  m_price_label->setAlignment(Qt::AlignCenter);
  m_price_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    font-weight: 550;)").arg(scale_height(12)));
  label_layout->addWidget(m_price_label);
  m_size_label = new QLabel(this);
  m_size_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  m_size_label->setStyleSheet(QString(R"(
    color: #4B23A0;
    font-family: Roboto;
    font-size: %1px;
    padding-top: %2px;
    font-weight: 550;)").arg(scale_height(10)).arg(scale_height(1)));
  label_layout->addWidget(m_size_label);
  label_layout->addStretch(1);
  layout->addLayout(label_layout);
  auto& bbo = model.get_bbo();
  if(m_side == Side::BID) {
    set_quote_text(bbo.m_bid.m_price, bbo.m_bid.m_size);
  } else {
    set_quote_text(bbo.m_ask.m_price, bbo.m_ask.m_size);
  }
  set_model(model);
}

void QuotePanel::set_model(const BookViewModel& model) {
  m_bbo_connection = model.connect_bbo_slot(
    [=] (auto& b) { on_bbo_quote(b); });
}

void QuotePanel::set_indicator_color(const QColor& color) {
  m_indicator_widget->setStyleSheet(
    QString("background-color: %1").arg(color.name()));
}

void QuotePanel::set_quote_text(const Money& price, const Quantity& size) {
    m_price_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_price_label->setText(QString::fromStdString(price.ToString()));
    m_size_label->setText(QString::fromStdString(
      " / " + Beam::ToString(size)));
}

void QuotePanel::on_bbo_quote(const BboQuote& bbo) {
  auto side = [&] (auto& b) { 
    if(m_side == Side::BID) {
      return b.m_bid;
    }
    return b.m_ask;
  };
  auto quote = side(bbo);
  auto current_quote = side(m_current_bbo);
  if(quote.m_price > current_quote.m_price) {
    set_indicator_color("#37D186");
  } else if(quote.m_price < current_quote.m_price) {
    set_indicator_color("#FF6F7A");
  }
  set_quote_text(quote.m_price, quote.m_size);
  m_current_bbo = bbo;
}
