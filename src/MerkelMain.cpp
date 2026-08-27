
#include "MerkelMain.h"
#include <iostream>
#include <vector>
#include "OrderBookEntry.h"
#include "CSVReader.h"
#include <limits>
#include "User.h"
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <conio.h> 
#include <thread> 
#include <regex>
#include <random>

MerkelMain::MerkelMain()
    : transactionManager("transactions.csv")
{
    isLoggedIn = false;
}

void MerkelMain::init()
{
    showLoginMenu();

    if (!isLoggedIn)
        return;
// Initialise simulation time at earliest dataset timestamp 
    currentTime = orderBook.getEarliestTime();
    
    //  Rebuild wallet from persisted transactions AFTER login
    wallet = transactionManager.buildWalletForUser(currentUser.userId);
    
    std::cout << "\n========================================\n";
    std::cout << " Wallet loaded from transaction history\n";
    std::cout << "========================================\n\n";
    std::cout << "Current wallet balances:\n";
    std::cout << wallet.toString();
    std::cout << "\nPress enter key to continue to main menu...";
    _getch();
    system("cls");

    // Main menu loop
    while (true)
    {
        printMenu();
        int choice = getUserOption();
        processUserOption(choice);
    }
}
// Helper functions to split product "BASE/QUOTE" into currencies.
std::string getBaseCurrency(const std::string& product)
{
    return product.substr(0, product.find('/'));
}

std::string getQuoteCurrency(const std::string& product)
{
    return product.substr(product.find('/') + 1);
}

// Login Menu UI
// Provides options:
//  1) Register
//  2) Login
//  3) Reset Password
//  4) Exit


void MerkelMain::showLoginMenu()
{
    while (!isLoggedIn) {
        std::system("cls");
        std::cout << R"( 
        
        
        
        
                               ******************************************************** 
                               *** Welcome to the Markel FX Trading Simulator !!!   *** 
                               ******************************************************** 
                                              
                                 1. Register 
                                 2. Login 
                                 3. Reset Password
                                 4. Exit 
                                 Choose option: )";

        int option;

        if (std::cin >> option) {
            if (option == 1) {
                handleRegister();
            }
            else if (option == 2) {
                handleLogin();
            }
            else if (option == 3)
            {
                handlePasswordReset();   
            }
            else if (option == 4) {
                exit(0);
            }
            else {
                std::cout << ">>> Invalid option - Valid Options (1/2/3/4) <<<< \n";
                _getch();
            }
        }
        else {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << ">>> Invalid option - please enter a number <<< \n";
            _getch();
        }
    }
}


// Password reset
void MerkelMain::handlePasswordReset()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string userId;
    std::string pwd1, pwd2;
    char ch;

    std::cout << "\nEnter User ID: ";
    std::getline(std::cin, userId);

    if (!userManager.userIdExists(userId))
    {
        std::cout << "User ID not found.\n";
        _getch();
        return;
    }
// Read password #1 masked with '*'
    std::cout << "Enter new password: ";
    while ((ch = _getch()) != '\r')
    {
        if (ch == '\b' && !pwd1.empty())
        {
            pwd1.pop_back();
            std::cout << "\b \b";
        }
        else if (ch != '\b')
        {
            pwd1.push_back(ch);
            std::cout << '*';
        }
    }
// Read password #2 masked with '*'
    std::cout << "\nRe-enter new password: ";
    while ((ch = _getch()) != '\r')
    {
        if (ch == '\b' && !pwd2.empty())
        {
            pwd2.pop_back();
            std::cout << "\b \b";
        }
        else if (ch != '\b')
        {
            pwd2.push_back(ch);
            std::cout << '*';
        }
    }
// Validate passwords match and are not empty
    if (pwd1 != pwd2 || pwd1.empty())
    {
        std::cout << "\nPasswords do not match.\n";
        _getch();
        return;
    }

    if (userManager.resetPassword(userId, pwd1))
        std::cout << "\n Password reset successful.\n";
    else
        std::cout << "\n Password reset failed.\n";

    _getch();
}

