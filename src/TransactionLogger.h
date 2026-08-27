#pragma once
#include "OrderBookEntry.h"
#include "Wallet.h"
#include <string>

class TransactionLogger
{
public:
    static void log(const OrderBookEntry& sale,
                    const Wallet& wallet,
                    const std::string& userId);
};