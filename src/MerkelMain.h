#pragma once

#include <vector>
#include <string>
#include "OrderBookEntry.h"
#include "OrderBook.h"
#include "Wallet.h"
#include "User.h"
#include "UserManager.h"
#include "TransactionManager.h"

class MerkelMain
{
    public:
        MerkelMain();
        /** Call this to start the sim */
        void init();
    private: 
        void printMenu();
        void printCandlesticks();
        void printCandlestickTable(const std::vector<Candlestick>& candles);
        void printHelp();
        void printMarketStats();
        void enterAsk();
        void enterBid();
        void printWallet();
        void gotoNextTimeframe();
        int getUserOption();
        void processUserOption(int userOption);
        void handleWalletUpdate();
        void simulateUserTrades();
        void deposit();
        void withdraw();
        void printLast5Transactions();
        void printActivitySummary();
        void handlePasswordReset();

        std::string currentTime;
        std::string getSystemTimestamp();

//        OrderBook orderBook{"20200317.csv"};
	OrderBook orderBook{"20200601.csv"};
        Wallet wallet;

        UserManager userManager{ "users.csv" };

        User currentUser;
        Wallet currentWallet;
        bool loggedIn = false;
        bool isLoggedIn = false;
        TransactionManager transactionManager;

        void showLoginMenu();
        void handleLogin();
        void handleRegister();



};