// Registration
void MerkelMain::handleRegister()
{
    std::string fullName;
    std::string email;
    std::string password;
    std::string confirmPassword;
    char ch;

 // Clear any leftover newline from previous numeric input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\n\n\n\n\n";
    std::cout << std::setw(60) << "\t\t\t\t\t     **************************************************************" << "\n";

    std::cout << std::setw(40) << "\t\t\t\t\t     Enter Name                                     : ";
    std::getline(std::cin, fullName);

    std::regex pattern(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    while (true) {
        email.clear();
        std::cout << "\r" << std::setw(40) << "\t\t\t\t\t     Enter email address (Eg., username@gmail.com)  : " << std::flush;

        while ((ch = _getch()) != '\r') {
            if (ch == '\b') {
                if (!email.empty()) {
                    email.pop_back();
                    std::cout << "\b \b";
                }
            }
            else {
                email.push_back(ch);
                std::cout << ch;
            }
        }
          // Validate email format
        if (std::regex_match(email, pattern)) {
            break;
        }
        else {
            std::cout << "\r \t\t\t\t\t     Enter email address (Eg., username@gmail.com): "
                << "<Invalid Login, please re-enter as advised>" << std::flush;

            ch = _getch();

            // Clear line visually
            std::cout << "\r" << std::string(160, ' ') << "\r";
            std::cout << std::setw(40)
                << "\t\t\t\t\t     Enter email address (Eg., username@gmail.com)  : "
                << std::flush;
        }
    }
    // Password masked input
    std::cout << "\n";
    std::cout << std::setw(40) << "\t\t\t\t\t     Enter Password                                 : ";
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        }
        else {
            password.push_back(ch);
            std::cout << '*';
        }
    }

    // Confirm password masked input
    std::cout << "\n";
    std::cout << std::setw(40) << "\t\t\t\t\t     Confirm Password                               : ";
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!confirmPassword.empty()) {
                confirmPassword.pop_back();
                std::cout << "\b \b";
            }
        }
        else {
            confirmPassword.push_back(ch);
            std::cout << '*';
        }
    }

    std::cout << "\n" << std::setw(60) << "\t\t\t\t\t     **************************************************************" << "\n";

    if (password != confirmPassword) {
        std::cout << "\nPasswords do not match. Registration failed.\n";
        _getch();
        system("cls");
        return;
    }

    if (fullName.empty() || email.empty() || password.empty()) {
        std::cout << "\nAll fields are required. Registration failed.\n";
        _getch();
        system("cls");
        return;
    }

    // Optional: show password (user choice)
    char choice;
    std::cout << "\nShow password in plain text? (Y/N): ";
    std::cin >> choice;

    if (choice == 'Y' || choice == 'y') {
        std::cout << "Password: " << password << std::endl;
    }
    else {
        std::cout << "Password kept hidden." << std::endl;
    }

       // Create user object (userId assigned inside UserManager)
    User newUser("", fullName, email, 0, std::time(nullptr));

    if (userManager.registerUser(newUser, password))
    {
        std::cout << "\n Registration successful!\n";
        std::cout << "\nYour User ID is: " << "<<< " << newUser.userId << " >>>" << "\n";
        std::cout << "\nSelect Option (2) from the menu to logon and continue Trading...  >>>" << "\n";
        std::cout << "\nPress enter key to continue...";
        std::cin.get();
        std::cin.get();
        system("cls");
    }
    else
    {
        std::cout << " User already exists or registration failed.\n";
        std::cout << "Press enter key to continue...";
        std::cin.get();
        std::cin.get();
        system("cls");
    }
}
// Login
void MerkelMain::handleLogin()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string userId;
    std::string password;
    char ch;

    std::cout << "\n\n\n\n\n";
    std::cout << std::setw(60) << "\t\t\t\t\t     **************************************************************" << "\n";
    std::cout << std::setw(40) << "\t\t\t\t\t     Enter User ID:                           : ";

    std::getline(std::cin, userId);

    if (userId.empty())
    {
        std::cout << "User ID cannot be empty.\n";
        std::cout << "\nPress any key to continue...";
        std::cin.get();
        system("cls");
        return;
    }

    std::cout << std::setw(40) << "\t\t\t\t\t     Enter password                           : ";

    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        }
        else {
            password.push_back(ch);
            std::cout << '*';
        }
    }

    if (password.empty())
    {
        std::cout << "Password cannot be empty.\n";
        std::cout << "\nPress enter key to continue...";
        std::cin.get();
        system("cls");
        return;
    }

    // Prepare credentials for validation
    User user;
    user.userId = userId;
    user.passwordHash = std::hash<std::string>{}(password);

    if (userManager.loginUser(user))
    {
        currentUser = user;
        isLoggedIn = true;

        //  Rebuild wallet from transactions.csv immediately after login
        wallet = transactionManager.buildWalletForUser(currentUser.userId);

        std::time_t now = std::time(nullptr);
        int daysSinceChange = (now - currentUser.passwordLastUpdated) / (60 * 60 * 24);
        int expiryDays = 90;
        int warnDays = 7;

        system("cls");
        std::cout << "\n Login successful. Welcome <<< "
            << currentUser.fullName << " >>>"
            << "\n********************************\n\n";

        if (daysSinceChange >= expiryDays)
        {
            std::cout << " Your password has expired! Please reset it immediately.\n";
            std::cout << "\nPress any key to continue...";
            std::cin.get();
            system("cls");
        }
        else if (daysSinceChange >= expiryDays - warnDays)
        {
            std::cout << " Reminder: Your password will expire in "
                << (expiryDays - daysSinceChange)
                << " day(s). Please consider changing it.\n";
        }

        std::cout << "Last password update: "
            << std::ctime(&currentUser.passwordLastUpdated);
    }
    else
    {
        std::cout << "\n\n Invalid User ID or password entered.......\n";
        std::cout << "\nPress enter key to to continue...";
        std::cin.get();
        system("cls");
    }
}

