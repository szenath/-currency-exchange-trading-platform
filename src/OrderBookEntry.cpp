#include "OrderBookEntry.h"
#include <stdexcept>

// Constructor
OrderBookEntry::OrderBookEntry(double _price, double _amount,
    const std::string& _timestamp,
    const std::string& _product,
    OrderBookType _orderType,
    const std::string& _username)
    : price(_price), amount(_amount), timestamp(_timestamp),
    product(_product), orderType(_orderType), username(_username) {
}

// Convert string to OrderBookType
OrderBookType OrderBookEntry::stringToOrderBookType(const std::string& s) {
    if (s == "BID" || s == "bid") return OrderBookType::bid;
    if (s == "ASK" || s == "ask") return OrderBookType::ask;
    if (s == "BIDSALE" || s == "bidsale") return OrderBookType::bidsale;
    if (s == "ASKSALE" || s == "asksale") return OrderBookType::asksale;
    return OrderBookType::unknown;
}