-- Ensure foreign key constraint is enabled.
-- This is the file that will generate a SQLite3 database for personal finance tracking
-- I'm not sure what the best method is currently for storing the category info for transactions


-- This holds the transaction types
CREATE TABLE IF NOT EXISTS months (
	no integer, -- 1 - 12
	name text, -- actual month name
	abv char(3), -- abbreviation of the month
	CHECK ( no < 13 AND no > 0 ),
	PRIMARY KEY (no, name, abv)
);

-- Populate valid months
BEGIN;
	INSERT INTO months VALUES 
	(1, 'JANUARY', 'JAN'), (2, 'FEBRUARY', 'FEB'), (3, 'MARCH', 'MAR'), (4, 'APRIL', 'APR'), (5, 'MAY', 'MAY'), (6, 'JUNE', 'JUN'), 
	(7, 'JULY', 'JUL'), (8, 'AUGUST', 'AUG'), (9, 'SEPTEMBER', 'SEP'), (10, 'OCTOBER', 'OCT'), (11, 'NOVEMBER', 'NOV'),(12, 'DECEMBER', 'DEC') ;
COMMIT;

-- Viable days in a given month
CREATE TABLE IF NOT EXISTS days (
	month integer, -- 1 - 12, cannot be primary key as there's no way to map this with a unique value here
	day integer, -- one per day in the month
	CHECK ( month > 0 AND month < 13),
	CHECK ( day > 0 AND day < 32),
	FOREIGN KEY (month) REFERENCES months(no)
);

-- Transaction types
CREATE TABLE IF NOT EXISTS xtypes (
	key integer, -- The unique id of a type
	type text, -- Unique human-friendly transaction type
	CHECK ( key >= 0 ),
	PRIMARY KEY (key, type)
);

-- Transaction categories
CREATE TABLE IF NOT EXISTS xcats (
	key integer, -- The unique id used internally
	cat text, -- Human-friendly version
	CHECK ( key >= 0 ),
	PRIMARY KEY (key, cat)
);

-- Create the main table
-- This could use a date value, but it would require more
-- work to properly request date bounded information.
CREATE TABLE IF NOT EXISTS transactions (
	tid varchar(64) UNIQUE NOT NULL, -- Should be a unique, generally incrementing value
	year int, -- Should be obvious, year of the transaction
	month integer, -- Restrict month data to valid months
	day integer, -- Tinyint to try using a byte, as this should never be above 31, hopefully foreign key constraint can do this
	-- Need to examine what's going wrong with foreign key constraints
	type integer, -- Transaction type
	amount numeric, -- Amount paid/recieved
	category integer, -- The category of the transaction
	desc text NOT NULL, -- Description of the transaction, default determined by other fields prior to being inserted
	-- These constraints do not appear to work as desired yet
	-- more work needed to ensure they work properly
	CHECK ( year > 0 ),
	CHECK ( month > 0 AND month < 13 ),
	CHECK ( day > 0 AND day < 32),
	PRIMARY KEY (tid),
	FOREIGN KEY (month) REFERENCES months(no)
);


-- Populate valid days in each given month
-- JAN
BEGIN;
	INSERT INTO days VALUES
	(1, 1), (1, 2), (1, 3), (1, 4), (1, 5), (1, 6), (1, 7), (1, 8), (1, 9), (1, 10), (1, 11), (1, 12), (1, 13), (1, 14), (1, 15), (1, 16), (1, 17), (1, 18), (1, 19), (1, 20), (1, 21), (1, 22), (1, 23), (1, 24), (1, 25), (1, 26), (1, 27), (1, 28), (1, 29), (1, 30), (1, 31) ,
-- FEB
	(2, 1), (2, 2), (2, 3), (2, 4), (2, 5), (2, 6), (2, 7), (2, 8), (2, 9), (2, 10), (2, 11), (2, 12), (2, 13), (2, 14), (2, 15), (2, 16), (2, 17), (2, 18), (2, 19), (2, 20), (2, 21), (2, 22), (2, 23), (2, 24), (2, 25), (2, 26), (2, 27), (2, 28), (2, 29) ,
-- MAR
	(3, 1), (3, 2), (3, 3), (3, 4), (3, 5), (3, 6), (3, 7), (3, 8), (3, 9), (3, 10), (3, 11), (3, 12), (3, 13), (3, 14), (3, 15), (3, 16), (3, 17), (3, 18), (3, 19), (3, 20), (3, 21), (3, 22), (3, 23), (3, 24), (3, 25), (3, 26), (3, 27), (3, 28), (3, 29), (3, 30), (3, 31) ,
-- APR
	(4, 1), (4, 2), (4, 3), (4, 4), (4, 5), (4, 6), (4, 7), (4, 8), (4, 9), (4, 10), (4, 11), (4, 12), (4, 13), (4, 14), (4, 15), (4, 16), (4, 17), (4, 18), (4, 19), (4, 20), (4, 21), (4, 22), (4, 23), (4, 24), (4, 25), (4, 26), (4, 27), (4, 28), (4, 29), (4, 30) ,
-- MAY
	(5, 1), (5, 2), (5, 3), (5, 4), (5, 5), (5, 6), (5, 7), (5, 8), (5, 9), (5, 10), (5, 11), (5, 12), (5, 13), (5, 14), (5, 15), (5, 16), (5, 17), (5, 18), (5, 19), (5, 20), (5, 21), (5, 22), (5, 23), (5, 24), (5, 25), (5, 26), (5, 27), (5, 28), (5, 29), (5, 30), (5, 31) ,
