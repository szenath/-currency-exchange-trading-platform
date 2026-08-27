#include "TransactionLogger.h"
#include <fstream>

void TransactionLogger::log(const OrderBookEntry& sale,
    const Wallet& wallet,
    const std::string& action)
{
    std::ofstream file("transactions.csv", std::ios::app);
    if (!file.is_open()) return;

    file << sale.product << ","
        << sale.amount << ","
        << sale.price << ","
        << OrderBookEntry::orderBookTypeToString(sale.orderType) << ","
        << wallet.toString() << ","
        << action
        << std::endl;
}