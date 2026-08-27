#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

struct ProductActivity
{
    int asks = 0;                 // number of sell actions/trades for this product
    int bids = 0;                 // number of buy actions/trades for this product
    double moneySpent = 0.0;     // money spent for this product (quote currency)
    std::string quoteCurrency;    // e.g., BTC, USDT
};

// Overall summary for a user for the current run / loaded history
struct ActivitySummary
{
    int totalAsks = 0;
    int totalBids = 0;

    // map product -> stats
    std::unordered_map<std::string, ProductActivity> byProduct;

    // total money spent by quote currency (BTC, USDT, etc.)
    std::map<std::string, double> totalMoneySpentByCurrency;
};