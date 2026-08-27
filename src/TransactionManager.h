


#pragma once
#include "Transaction.h"
#include "Wallet.h"           //  needed for buildWalletForUser()
#include <vector>
#include <string>
#include <map>
#include "CSVReader.h"
#include "ActivitySummary.h"

class TransactionManager
{
public:
    TransactionManager(std::string filename);

    // Transaction logging
    void logTransaction(const Transaction& t);
    
    // Query transactions
    std::vector<Transaction> getLastNTransactions(int n);
    std::vector<Transaction> getTransactionsByProduct(const std::string& product);
    void printLastNForUser(const std::string& userId, int n);

    // Activity summaries
    ActivitySummary getUserActivitySummary(const std::string& userId);
    
    //  Wallet reconstruction from CSV
    Wallet buildWalletForUser(const std::string& userId) const;
    
    //  Timeframe filtering for money spent
    std::map<std::string, double> getMoneySpentByCurrencyForUserInTimeframe(
        const std::string& userId,
        const std::string& timeframe,
        const std::string& timeframeKey) const;

private:
    std::string fileName;
    std::vector<Transaction> transactions;  // in-memory transaction storage
    
    //  Load transactions from CSV on startup
    void loadTransactionsFromFile();
    
    //  Helper for timeframe truncation
   std::string truncateTimestamp(const std::string& timestamp, const std::string& timeframe) const;

};