// Candlestick UI:
void MerkelMain::printCandlesticks()
{
    std::string product;
    
    while (true)
    {
        std::cout << "Enter product (e.g. ETH/USDT): ";
        std::getline(std::cin, product);

        if (!product.empty())
            break;

        std::cout << "Product cannot be empty. Please try again.\n";
    }

    std::string timeframe = "hourly";

    while (true)
    {
        auto askCandles = orderBook.getCandlesticks(
            product,
            OrderBookType::ask,
            timeframe
        );

        std::cout << "\n===== ASK Candlesticks (" << timeframe << ") =====\n";
        printCandlestickTable(askCandles);

        auto bidCandles = orderBook.getCandlesticks(
            product,
            OrderBookType::bid,
            timeframe
        );

        std::cout << "\n===== BID Candlesticks (" << timeframe << ") =====\n";
        printCandlestickTable(bidCandles);

        std::cout << "\nChange timeframe?\n";
        std::cout << "1) Hourly\n";
        std::cout << "2) Minutely\n";
        std::cout << "3) Secondly\n";
        std::cout << "0) Back to menu\n";
        std::cout << "Choice(1,2,3): ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "0")
            break;
        else if (choice == "1")
            timeframe = "hourly";
        else if (choice == "2")
            timeframe = "minutely";
        else if (choice == "3")
            timeframe = "secondly";
        else
            std::cout << "Invalid choice. Staying on current timeframe.\n";
    }
}

// Prints candlestick OHLC table to console

void MerkelMain::printCandlestickTable(const std::vector<Candlestick>& candles)
{
    if (candles.empty())
    {
        std::cout << "No data available.\n";
        return;
    }

    std::cout << "Time\t\t\tOpen\t\tHigh\t\tLow\t\tClose\n";

    for (const auto& c : candles)
    {
        std::cout << c.time << "\t\t"
            << c.open << "\t\t"
            << c.high << "\t\t"
            << c.low << "\t\t"
            << c.close << "\n";
    }
}

// Main menu options display

void MerkelMain::printMenu()
{
    std::cout << "\n*************************************** \n" << std::endl;
    std::cout << "1: Print help " << std::endl;
    std::cout << "2: Print exchange stats" << std::endl;
    std::cout << "3: Make an offer " << std::endl;
    std::cout << "4: Make a bid " << std::endl;
    std::cout << "5: Print wallet " << std::endl;
    std::cout << "6: Print candlesticks " << std::endl;
    std::cout << "7: Deposit/Withdraw Wallet\n";
    std::cout << "8: Show Last 5 Transactions\n";
    std::cout << "9: Show Activity Summary\n";
    std::cout << "10: Simulate Trading Activity\n";
    std::cout << "11: Trigger Matching " << std::endl;
    std::cout << "12: Exit " << std::endl;

    std::cout << "============== " << std::endl;
    std::cout << "Current time is: " << currentTime << std::endl;
}

// Dispatches menu option to the correct handler.

