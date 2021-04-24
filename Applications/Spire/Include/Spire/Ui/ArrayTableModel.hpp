#ifndef SPIRE_ARRAY_TABLE_MODEL_HPP
#define SPIRE_ARRAY_TABLE_MODEL_HPP
#include "Spire/Ui/TableModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a TableModel using an array as its backing data structure. */
  class ArrayTableModel : public TableModel {
    public:

      /** Constructs an empty ArrayTableModel. */
      ArrayTableModel();

      /**
       * Takes a callable function and invokes it. All operations performed on
       * this model during the transaction get appended to a
       * <code>Transaction</code> that is signalled at the end of the
       * transaction. If a transaction is already being invoked, then all
       * operations are appened into the parent transaction.
       * @param transaction The transaction to perform.
       * @return The result of the transaction.
       */
      template<typename F>
      decltype(auto) transact(F&& transaction);

      /**
       * Appends a row represented by a <code>std::vector</code>.
       * @param row The row to append to this model.
       * @throws <code>std::out_of_range</code> - This model is not empty and
       *         <code>row.size() != get_column_size()</code>.
       */
      void push(const std::vector<std::any>& row);

      /**
       * Inserts a row at a specified index.
       * @param row The row to insert.
       * @param index The index to insert the row at.
       * @throws <code>std::out_of_range</code> - This model is not empty and
       *         <code>model.get_column_size() != get_column_size()</code>.
       * @throws <code>std::out_of_range</code> -
       *         <code>index < 0 or index >= get_row_size()</code>.
       */
      void insert(const TableModel& model, int index);

      /**
       * Moves a row.
       * @param source - The index of the row to move.
       * @param destination - The index to move the row to.
       * @throws <code>std::out_of_range</code> - The source or destination are
       * not within this table's range.
       */
      void move(int source, int destination);

      /**
       * Removes a row from the table.
       * @param index - The index of the row to remove.
       * @throws <code>std::out_of_range</code> - The index is not within this
       *         table's range.
       */
      void remove(int index);

      /**
       * Sets the value at a specified row and column.
       * @param row - The row to set.
       * @param column - The column to set.
       * @param value - The value to set at the specified row and column.
       * @throws <code>std::out_of_range</code> - The row or column is not
       *         within this table's range.
       */
      void set(int row, int column, const std::any& value);

      int get_row_size() const override;

      int get_column_size() const override;

      const std::any& at(int row, int column) const override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;
  };

  template<typename F>
  decltype(auto) ArrayTableModel::transact(F&& transaction) {
  }
}

#endif
