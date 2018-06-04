#ifndef SPIRE_ITEM_PADDING_DELEGATE
#define SPIRE_ITEM_PADDING_DELEGATE
#include <QStyledItemDelegate>
#include "spire/ui/ui.hpp"

namespace spire {

  //! Implements Qt' item delegate to support custom item padding.
  class item_padding_delegate : public QStyledItemDelegate {
    public:

      //! Constructs an item delegate for custom padding.
      /*!
        \param padding The amount of item padding in pixels.
        \param delegate The delegate to add padding to.
        \param parent The parent to this delegate instance.
      */
      item_padding_delegate(int padding, QStyledItemDelegate* delegate,
        QObject* parent = nullptr);

      QString displayText(const QVariant& value,
        const QLocale& locale) const override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    private:
      int m_padding;
      QStyledItemDelegate* m_item_delegate;
  };
}

#endif
