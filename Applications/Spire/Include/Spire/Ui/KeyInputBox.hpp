#ifndef SPIRE_KEY_INPUT_BOX_HPP
#define SPIRE_KEY_INPUT_BOX_HPP
#include <QHBoxLayout>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class KeyInputBox : public QWidget {
    public:

      using SubmitSignal = Signal<void (const QKeySequence& submission)>;

      KeyInputBox(QWidget* parent = nullptr);

      const QKeySequence& get_submission() const;

      const std::shared_ptr<KeySequenceModel>& get_model() const;

      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      enum class Status {
        NONE,
        PROMPT
      };

      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<KeySequenceModel> m_model;
      QKeySequence m_submission;
      TextBox* m_text_box;

      void set_status(Status status);
      void on_current_sequence(const QKeySequence& sequence);
  };
}

#endif
