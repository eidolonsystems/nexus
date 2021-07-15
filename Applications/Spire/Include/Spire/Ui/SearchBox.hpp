#ifndef SPIRE_SEARCH_BOX_HPP
#define SPIRE_SEARCH_BOX_HPP
#include <QWidget>
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a text search input box. */
  class SearchBox : public QWidget {
    public:

      /**
       * Constructs a SearchBox.
       * @param parent The parent widget.
       */
      explicit SearchBox(QWidget* parent = nullptr);

      /** Returns the model associated with the TextBox. */
      const std::shared_ptr<TextModel>& get_model() const;

      /**
       * Sets the placeholder text.
       * @param text The placeholder text.
       */
      void set_placeholder(const QString& text);

    private:
      boost::signals2::scoped_connection m_current_connection;
      TextBox* m_text_box;
      Button* m_delete_button;
  };
}

#endif
