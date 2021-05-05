#include "Spire/Ui/DurationBox.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QHBoxLayout>
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/IntegerBox.hpp"
#include "Spire/Ui/LocalScalarValueModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct HourModel : ScalarValueModel<optional<int>> {
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    optional<int> m_current;
    QValidator::State m_state;
    scoped_connection m_source_connection;

    HourModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)),
          m_state(m_source->get_state()),
          m_source_connection(m_source->connect_current_signal(
            [=] (const auto& current) { on_current(current); })) {
      on_current(m_source->get_current());
    }

    optional<int> get_minimum() const {
      return 0;
    }

    optional<int> get_maximum() const {
      return none;
    }

    int get_increment() const {
      return 1;
    }

    QValidator::State get_state() const {
      return m_state;
    }

    const optional<int>& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const Type& value) {
      auto source_current = m_source->get_current();
      auto self_current = hours(m_current.get_value_or(0));
      auto updated_current = hours(value.get_value_or(0));
      if(source_current) {
        *source_current += updated_current - self_current;
      } else {
        source_current = updated_current - self_current;
      }
      if(m_source->set_current(source_current) != QValidator::State::Invalid) {
        return m_state;
      }
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const {
      return m_current_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        m_current = static_cast<int>(current->hours());
      } else {
        m_current = none;
      }
      m_state = QValidator::State::Acceptable;
      m_current_signal(m_current);
    }
  };

  struct MinuteModel : ScalarValueModel<optional<int>> {
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    optional<int> m_current;

    MinuteModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)) {
      m_source->connect_current_signal([=] (const auto& current) {
        on_current(current);
      });
    }

    optional<int> get_minimum() const {
      return 0;
    }

    optional<int> get_maximum() const {
      return 59;
    }

    int get_increment() const {
      return 1;
    }

    QValidator::State get_state() const {
      return QValidator::State::Invalid;
    }

    const optional<int>& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const Type& value) {
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const {
      return m_current_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        set_current(static_cast<int>(current->minutes()));
      } else {
        set_current(none);
      }
    }
  };

  struct SecondModel : ScalarValueModel<optional<DecimalBox::Decimal>> {
    mutable CurrentSignal m_current_signal;
    std::shared_ptr<OptionalDurationModel> m_source;
    optional<DecimalBox::Decimal> m_current;

    SecondModel(std::shared_ptr<OptionalDurationModel> source)
        : m_source(std::move(source)) {
      m_source->connect_current_signal([=] (const auto& current) {
        on_current(current);
      });
    }

    optional<DecimalBox::Decimal> get_minimum() const {
      return 0;
    }

    optional<DecimalBox::Decimal> get_maximum() const {
      return DecimalBox::Decimal("59.999");
    }

    DecimalBox::Decimal get_increment() const {
      return 1;
    }

    QValidator::State get_state() const {
      return QValidator::State::Invalid;
    }

    const optional<DecimalBox::Decimal>& get_current() const {
      return m_current;
    }

    QValidator::State set_current(const Type& value) {
      return QValidator::State::Invalid;
    }

    connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const {
      return m_current_signal.connect(slot);
    }

    void on_current(const optional<time_duration>& current) {
      if(current) {
        auto seconds = (*current - hours(current->hours()) -
          minutes(current->minutes())).total_milliseconds() / 1000;
        set_current(DecimalBox::Decimal(seconds));
      } else {
        set_current(none);
      }
    }
  };

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(BodyAlign(Qt::AlignCenter)).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(horizontal_padding(scale_width(4)));
    style.get(Hover()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  auto FOCUS_IN_STYLE(StyleSheet style) {
    style.get(Any()).set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    return style;
  }

  auto FOCUS_OUT_STYLE(StyleSheet style) {
    style.get(Any()).set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  auto HOUR_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility(VisibilityOption::NONE));
    return style;
  }

  auto MINUTE_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(2)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility(VisibilityOption::NONE));
    return style;
  }

  auto SECOND_FIELD_STYLE(StyleSheet style) {
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_size(0)).
      set(horizontal_padding(scale_width(0))).
      set(LeadingZeros(2)).set(TrailingZeros(3)).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter)));
    style.get(Any() > is_a<Button>()).set(Visibility(VisibilityOption::NONE));
    return style;
  }

  auto COLON_FIELD_STYLE(StyleSheet style) {
    style.get(Any() > Colon()).
      set(TextAlign(Qt::Alignment(Qt::AlignCenter))).
      set(TextColor(QColor::fromRgb(0, 0, 0)));
    style.get(Disabled() > Colon()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }

  template<typename T>
  auto create_modifiers() {
    return QHash<Qt::KeyboardModifier, T>(
      {{Qt::NoModifier, 1}, {Qt::AltModifier, 5}, {Qt::ControlModifier, 10},
       {Qt::ShiftModifier, 20}});
  }
}

DurationBox::DurationBox(QWidget* parent)
  : DurationBox(std::make_shared<LocalOptionalDurationModel>(), parent) {}

