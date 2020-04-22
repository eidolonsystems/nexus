#ifndef SPIRE_DESTINATION_ITEM_DELEGATE_HPP
#define SPIRE_DESTINATION_ITEM_DELEGATE_HPP
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class DestinationItemDelegate : public KeyBindingItemDelegate {
    public:

      explicit DestinationItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
