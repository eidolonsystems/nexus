#ifndef SPIRE_SCROLLABLE_LIST_BOX_HPP
#define SPIRE_SCROLLABLE_LIST_BOX_HPP
#include <QWidget>
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Represents a ScrollBox which contains a ListView.
   */
  class ScrollableListBox : public ScrollBox {
    public:

      /**
       * Constructs a ScrollableListBox.
       * @param list_view The ListView contained within the ScrollBox.
       * @param parent The parent widget.
       */
      explicit ScrollableListBox(ListView* list_view, QWidget* parent = nullptr);

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      ListView* m_list_view;
      QWidget* m_body;
      QSize m_body_size;

      bool is_horizontal_layout_of_body();
      QWidget* make_body();
      void on_current(const boost::optional<std::any>& current);
  };
}

#endif