DurationBox::DurationBox(std::shared_ptr<OptionalDurationModel> model,
    QWidget* parent)
    : QWidget(parent),
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_is_warning_displayed(true) {
  auto container = new QWidget(this);
  container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  create_hour_field();
  create_minute_field();
  create_second_field();
  create_colons();
  auto container_layout = new QHBoxLayout(container);
  container_layout->setContentsMargins({});
  container_layout->setSpacing(0);
  container_layout->addWidget(m_hour_field, 6);
  container_layout->addWidget(m_hour_minute_colon);
  container_layout->addWidget(m_minute_field, 7);
  container_layout->addWidget(m_minute_second_colon);
  container_layout->addWidget(m_second_field, 11);
  set_style(*container, COLON_FIELD_STYLE(get_style(*container)));
  m_box = new Box(container);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_box);
  setFocusPolicy(Qt::StrongFocus);
  setFocusProxy(m_box);
  proxy_style(*this, *m_box);
  set_style(*this, DEFAULT_STYLE());
  m_hour_field->connect_submit_signal([=] (const auto& submission) {
    if(m_hour_field->hasFocus()) {
      on_submit();
    }
  });
  m_minute_field->connect_submit_signal([=] (const auto& submission) {
    if(m_minute_field->hasFocus()) {
      on_submit();
    }
  });
  m_second_field->connect_submit_signal([=] (const auto& submission) {
    if(m_second_field->hasFocus()) {
      on_submit();
    }
  });
  m_hour_field->connect_reject_signal([=] (const auto& value) { on_reject(); });
  m_minute_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
  m_second_field->connect_reject_signal(
    [=] (const auto& value) { on_reject(); });
}

const std::shared_ptr<OptionalDurationModel>& DurationBox::get_model() const {
  return m_model;
}

bool DurationBox::is_warning_displayed() const {
  return m_is_warning_displayed;
}

void DurationBox::set_warning_displayed(bool is_displayed) {
  m_is_warning_displayed = is_displayed;
}

connection DurationBox::connect_reject_signal(
    const RejectSignal::slot_type& slot) const {
  return m_reject_signal.connect(slot);
}

connection DurationBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize DurationBox::sizeHint() const {
  return scale(126, 26);
}

bool DurationBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    set_style(*this, FOCUS_IN_STYLE(get_style(*this)));
  } else if(event->type() == QEvent::FocusOut) {
    if(!m_hour_field->hasFocus() && !m_minute_field->hasFocus() &&
        !m_second_field->hasFocus()) {
      set_style(*this, FOCUS_OUT_STYLE(get_style(*this)));
      if(m_hour_field->get_model()->get_state() == QValidator::Acceptable &&
          m_minute_field->get_model()->get_state() == QValidator::Acceptable &&
          m_second_field->get_model()->get_state() == QValidator::Acceptable) {
        on_submit();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

void DurationBox::create_hour_field() {
  m_hour_field = new IntegerBox(std::make_shared<HourModel>(m_model),
    create_modifiers<int>());
  m_hour_field->setMinimumWidth(scale_width(24));
  m_hour_field->set_placeholder("hh");
  m_hour_field->set_warning_displayed(false);
  set_style(*m_hour_field, HOUR_FIELD_STYLE(get_style(*m_hour_field)));
  m_hour_field->findChild<QLineEdit*>()->installEventFilter(this);
}

void DurationBox::create_minute_field() {
  m_minute_field = new IntegerBox(std::make_shared<MinuteModel>(m_model),
    create_modifiers<int>());
  m_minute_field->setMinimumWidth(scale_width(28));
  m_minute_field->set_placeholder("mm");
  m_minute_field->set_warning_displayed(false);
  set_style(*m_minute_field, MINUTE_FIELD_STYLE(get_style(*m_minute_field)));
  m_minute_field->findChild<QLineEdit*>()->installEventFilter(this);
}

void DurationBox::create_second_field() {
  m_second_field = new DecimalBox(std::make_shared<SecondModel>(m_model),
    create_modifiers<DecimalBox::Decimal>());
  m_second_field->setMinimumWidth(scale_width(44));
  m_second_field->set_placeholder("ss.sss");
  m_second_field->set_warning_displayed(false);
  set_style(*m_second_field, SECOND_FIELD_STYLE(get_style(*m_second_field)));
  m_second_field->findChild<QLineEdit*>()->installEventFilter(this);
}

void DurationBox::create_colons() {
  m_hour_minute_colon = new TextBox(":");
  m_hour_minute_colon->setFixedWidth(scale_width(10));
  m_hour_minute_colon->setEnabled(false);
  m_hour_minute_colon->set_read_only(true);
  m_minute_second_colon = new TextBox(":");
  m_minute_second_colon->setFixedWidth(scale_width(10));
  m_minute_second_colon->setEnabled(false);
  m_minute_second_colon->set_read_only(true);
  find_stylist(*m_hour_minute_colon).match(Colon());
  find_stylist(*m_minute_second_colon).match(Colon());
}

void DurationBox::on_submit() {
  auto minimum = m_model->get_minimum();
  auto maximum = m_model->get_maximum();
  if(minimum && m_model->get_current() < *minimum ||
      maximum && m_model->get_current() > *maximum) {
    on_reject();
  } else {
    m_submission = m_model->get_current();
    m_submit_signal(m_submission);
  }
}

void DurationBox::on_reject() {
  m_reject_signal(m_model->get_current());
  auto minimum = m_model->get_minimum();
  if(minimum && m_submission < *minimum) {
    m_submission = *minimum;
  }
  auto maximum = m_model->get_maximum();
  if(maximum && m_submission > *maximum) {
    m_submission = *maximum;
  }
  m_model->set_current(m_submission);
  if(m_is_warning_displayed) {
    display_warning_indicator(*this);
  }
}
