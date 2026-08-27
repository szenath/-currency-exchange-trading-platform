#pragma once
#include <string>

enum class OrderBookType { bid, ask, unknown, asksale, bidsale };

class OrderBookEntry {
public:
    double price;
    double amount;
    std::string timestamp;
    std::string product;
    OrderBookType orderType;
    std::string username;

    // Constructor
    OrderBookEntry(double _price, double _amount,
        const std::string& _timestamp,
        const std::string& _product,
        OrderBookType _orderType,
        const std::string& _username = "dataset");

    // Convert string to OrderBookType
    static OrderBookType stringToOrderBookType(const std::string& s);

    // Sorting helpers
    static bool compareByTimestamp(const OrderBookEntry& e1, const OrderBookEntry& e2) {
        return e1.timestamp < e2.timestamp;
    }
    static bool compareByPriceAsc(const OrderBookEntry& e1, const OrderBookEntry& e2) {
        return e1.price < e2.price;
    }
    static bool compareByPriceDesc(const OrderBookEntry& e1, const OrderBookEntry& e2) {
        return e1.price > e2.price;
    }

    // Convert OrderBookType to string
    static std::string orderBookTypeToString(OrderBookType type) {
        switch (type) {
        case OrderBookType::ask: return "Ask";
        case OrderBookType::bid: return "Bid";
        case OrderBookType::asksale: return "AskSale";
        case OrderBookType::bidsale: return "BidSale";
        default: return "Unknown";
        }
    }
};