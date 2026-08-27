#pragma once
#include <string>
#include <map>
#include <iostream>
#include <sstream>      // <-- add this for std::ostringstream
#include "OrderBookEntry.h"
#include "CSVReader.h"  // for tokenise()

class Wallet {
public:
    Wallet() = default;

    // Store balances
    std::map<std::string, double> currencies;

    // Deposit / Withdraw
    void deposit(const std::string& currency, double amount);
    bool withdraw(const std::string& currency, double amount);

    // Insert / Remove currency
    void insertCurrency(const std::string& type, double amount);
    bool removeCurrency(const std::string& type, double amount);

    // Query
    bool containsCurrency(const std::string& type, double amount) const;
    double getBalance(const std::string& currency) const;

    // Order processing
    bool canFulfillOrder(const OrderBookEntry& order) const;
    void processSale(const OrderBookEntry& sale);

    // String representation
    std::string toString() const {
        std::ostringstream os;   // <--- fixed
        for (auto& [currency, amount] : currencies) {
            os << currency << ":" << amount << "\n";
        }
        return os.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Wallet& wallet) {
        os << wallet.toString();
        return os;
    }
};