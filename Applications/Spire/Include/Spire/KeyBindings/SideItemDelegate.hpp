#ifndef SPIRE_SIDE_ITEM_DELEGATE_HPP
#define SPIRE_SIDE_ITEM_DELEGATE_HPP
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents an item delegate for displaying and editing the side of a key
  //! binding.
  class SideItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs a SideItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit SideItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
