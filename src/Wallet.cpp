#include "Wallet.h"
#include <stdexcept>
#include <iostream>

// Deposit currency
void Wallet::deposit(const std::string& currency, double amount) {
    if (amount <= 0) throw std::runtime_error("Invalid deposit amount");
    currencies[currency] += amount;
}

// Withdraw currency
bool Wallet::withdraw(const std::string& currency, double amount) {
    if (amount <= 0) return false;
    if (!containsCurrency(currency, amount)) return false;
    currencies[currency] -= amount;
    return true;
}

// Insert currency (like deposit)
void Wallet::insertCurrency(const std::string& type, double amount) {
    if (amount < 0) throw std::runtime_error("Cannot insert negative amount");
    currencies[type] += amount;
}

// Remove currency (like withdraw)
bool Wallet::removeCurrency(const std::string& type, double amount) {
    if (amount < 0) return false;
    if (!containsCurrency(type, amount)) return false;
    currencies[type] -= amount;
    return true;
}

// Check balance
bool Wallet::containsCurrency(const std::string& type, double amount) const {
    auto it = currencies.find(type);
    return it != currencies.end() && it->second >= amount;
}

// Get balance
double Wallet::getBalance(const std::string& currency) const {
    auto it = currencies.find(currency);
    return (it != currencies.end()) ? it->second : 0.0;
}

// Check if wallet can fulfill an order
bool Wallet::canFulfillOrder(const OrderBookEntry& order) const {
    std::vector<std::string> currs = CSVReader::tokenise(order.product, '/');
    if (order.orderType == OrderBookType::ask) {
        double amount = order.amount;
        std::string currency = currs[0];
        std::cout << "Checking wallet balance: need " << amount << " " << currency << std::endl;
        return containsCurrency(currs[0], order.amount);
    }
    else if (order.orderType == OrderBookType::bid) {
        double amount = order.amount * order.price;
        std::string currency = currs[1];
        std::cout << "Checking wallet balance: need " << amount << " " << currency << std::endl;
        return containsCurrency(currs[1], order.amount * order.price);
    }
    return false;
}

// Update wallet after order execution
void Wallet::processSale(const OrderBookEntry& sale) {
    std::vector<std::string> currs = CSVReader::tokenise(sale.product, '/');

    if (sale.orderType == OrderBookType::asksale) {
        double outgoingAmount = sale.amount;
        double incomingAmount = sale.amount * sale.price;
        currencies[currs[0]] -= outgoingAmount;
        currencies[currs[1]] += incomingAmount;
    }
    else if (sale.orderType == OrderBookType::bidsale) {
        double outgoingAmount = sale.amount * sale.price;
        double incomingAmount = sale.amount;
        currencies[currs[0]] += incomingAmount;
        currencies[currs[1]] -= outgoingAmount;
    }
}