void MerkelMain::processUserOption(int userOption)
{
    system("cls");

    switch (userOption)
    {
    case 1:
        printHelp();
        break;

    case 2:
        printMarketStats();
        break;

    case 3:
        enterAsk();
        break;

    case 4:
        enterBid();
        break;

    case 5:
        printWallet();
        break;

    case 6:
        printCandlesticks();
        break;

    case 7:
        handleWalletUpdate();
        break;

    case 8:
        transactionManager.printLastNForUser(currentUser.userId, 5);
        break;

    case 9:
    {
        //  Activity summary 
        ActivitySummary summary = transactionManager.getUserActivitySummary(currentUser.userId);

        std::cout << "\n========================================\n";
        std::cout << "         ACTIVITY SUMMARY\n";
        std::cout << "========================================\n\n";
        std::cout << "Total asks: " << summary.totalAsks << "\n";
        std::cout << "Total bids: " << summary.totalBids << "\n\n";

        // Display total money spent per quote currency
        if (!summary.totalMoneySpentByCurrency.empty())
        {
            std::cout << "Total money spent (all time):\n";
            for (const auto& [currency, amount] : summary.totalMoneySpentByCurrency)
            {
                std::cout << "  " << currency << ": " << amount << "\n";
            }
        }
        else
        {
            std::cout << "Total money spent: 0\n";
        }

                    

        std::cout << "\n========================================\n";
        std::cout << "      BREAKDOWN BY PRODUCT\n";
        std::cout << "========================================\n";
        for (const auto& [product, stats] : summary.byProduct)
    {
        if (stats.asks == 0 && stats.bids == 0)
            continue;

        std::cout << "\nProduct: " << product << "\n";
        if (stats.asks > 0)
            std::cout << "  Asks: " << stats.asks << "\n";
        if (stats.bids > 0)
            std::cout << "  Bids: " << stats.bids << "\n";

        std::cout << "  Money spent (" << stats.quoteCurrency << "): " 
                  << stats.moneySpent << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "Press Enter key to return to menu...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //std::cin.get();
    break;
}

case 10:
    simulateUserTrades();
    break;

case 11:
    gotoNextTimeframe();
    break;

case 12:
    std::cout << "\n\n\n\n\n\n\n\t\t\t\t\t<<<<<<<<<<<<<<<<<<<***>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "\n\t\t\t\t\tExchange Simulator shutdown in progress... " << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "\t\t\t\t\t....." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "\t\t\t\t\t.........." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n\t\t\t\t\tTrading Session closed, Thank you for Trading." << std::endl;
    std::cout << "\n\t\t\t\t\t<<<<<<<<<<<<<<<<<<<<<<***>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "\n\n\n\n\n";
    exit(0);
    break;

default:
    std::cout << "Invalid choice. Choose 1-12" << std::endl;
    break;
}
}
void MerkelMain::printHelp()
{
system("cls");
std::cout << "\n\n\n";

std::cout << "\t\t=========================================================\n";
std::cout << "\t\t              MARKEL FX TRADING SIMULATOR\n";
std::cout << "\t\t=========================================================\n\n";

std::cout << "\t\t MENU GUIDE & FEATURES\n";
std::cout << "\t\t---------------------------------------------------------\n\n";

std::cout << "\t\t[1] Help\n";
std::cout << "\t\t    Shows this guide explaining all features\n\n";

std::cout << "\t\t[2] Exchange Stats\n";
std::cout << "\t\t      View min, max, average prices for each product\n\n";

std::cout << "\t\t[3] Place Ask (Sell)\n";
std::cout << "\t\t      Place a SELL order after wallet balance checks\n\n";

std::cout << "\t\t[4] Place Bid (Buy)\n";
std::cout << "\t\t      Place a BUY order using available wallet funds\n\n";

std::cout << "\t\t[5] Print Wallet\n";
std::cout << "\t\t      Displays wallet balances (one currency per line)\n\n";

std::cout << "\t\t[6] Candlestick Data\n";
std::cout << "\t\t      View Open/High/Low/Close data (Hourly / Minutely / Secondly)\n\n";

std::cout << "\t\t[7] Deposit/Withdraw Wallet\n";
std::cout << "\t\t      Manage your wallet balance\n\n";

std::cout << "\t\t[8] Last 5 Transactions\n";
std::cout << "\t\t      Shows your most recent trades\n\n";

std::cout << "\t\t[9] Activity Summary\n";
std::cout << "\t\t      Displays bids & asks per product\n\n";

std::cout << "\t\t[10] Simulate Trading Activity\n";
std::cout << "\t\t       Create demo trades for all products\n\n";

std::cout << "\t\t[11] Trigger Matching\n";
std::cout << "\t\t       Move to Next Timeframe - Matches bids & asks\n\n";

std::cout << "\t\t[12] Exit\n";
std::cout << "\t\t       Safely logs out and closes the simulator\n\n";

std::cout << "\t\t SECURITY NOTES\n";
std::cout << "\t\t---------------------------------------------------------\n";
std::cout << "\t\t  Password expiry reminders are provided.\n";
std::cout << "\t\t  A login warning appears 7 days before expiry (Expires every 90 days).\n";

std::cout << "\t\t=========================================================\n";
std::cout << "\t\t        Press enter key to return to main menu...\n";
std::cout << "\t\t=========================================================\n";

_getch();
system("cls");
}
// Prints market statistics for ASK orders at the earliest timestamp.
void MerkelMain::printMarketStats()
{
system("cls");
std::string timestamp = orderBook.getEarliestTime();
auto products = orderBook.getKnownProducts();

std::cout << "\n========== EXCHANGE ASK STATISTICS ==========\n\n";

for (const std::string& product : products)
{
    auto asks = orderBook.getOrders(OrderBookType::ask, product, timestamp);

    if (asks.empty())
        continue;

    double maxAsk = OrderBook::getHighPrice(asks);
    double minAsk = OrderBook::getLowPrice(asks);

    std::cout << "Product: " << product << "\n";
    std::cout << "---------------------------------------------------------\n";

    std::cout << std::left
        << std::setw(20) << "Asks Seen"
        << std::setw(20) << "Max Ask"
        << std::setw(20) << "Min Ask"
        << "\n";

    std::cout << std::left
        << std::setw(20) << asks.size()
        << std::setw(20) << maxAsk
        << std::setw(20) << minAsk
        << "\n\n";
}

std::cout << "============================================\n";
std::cout << "Press enter key to return to menu...";
_getch();
}
// Place an ASK order (sell):
void MerkelMain::enterAsk()
{
std::cout << "Enter ask: product,price,amount (e.g. ETH/BTC,200,0.5)\n";
std::string input;
std::getline(std::cin, input);
auto tokens = CSVReader::tokenise(input, ',');

if (tokens.size() != 3)
{
    std::cout << "Bad input\n";
    return;
}

OrderBookEntry obe(
    std::stod(tokens[1]),
    std::stod(tokens[2]),
    currentTime,
    tokens[0],
    OrderBookType::ask
);

obe.username = currentUser.userId;   

if (wallet.canFulfillOrder(obe))
{
    std::cout << "\nWallet looks good.\n";
    orderBook.insertOrder(obe);

    // Log ORDER PLACEMENT (not an executed trade).
    // Executed trades are logged inside gotoNextTimeframe() as BUY/SELL.
    Transaction t(
        currentTime,
        "ASK",
        obe.product,
        obe.price,
        obe.amount,
        wallet.getBalance(getBaseCurrency(obe.product)),
        currentUser.userId
    );

    transactionManager.logTransaction(t);
}
else
{
    std::cout << "\nInsufficient funds\n";
}
}
void MerkelMain::enterBid()
{
std::cout << "Enter bid: product,price,amount (e.g. ETH/BTC,200,0.5)\n";
std::string input;
std::getline(std::cin, input);
auto tokens = CSVReader::tokenise(input, ',');

if (tokens.size() != 3)
{
    std::cout << "Bad input\n";
    return;
}

OrderBookEntry obe(
    std::stod(tokens[1]),
    std::stod(tokens[2]),
    currentTime,
    tokens[0],
    OrderBookType::bid
);
    // Link order to current logged-in user
obe.username = currentUser.userId;   

if (wallet.canFulfillOrder(obe))
{
    std::cout << "\nWallet looks good.\n";
    orderBook.insertOrder(obe);

    // Log ORDER PLACEMENT (not an executed trade).
    // Executed trades are logged inside gotoNextTimeframe() as BUY/SELL.
    Transaction t(
        currentTime,
        "BID",
        obe.product,
        obe.price,
        obe.amount,
        wallet.getBalance(getQuoteCurrency(obe.product)),
        currentUser.userId
    );

    transactionManager.logTransaction(t);
}
else
{
    std::cout << "\nInsufficient funds\n";
}
}

// Prints wallet balances
void MerkelMain::printWallet()
{
   std::cout << wallet.toString() << std::endl;
  _getch();
}
// Trigger Matching / Next Timeframe:
void MerkelMain::gotoNextTimeframe()
{
std::cout << "=== Moving to next time frame ===\n\n";
for (const std::string& product : orderBook.getKnownProducts())
{
    auto sales = orderBook.matchAsksToBids(product, currentTime);

    if (sales.empty())
        continue;

    std::cout << "Product: " << product << "\n";

    for (const OrderBookEntry& sale : sales)
    {
        if (sale.username != currentUser.userId)
            continue;

        std::string base = getBaseCurrency(sale.product);
        std::string quote = getQuoteCurrency(sale.product);

        if (sale.orderType == OrderBookType::bid)
        {
            wallet.withdraw(quote, sale.price * sale.amount);
            wallet.deposit(base, sale.amount);
        }
        else
        {
            wallet.withdraw(base, sale.amount);
            wallet.deposit(quote, sale.price * sale.amount);
        }

        std::cout << "TRADE: " << sale.product
            << " | " << (sale.orderType == OrderBookType::bid ? "BID" : "ASK")
            << " | Qty: " << sale.amount
            << " @ " << sale.price << "\n";

        std::cout << "Updated wallet:\n" << wallet.toString() << "\n\n";

        // Log EXECUTED trade so wallet can be reconstructed correctly after restart.
        // bid  => user bought base using quote (BUY)
        // ask  => user sold base for quote (SELL)
        const std::string tradeType = (sale.orderType == OrderBookType::bid) ? "BUY" : "SELL";
        const std::string balanceCcy = (sale.orderType == OrderBookType::bid) ? quote : base;

        Transaction t(
            sale.timestamp,
            tradeType,
            sale.product,
            sale.price,
            sale.amount,
            wallet.getBalance(balanceCcy),
            currentUser.userId
        );
        transactionManager.logTransaction(t);
    }
}
    // Advance simulation time
currentTime = orderBook.getNextTime(currentTime);

std::cout << "Press any key to continue to next timeframe...";
_getch();
system("cls");
}

// Wallet deposit/withdraw handler:

void MerkelMain::handleWalletUpdate()
{
std::string Wallet_TransType;
std::string currency;
double amount = 0.0;
std::cout << "Enter currency (e.g. USDT, BTC): ";
std::cin >> currency;

std::cout << "Enter amount (positive = deposit, negative = withdraw): ";
std::cin >> amount;

std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

std::string timestamp = getSystemTimestamp();

bool success = false;

if (amount > 0)
{
    try
    {
        wallet.deposit(currency, amount);
        success = true;
        Wallet_TransType = "Deposit";
    }
    catch (const std::exception& e)
    {
        std::cout << "Deposit failed: " << e.what() << "\n";
        return;
    }
}
else if (amount < 0)
{
    success = wallet.withdraw(currency, -amount);
    Wallet_TransType = "Withdraw";
}
else
{
    std::cout << "Amount cannot be zero.\n";
    return;
}

if (!success)
{
    std::cout << "Wallet update failed (insufficient balance).\n";
    return;
}

Transaction tx(
    timestamp,
    Wallet_TransType,
    currency,
    0.0,
    amount,
    wallet.getBalance(currency),
    currentUser.userId
);    

transactionManager.logTransaction(tx);

std::cout << " Wallet updated successfully.\n";
}

// Reads a full line and parses option integer.
int MerkelMain::getUserOption()
{
int option;
std::string line;
std::getline(std::cin, line);

try
{
    option = std::stoi(line);// convert to integer
}
catch (...)
{
    std::cout << "Invalid input. Please enter a number.\n";
    return -1;
}

return option;
}

// Simulates trades for demo/testing:
void MerkelMain::simulateUserTrades()
{
    system("cls");
    std::cout << "\n\n";
    std::cout << "\t=============================================\n";
    std::cout << "\t      Simulated Trading Activity (Demo)\n";
    std::cout << "\t=============================================\n\n";

    auto products = orderBook.getKnownProducts();

    // Randomness for small price variations around market mid-price
    std::mt19937 rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<double> pctMove(-0.005, 0.005); // +/- 0.5%

    if (products.empty())
    {
        std::cout << "No products available for simulation.\n";
        std::cout << "\nPress Enter to return to menu...";

        // ensure clean input state, then wait for real Enter
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();

        system("cls");
        return;
    }

    for (const std::string& product : products)
    {
        std::string base = getBaseCurrency(product);
        std::string quote = getQuoteCurrency(product);

        // Use market mid-price as reference (fallback to 100 if dataset has no current bids/asks)
        double referencePrice = orderBook.getLatestMidPrice(product);
        if (referencePrice <= 0.0)
            referencePrice = 100.0;

        // Ensure we have enough quote currency to run the demo consistently.
        const double minQuoteNeeded = referencePrice * 10.0; // enough for multiple demo buys
        const double currentQuoteBal = wallet.getBalance(quote);

        if (currentQuoteBal < minQuoteNeeded)
        {
            const double topUp = (minQuoteNeeded - currentQuoteBal) + (minQuoteNeeded * 0.5);
            try
            {
                wallet.deposit(quote, topUp);
                transactionManager.logTransaction(
                    Transaction(getSystemTimestamp(), "Deposit", quote, 0.0, topUp,
                                wallet.getBalance(quote), currentUser.userId)
                );
            }
            catch (...)
            {
                // If deposit fails for any reason, continue simulation without crashing.
            }
        }

        std::cout << "---------------------------------------------\n";
        std::cout << "Product: " << product << "\n";
        std::cout << "Base Currency : " << base << "\n";
        std::cout << "Quote Currency: " << quote << "\n\n";

        // Requirement: at least FIVE bids and FIVE asks per product
        for (int i = 0; i < 5; ++i)
        {
            const double qty = 1.0;
            const double move = pctMove(rng);
            const double center = referencePrice * (1.0 + move);
            const double spread = 0.002; // 0.2%

            // BUY
            const double bidPrice = center * (1.0 - spread);
            const double cost = bidPrice * qty;

            std::cout << "[BUY ] Executing Trade (#" << (i + 1) << ")\n";
            std::cout << "  Buying : " << qty << " " << base << "\n";
            std::cout << "  Price  : " << bidPrice << " " << quote << "\n";
            std::cout << "  Paying : " << cost << " " << quote << "\n";

            if (wallet.withdraw(quote, cost))
            {
                wallet.deposit(base, qty);

                transactionManager.logTransaction(
                    Transaction(getSystemTimestamp(), "BUY", product,
                                bidPrice, qty,
                                wallet.getBalance(quote),
                                currentUser.userId)
                );

                std::cout << "  BUY executed successfully\n";
                std::cout << "  Wallet " << quote << ": " << wallet.getBalance(quote) << "\n";
            }
            else
            {
                std::cout << "  Insufficient " << quote << " balance\n";
            }

            std::cout << "\n";

            // SELL
            const double askPrice = center * (1.0 + spread);
            const double proceeds = askPrice * qty;

            std::cout << "[SELL] Executing Trade (#" << (i + 1) << ")\n";
            std::cout << "  Selling : " << qty << " " << base << "\n";
            std::cout << "  Price   : " << askPrice << " " << quote << "\n";
            std::cout << "  Receive : " << proceeds << " " << quote << "\n";

            if (wallet.withdraw(base, qty))
            {
                wallet.deposit(quote, proceeds);

                transactionManager.logTransaction(
                    Transaction(getSystemTimestamp(), "SELL", product,
                                askPrice, qty,
                                wallet.getBalance(quote),
                                currentUser.userId)
                );

                std::cout << "  SELL executed successfully\n";
                std::cout << "  Wallet " << quote << ": " << wallet.getBalance(quote) << "\n";
            }
            else
            {
                std::cout << "  Insufficient " << base << " balance\n";
            }

            std::cout << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    std::cout << "=============================================\n";
    std::cout << " Simulation completed successfully.\n";
    std::cout << "\nPress Enter to return to menu...";

    // This is the key fix: clear any leftover input then wait for Enter
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //std::cin.get();

    system("cls");
}




// Generates a system timestamp including milliseconds.
std::string MerkelMain::getSystemTimestamp()
{
using namespace std::chrono;
auto now = system_clock::now();
auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
std::time_t tt = system_clock::to_time_t(now);

std::tm tm{};
#if defined(_WIN32)
localtime_s(&tm, &tt);
#else
localtime_r(&tt, &tm);
#endif
std::ostringstream oss;
oss << std::put_time(&tm, "%Y/%m/%d %H:%M:%S")
    << "." << std::setw(3) << std::setfill('0') << ms.count();

return oss.str();
}


