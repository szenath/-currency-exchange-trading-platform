#include "OrderBook.h"
#include "CSVReader.h"
#include <map>
#include <algorithm>
#include <iostream>


/** construct, reading a csv data file */
OrderBook::OrderBook(std::string filename)
{
    orders = CSVReader::readCSV(filename);
}
//Extract hourly, mintutely or secondly data from excel sheet 
std::string OrderBook::truncateTimestamp(std::string timestamp, std::string timeframe)
{
    if (timeframe == "hourly")
        return timestamp.substr(0, 13);     // YYYY-MM-DD HH
    if (timeframe == "minutely")
        return timestamp.substr(0, 16);     // YYYY-MM-DD HH:MM
    if (timeframe == "secondly")
        return timestamp.substr(0, 19);     // YYYY-MM-DD HH:MM:SS

    return timestamp.substr(0, 13); // default hourly
}

//Generates candlestick data (OHLC) for a given product and order type
std::vector<Candlestick> OrderBook::getCandlesticks(std::string product,
    OrderBookType type,
    std::string timeframe)
{
    std::map<std::string, std::vector<OrderBookEntry>> buckets;

    for (OrderBookEntry& e : orders)
    {
        if (e.product == product && e.orderType == type)
        {
            std::string key = truncateTimestamp(e.timestamp, timeframe);
            buckets[key].push_back(e);
        }
    }

    std::vector<Candlestick> candles;

// Convert each bucket into a Candlestick
    for (auto& pair : buckets)
    {
        auto& vec = pair.second;

        std::sort(vec.begin(), vec.end(),
            OrderBookEntry::compareByTimestamp);

        double open = vec.front().price;
        double close = vec.back().price;
        double high = open;
        double low = open;

        for (auto& e : vec)
        {
            high = std::max(high, e.price);
            low = std::min(low, e.price);
        }

        candles.emplace_back(pair.first, open, high, low, close);
    }

    return candles;
}

//Computes the latest mid-price for a product.
double OrderBook::getLatestMidPrice(const std::string& product)
{
    // Find the latest timestamp for this product by scanning orders.
    std::string latestTs;
    for (const auto& o : orders)
    {
        if (o.product == product)
        {
            if (latestTs.empty() || o.timestamp > latestTs)
                latestTs = o.timestamp;
        }
    }

    if (latestTs.empty())
        return 0.0;

    std::vector<OrderBookEntry> bids;
    std::vector<OrderBookEntry> asks;

    for (const auto& o : orders)
    {
        if (o.product == product && o.timestamp == latestTs)
        {
            if (o.orderType == OrderBookType::bid) bids.push_back(o);
            else if (o.orderType == OrderBookType::ask) asks.push_back(o);
        }
    }

    if (bids.empty() || asks.empty())
        return 0.0;

    double bestBid = OrderBook::getHighPrice(bids);
    double bestAsk = OrderBook::getLowPrice(asks);
    return (bestBid + bestAsk) / 2.0;
}



/** return vector of all know products in the dataset*/
std::vector<std::string> OrderBook::getKnownProducts()
{
    std::vector<std::string> products;

    std::map<std::string,bool> prodMap;

    for (OrderBookEntry& e : orders)
    {
        prodMap[e.product] = true;
    }
    
    // now flatten the map to a vector of strings
    for (auto const& e : prodMap)
    {
        products.push_back(e.first);
    }

    return products;
}
/** return vector of Orders according to the sent filters*/
std::vector<OrderBookEntry> OrderBook::getOrders(OrderBookType type, 
                                        std::string product, 
                                        std::string timestamp)
{
    std::vector<OrderBookEntry> orders_sub;
    for (OrderBookEntry& e : orders)
    {
        if (e.orderType == type && 
            e.product == product && 
            e.timestamp == timestamp )
            {
                orders_sub.push_back(e);
            }
    }
    return orders_sub;
}

//Utility: highest price in a list of orders
double OrderBook::getHighPrice(std::vector<OrderBookEntry>& orders)
{
    double max = orders[0].price;
    for (OrderBookEntry& e : orders)
    {
        if (e.price > max)max = e.price;
    }
    return max;
}

//Utility: lowest price in a list of orders
double OrderBook::getLowPrice(std::vector<OrderBookEntry>& orders)
{
    double min = orders[0].price;
    for (OrderBookEntry& e : orders)
    {
        if (e.price < min)min = e.price;
    }
    return min;
}

//Returns the earliest timestamp in the dataset
std::string OrderBook::getEarliestTime()
{
    return orders[0].timestamp;
}

//Returns the next timestamp after the given one.
std::string OrderBook::getNextTime(std::string timestamp)
{
    std::string next_timestamp = "";
    for (OrderBookEntry& e : orders)
    {
        if (e.timestamp > timestamp) 
        {
            next_timestamp = e.timestamp;
            break;
        }
    }
    if (next_timestamp == "")
    {
        next_timestamp = orders[0].timestamp;
    }
    return next_timestamp;
}

//Inserts a new order and keeps the order book sorted by timestamp.
void OrderBook::insertOrder(OrderBookEntry& order)
{
    orders.push_back(order);
    std::sort(orders.begin(), orders.end(), OrderBookEntry::compareByTimestamp);
}



//Matches asks and bids for a given product at a given timestamp.
std::vector<OrderBookEntry>
OrderBook::matchAsksToBids(std::string product, std::string timestamp)
{
    std::vector<OrderBookEntry> asks;
    std::vector<OrderBookEntry> bids;
    std::vector<OrderBookEntry> sales;

    // 1️Collect relevant orders
    for (OrderBookEntry& e : orders)
    {
        if (e.product == product && e.timestamp == timestamp)
        {
            if (e.orderType == OrderBookType::ask)
                asks.push_back(e);
            else if (e.orderType == OrderBookType::bid)
                bids.push_back(e);
        }
    }

    // 2️ Sort
    std::sort(asks.begin(), asks.end(), OrderBookEntry::compareByPriceAsc);
    std::sort(bids.begin(), bids.end(), OrderBookEntry::compareByPriceDesc);

    // 3 Match
    for (OrderBookEntry& ask : asks)
    {
        for (OrderBookEntry& bid : bids)
        {
            if (bid.price >= ask.price && bid.amount > 0 && ask.amount > 0)
            {
                double tradeAmount = std::min(bid.amount, ask.amount);
                double tradePrice = ask.price;

                //  ASK-side sale
                OrderBookEntry saleAsk(
                    tradePrice,          // price
                    tradeAmount,         // amount
                    timestamp,           // timestamp
                    product,             // product
                    OrderBookType::ask,  // order type
                    ask.username         // user
                );

                //  BID-side sale
                OrderBookEntry saleBid(
                    tradePrice,
                    tradeAmount,
                    timestamp,
                    product,
                    OrderBookType::bid,
                    bid.username
                );

                sales.push_back(saleAsk);
                sales.push_back(saleBid);

                bid.amount -= tradeAmount;
                ask.amount -= tradeAmount;
            }
        }
    }

    return sales;
}



std::vector<OrderBookEntry> OrderBook::getOrdersForCurrentTime()
{
    std::vector<OrderBookEntry> result;

    for (const OrderBookEntry& e : orders)
    {
        if (e.timestamp == currentTime)
        {
            result.push_back(e);
        }
    }
    return result;
}

