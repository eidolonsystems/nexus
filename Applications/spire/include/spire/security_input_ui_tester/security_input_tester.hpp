#ifndef SPIRE_SECURITY_INPUT_TESTER
#define SPIRE_SECURITY_INPUT_TESTER
#include <QListWidget>
#include "Nexus/Definitions/Definitions.hpp"
#include "spire/security_input/local_security_input_model.hpp"
#include "spire/security_input_ui_tester/security_input_ui_tester.hpp"

namespace spire {

  //! Tester class for security_input_dialog.
  class security_input_tester : public QListWidget {
    public:
      
      //! Constructs an empty security_input_tester.
      security_input_tester(QWidget* parent = nullptr);

    protected:
      void closeEvent(QCloseEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      local_security_input_model m_model;
      security_input_dialog* m_dialog;
  };
}

#endif
