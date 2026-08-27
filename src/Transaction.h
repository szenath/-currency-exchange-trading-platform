#pragma once
#include <string>

class Transaction
{
public:
    std::string timestamp;
    std::string type;        // BUY / SELL / DEPOSIT / WITHDRAW
    std::string product;
    double price;
    double amount;
    double walletBalance;
    std::string userId;      //  REQUIRED for per-user isolation

    Transaction() = default;

    Transaction(std::string ts,
        std::string t,
        std::string p,
        double pr,
        double amt,
        double wb,
        std::string uid);
};