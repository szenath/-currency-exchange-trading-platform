

#include "TransactionManager.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

TransactionManager::TransactionManager(std::string filename)
    : fileName(std::move(filename))
{
    loadTransactionsFromFile();
}

void TransactionManager::loadTransactionsFromFile()
{
    transactions.clear();

    std::ifstream file(fileName);
    if (!file.is_open())
    {
        // First run: file may not exist yet.
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::stringstream ss(line);
        Transaction t;

        // Expected format:
        // timestamp,type,product,price,amount,walletBalance,userId
        std::getline(ss, t.timestamp, ',');
        std::getline(ss, t.type, ',');
        std::getline(ss, t.product, ',');

        std::string priceStr, amtStr, wbStr;
        std::getline(ss, priceStr, ',');
        std::getline(ss, amtStr, ',');
        std::getline(ss, wbStr, ',');
        std::getline(ss, t.userId);

        if (t.timestamp.empty() || t.type.empty() || t.product.empty())
            continue;

        try
        {
            t.price = std::stod(priceStr);
            t.amount = std::stod(amtStr);
            t.walletBalance = std::stod(wbStr);
        }
        catch (...) {
            continue;
        }

        // Trim CRLF from userId if needed
        while (!t.userId.empty() && (t.userId.back() == '\r' || t.userId.back() == '\n'))
            t.userId.pop_back();

        transactions.push_back(t);
    }
}

void TransactionManager::logTransaction(const Transaction& t)
{
    // Keep in memory so this session reflects new transactions immediately
    transactions.push_back(t);

    std::ofstream file(fileName, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open transaction file for writing: " << fileName << "\n";
        return;
    }

    file << t.timestamp << ","
         << t.type << ","
         << t.product << ","
         << t.price << ","
         << t.amount << ","
         << t.walletBalance << ","
         << t.userId
         << "\n";
}

std::vector<Transaction> TransactionManager::getLastNTransactions(int n)
{
    if (n <= 0) return {};
    if ((int)transactions.size() <= n) return transactions;
    return std::vector<Transaction>(transactions.end() - n, transactions.end());
}

std::vector<Transaction> TransactionManager::getTransactionsByProduct(const std::string& product)
{
    std::vector<Transaction> result;
    for (const auto& t : transactions)
    {
        if (t.product == product)
            result.push_back(t);
    }
    return result;
}

void TransactionManager::printLastNForUser(const std::string& userId, int n)
{
    std::vector<Transaction> userTxns;
    userTxns.reserve(transactions.size());

    for (const auto& t : transactions)
        if (t.userId == userId)
            userTxns.push_back(t);

    if (userTxns.empty())
    {
        std::cout << "\nNo active transactions to show yet.\n";
        return;
    }

    int start = std::max(0, (int)userTxns.size() - n);

    for (int i = start; i < (int)userTxns.size(); ++i)
    {
        const auto& tx = userTxns[i];
        std::cout << tx.timestamp << " "
                  << tx.type << " "
                  << tx.product << " "
                  << tx.amount << " @ "
                  << tx.price
                  << " | Wallet: "
                  << tx.walletBalance << "\n";
    }
}

ActivitySummary TransactionManager::getUserActivitySummary(const std::string& userId)
{
    ActivitySummary summary;

    for (const Transaction& txn : transactions)
    {
        if (txn.userId != userId)
            continue;

        // Determine quote currency from product e.g. ETH/USDT
        std::string quote = "";
        std::size_t slashPos = txn.product.find('/');
        if (slashPos != std::string::npos && slashPos + 1 < txn.product.size())
            quote = txn.product.substr(slashPos + 1);

        auto& stats = summary.byProduct[txn.product];
        stats.quoteCurrency = quote;

        // Count placed orders as BID/ASK.
        // Executed trades are logged separately as BUY/SELL (used for money spent + wallet rebuild).
        if (txn.type == "ASK" || txn.type == "Ask")
        {
            stats.asks++;
            summary.totalAsks++;
        }
        else if (txn.type == "BID" || txn.type == "Bid")
        {
            stats.bids++;
            summary.totalBids++;
        }
        else if (txn.type == "BUY" || txn.type == "Buy")
        {
            // Money spent is based on executed BUY trades only.
            double spent = txn.price * txn.amount;
            stats.moneySpent += spent;

            if (!quote.empty())
                summary.totalMoneySpentByCurrency[quote] += spent;
        }
    }
    return summary;
}

Wallet TransactionManager::buildWalletForUser(const std::string& userId) const
{
    Wallet w;

    // Apply user transactions in timestamp order. The format you use sorts correctly as string.
    std::vector<Transaction> userTxns;
    userTxns.reserve(transactions.size());

    for (const auto& t : transactions)
        if (t.userId == userId)
            userTxns.push_back(t);

    std::sort(userTxns.begin(), userTxns.end(),
              [](const Transaction& a, const Transaction& b) { return a.timestamp < b.timestamp; });

    for (const auto& t : userTxns)
    {
        if (t.type == "Deposit")
        {
            // For deposits/withdrawals, your code uses product field as currency name
            w.currencies[t.product] += std::fabs(t.amount);
        }
        else if (t.type == "Withdraw")
        {
            w.currencies[t.product] -= std::fabs(t.amount);
        }
        else if (t.type == "BUY" || t.type == "Buy")
        {
            std::size_t slashPos = t.product.find('/');
            if (slashPos == std::string::npos) continue;

            std::string base = t.product.substr(0, slashPos);
            std::string quote = t.product.substr(slashPos + 1);

            double cost = t.price * t.amount;
            w.currencies[quote] -= cost;
            w.currencies[base] += t.amount;
        }
        else if (t.type == "SELL" || t.type == "Sell")
        {
            std::size_t slashPos = t.product.find('/');
            if (slashPos == std::string::npos) continue;

            std::string base = t.product.substr(0, slashPos);
            std::string quote = t.product.substr(slashPos + 1);

            double proceeds = t.price * t.amount;
            w.currencies[base] -= t.amount;
            w.currencies[quote] += proceeds;
        }
    }

    // Clean very small values
    for (auto& kv : w.currencies)
        if (std::fabs(kv.second) < 1e-12) kv.second = 0.0;

    return w;
}
std::string TransactionManager::truncateTimestamp(const std::string& timestamp, const std::string& timeframe) const

{
    // timestamp format in your CSV: "YYYY/MM/DD HH:MM:SS.ffffff"
    if (timeframe == "hourly")
    {
        return timestamp.size() >= 13 ? timestamp.substr(0, 13) : timestamp;
    }
    if (timeframe == "minutely")
    {
        return timestamp.size() >= 16 ? timestamp.substr(0, 16) : timestamp;
    }
    if (timeframe == "secondly")
    {
        return timestamp.size() >= 19 ? timestamp.substr(0, 19) : timestamp;
    }
    return timestamp;
}

std::map<std::string, double> TransactionManager::getMoneySpentByCurrencyForUserInTimeframe(
    const std::string& userId,
    const std::string& timeframe,
    const std::string& timeframeKey) const
{
    std::map<std::string, double> spentByCurrency;

    for (const auto& txn : transactions)
    {
        if (txn.userId != userId) continue;
        if (!(txn.type == "BUY" || txn.type == "Buy")) continue;

        if (truncateTimestamp(txn.timestamp, timeframe) != timeframeKey)
            continue;

        std::size_t slashPos = txn.product.find('/');
        if (slashPos == std::string::npos) continue;

        std::string quote = txn.product.substr(slashPos + 1);
        spentByCurrency[quote] += txn.price * txn.amount;
    }

    return spentByCurrency;
}

