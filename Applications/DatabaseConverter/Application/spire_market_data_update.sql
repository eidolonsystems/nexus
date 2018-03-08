START TRANSACTION
  ALTER TABLE order_imbalances MODIFY COLUMN price DOUBLE NOT NULL;
  ALTER TABLE order_imbalances MODIFY COLUMN size DOUBLE NOT NULL;
  UPDATE order_imbalances SET size = 1000000 * size;
COMMIT
START TRANSACTION
  ALTER TABLE bbo_quotes MODIFY COLUMN bid_price DOUBLE NOT NULL;
  ALTER TABLE bbo_quotes MODIFY COLUMN bid_size DOUBLE NOT NULL;
  UPDATE bbo_quotes SET bid_size = 1000000 * bid_size;
  ALTER TABLE bbo_quotes MODIFY COLUMN ask_price DOUBLE NOT NULL;
  ALTER TABLE bbo_quotes MODIFY COLUMN ask_size DOUBLE NOT NULL;
  UPDATE bbo_quotes SET ask_size = 1000000 * ask_size;
COMMIT
START TRANSACTION
  ALTER TABLE market_quotes MODIFY COLUMN bid_price DOUBLE NOT NULL;
  ALTER TABLE market_quotes MODIFY COLUMN bid_size DOUBLE NOT NULL;
  UPDATE market_quotes SET bid_size = 1000000 * bid_size;
  ALTER TABLE market_quotes MODIFY COLUMN ask_price DOUBLE NOT NULL;
  ALTER TABLE market_quotes MODIFY COLUMN ask_size DOUBLE NOT NULL;
  UPDATE market_quotes SET ask_size = 1000000 * ask_size;
COMMIT
START TRANSACTION
  ALTER TABLE book_quotes MODIFY COLUMN price DOUBLE NOT NULL;
  ALTER TABLE book_quotes MODIFY COLUMN size DOUBLE NOT NULL;
  UPDATE book_quotes SET size = 1000000 * size;
COMMIT
START TRANSACTION
  ALTER TABLE time_and_sales MODIFY COLUMN price DOUBLE NOT NULL;
  ALTER TABLE time_and_sales MODIFY COLUMN size DOUBLE NOT NULL;
  UPDATE time_and_sales SET size = 1000000 * size;
COMMIT
