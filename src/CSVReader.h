#pragma once

#include "OrderBookEntry.h"
#include <vector>
#include <string>

class CSVReader
{
public:
    CSVReader();

    // Existing functions
    static std::vector<OrderBookEntry> readCSV(std::string csvFile);
    static std::vector<std::string> tokenise(std::string csvLine, char separator);

    static OrderBookEntry stringsToOBE(std::string price,
        std::string amount,
        std::string timestamp,
        std::string product,
        OrderBookType OrderBookType);

    // New helper functions to get base and quote currencies
    static std::string getBaseCurrency(const std::string& product)
    {
        auto pos = product.find('/');
        if (pos != std::string::npos)
            return product.substr(0, pos); // everything before '/'
        return "";
    }
   // Helper: product "ETH/BTC" -> quote "BTC"
    static std::string getQuoteCurrency(const std::string& product)
    {
        auto pos = product.find('/');
        if (pos != std::string::npos)
            return product.substr(pos + 1); // everything after '/'
        return "";
    }

private:
// Converts a vector of tokens from a CSV row into an OrderBookEntry.
    // Expected tokens:
    static OrderBookEntry stringsToOBE(std::vector<std::string> strings);
};