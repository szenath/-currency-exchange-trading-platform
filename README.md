# Currency Exchange Trading Platform

A C++ technical analysis and trading simulation toolkit for a currency exchange platform. The system supports secure user authentication, persistent wallet management, order placement with a matching engine, transaction history tracking, and market visualisation through candlestick data (hourly, minutely, and secondly timeframes).

## Source Code

All application source code (`.cpp` and `.h` files) is organised by class, as listed below.

## Features

- **User authentication** — registration, login, and password reset with hashed passwords, auto-generated 10-digit user IDs, email validation, and 90-day password expiry
- **Persistent wallets** — balances linked to individual users, reconstructed deterministically by replaying transaction history on login
- **Order book & matching engine** — bid/ask order placement with price-time priority matching and partial fills
- **Candlestick data** — OHLC (Open, High, Low, Close) generation per product, switchable between hourly, minutely, and secondly timeframes
- **Transaction history** — full deposit/withdrawal/trade logging to CSV, with last-5-transaction view and all-time activity summaries
- **Simulated trading** — generates realistic buy/sell orders using the latest market mid-price, ±0.5% random variation, and a 0.2% bid-ask spread, replayed against historical market time (not system clock)
- **Robust input validation** — all menu options, order entries, and wallet operations validate input and recover gracefully without crashing

## Class Structure

| Class | Files | Responsibility |
|---|---|---|
| Main Controller | `MerkelMain.cpp/.h` | Application controller, menu handling, user interaction |
| Market Data | `OrderBook.cpp/.h` | Order management, matching engine, candlestick generation |
| Market Data | `CSVReader.cpp/.h` | CSV file parsing and tokenisation |
| Wallet | `Wallet.cpp/.h` | Currency balance management, deposit/withdrawal |
| Candlestick | `Candlestick.cpp/.h` | Computes OHLC data by timeframe |
| Users | `User.cpp/.h` | User data model with authentication credentials |
| UserManager | `UserManager.cpp/.h` | User registration, login, password management |
| Transaction | `Transaction.cpp/.h` | Transaction data model |
| TransactionManager | `TransactionManager.cpp/.h` | Transaction logging, wallet reconstruction, activity summaries |

## Data Persistence

| File | Purpose |
|---|---|
| `20200601.csv` | Historical exchange order book data (read-only) |
| `users.csv` | User credentials and profile information |
| `transactions.csv` | Full transaction history (deposits, withdrawals, trades) |

Wallet state is never stored directly — it's rebuilt on each login by replaying every transaction for that user in order, ensuring balances are always deterministic and consistent with the transaction log.

## How It Works

1. On startup, `MerkelMain` loads the order book, user credentials, and transaction manager into memory
2. The user is shown a login menu and must register or log in before continuing
3. After login, the wallet is reconstructed from that user's transaction history
4. The main menu loop lets the user place bids/asks, advance market time, view candlesticks, manage their wallet, and view activity summaries

## Market Time vs. System Time

The order book's historical data is from 2020, while the application runs years later. Rather than using system clock time (which would break matching against historical orders), the application tracks its own **market time**, advancing through timestamps already present in the CSV data. New orders are timestamped with the current *market* time, not the real-world clock, keeping matching, candlestick generation, and activity summaries fully reproducible.

## Simulated Trading

To generate realistic prices without relying on outdated 2020 data, simulated trades are priced as:centerPrice = referencePrice × (1 + randomMove) // randomMove: ±0.5%
bidPrice = centerPrice × (1 - spread) // spread: 0.2%
askPrice = centerPrice × (1 + spread)


`referencePrice` is the latest mid-price pulled from the order book, so simulated trades stay grounded in real historical market data while reflecting realistic day-to-day price movement and bid-ask spread.

## Design Highlights

- **No manual memory management** — all classes use `std::string`, `std::vector`, `std::map`, and value semantics (RAII, no leaks or dangling pointers)
- **Separation of concerns** — UI (`MerkelMain`), authentication (`UserManager`, `User`), market data (`OrderBook`, `Candlestick`), balances (`Wallet`), and persistence (`TransactionManager`, `CSVReader`) are each isolated
- **Reusable utilities** — shared helpers like `CSVReader::tokenise()`, timestamp formatting, and order comparators are used consistently across the codebase
- **Error handling** — all user input is validated with try/catch and bounds checks; invalid input never crashes the app, only prompts the user to retry
