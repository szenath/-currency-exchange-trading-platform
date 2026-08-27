#pragma once
#include "OrderBookEntry.h"
#include "Candlestick.h"
#include "CSVReader.h"
#include <string>
#include <vector>

class OrderBook
{
    public:
    /** construct, reading a csv data file */
        OrderBook(std::string filename);
    /** return vector of all know products in the dataset*/
        std::vector<std::string> getKnownProducts();
    /** return vector of Orders according to the sent filters*/
        std::vector<OrderBookEntry> getOrders(OrderBookType type, 
                                              std::string product, 
                                              std::string timestamp);

        /** returns the earliest time in the orderbook*/
        std::string getEarliestTime();
        /** returns the next time after the 
         * sent time in the orderbook  
         * If there is no next timestamp, wraps around to the start
         * */
        std::string getNextTime(std::string timestamp);

        void insertOrder(OrderBookEntry& order);

        std::vector<OrderBookEntry> matchAsksToBids(std::string product, std::string timestamp);

        static double getHighPrice(std::vector<OrderBookEntry>& orders);
        static double getLowPrice(std::vector<OrderBookEntry>& orders);

        //  Generates OHLC candlestick data for a product and order typ
        std::vector<Candlestick> getCandlesticks(std::string product,
            OrderBookType type,
            std::string timeframe);

        double getLatestMidPrice(const std::string& product);
        std::vector<OrderBookEntry> getOrdersForCurrentTime();
        
    private:
        std::vector<OrderBookEntry> orders;
        std::string currentTime;

        // Helper function to truncate timestamps based on timeframe.
        std::string truncateTimestamp(std::string timestamp,
            std::string timeframe);

};