-- JUN
	(6, 1), (6, 2), (6, 3), (6, 4), (6, 5), (6, 6), (6, 7), (6, 8), (6, 9), (6, 10), (6, 11), (6, 12), (6, 13), (6, 14), (6, 15), (6, 16), (6, 17), (6, 18), (6, 19), (6, 20), (6, 21), (6, 22), (6, 23), (6, 24), (6, 25), (6, 26), (6, 27), (6, 28), (6, 29), (6, 30) ,
-- JUL
	(7, 1), (7, 2), (7, 3), (7, 4), (7, 5), (7, 6), (7, 7), (7, 8), (7, 9), (7, 10), (7, 11), (7, 12), (7, 13), (7, 14), (7, 15), (7, 16), (7, 17), (7, 18), (7, 19), (7, 20), (7, 21), (7, 22), (7, 23), (7, 24), (7, 25), (7, 26), (7, 27), (7, 28), (7, 29), (7, 30), (7, 31) ,
-- AUG
	(8, 1), (8, 2), (8, 3), (8, 4), (8, 5), (8, 6), (8, 7), (8, 8), (8, 9), (8, 10), (8, 11), (8, 12), (8, 13), (8, 14), (8, 15), (8, 16), (8, 17), (8, 18), (8, 19), (8, 20), (8, 21), (8, 22), (8, 23), (8, 24), (8, 25), (8, 26), (8, 27), (8, 28), (8, 29), (8, 30), (8, 31) ,
-- SEP
	(9, 1), (9, 2), (9, 3), (9, 4), (9, 5), (9, 6), (9, 7), (9, 8), (9, 9), (9, 10), (9, 11), (9, 12), (9, 13), (9, 14), (9, 15), (9, 16), (9, 17), (9, 18), (9, 19), (9, 20), (9, 21), (9, 22), (9, 23), (9, 24), (9, 25), (9, 26), (9, 27), (9, 28), (9, 29), (9, 30) ,
-- OCT
	(10, 1), (10, 2), (10, 3), (10, 4), (10, 5), (10, 6), (10, 7), (10, 8), (10, 9), (10, 10), (10, 11), (10, 12), (10, 13), (10, 14), (10, 15), (10, 16), (10, 17), (10, 18), (10, 19), (10, 20), (10, 21), (10, 22), (10, 23), (10, 24), (10, 25), (10, 26), (10, 27), (10, 28), (10, 29), (10, 30), (10, 31) ,
-- NOV
	(11, 1), (11, 2), (11, 3), (11, 4), (11, 5), (11, 6), (11, 7), (11, 8), (11, 9), (11, 10), (11, 11), (11, 12), (11, 13), (11, 14), (11, 15), (11, 16), (11, 17), (11, 18), (11, 19), (11, 20), (11, 21), (11, 22), (11, 23), (11, 24), (11, 25), (11, 26), (11, 27), (11, 28), (11, 29), (11, 30) ,
-- DEC
	(12, 1), (12, 2), (12, 3), (12, 4), (12, 5), (12, 6), (12, 7), (12, 8), (12, 9), (12, 10), (12, 11), (12, 12), (12, 13), (12, 14), (12, 15), (12, 16), (12, 17), (12, 18), (12, 19), (12, 20), (12, 21), (12, 22), (12, 23), (12, 24), (12, 25), (12, 26), (12, 27), (12, 28), (12, 29), (12, 30), (12, 31) ;
COMMIT;
-- End populating months

-- Start Populating transaction data
-- Now the hard part, populating the various transaction types and categories
BEGIN;
	INSERT INTO xtypes VALUES
	(0, 'EXPENSE'), (1, 'DEPOSIT'), (2, 'INVOICE'), (3, 'INVESTMENT'), (4, 'SALARY'), (5, 'ADJUSTED');
COMMIT;

BEGIN;
	INSERT INTO xcats VALUES
	(0, 'CARPAYMENT'), (1, 'UTILITIES'), (2, 'DEBT'), (3, 'GROCERIES'), (4, 'ELECTRONICS'), (5, 'GAMES'), (6, 'MOVIES'), (7, 'FOOD'), (8, 'DATES'), (9, 'ALCOHOL'),
	(10, 'TRAVEL'), (11, 'EXILE'), (12, 'GUNS'), (13, 'GAS'), (14, 'GOLD'), (15, 'SILVER'), (16, 'STOCKS'), (17, 'RETIREMENT'), (18, 'EHI'), (19, 'PAID'), (20, 'ADJUST'), 
	(21, 'METALS'), (22, 'GEMS');
COMMIT;
-- End populating tables

-- Create some indexes for faster operations
-- Some may be redundant due to primary keys, but should be better to have than not
CREATE UNIQUE INDEX IF NOT EXISTS type_idx ON xtypes (key,type);
CREATE UNIQUE INDEX IF NOT EXISTS cat_idx ON xcats (key,cat);
CREATE UNIQUE INDEX IF NOT EXISTS months_idx ON months (no,name,abv);
CREATE INDEX IF NOT EXISTS trans_types ON transactions (tid,type,amount,desc);
CREATE INDEX IF NOT EXISTS trans_cats ON transactions (tid,category,amount,desc);
CREATE INDEX IF NOT EXISTS trans_by_year ON transactions (tid,year,amount,desc);
CREATE INDEX IF NOT EXISTS trans_by_month ON transactions (tid,month,amount,desc);

-- PRAGMA foreign_keys = ON;
-- NOTE: Later versions should make it possible to encrypt or hash this data on-disk so it's not possible to determine exactly what rows mean anything

-- Reminders on how to collect certain types of data
-- Balance according to tracked data:
--	select (select sum(amount) from transactions where type=4) - (select sum(amount) from transactions where type<>4) as balance;
-- Category by name:
--	select sum(tx.amount) from transactions as tx where category=(select key from xcats where cat='PAID');
-- Type by name:
--	select sum(amount) from transactions where type=(select key from xtypes where type='EXPENSE');
