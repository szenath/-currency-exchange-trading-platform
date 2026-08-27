#include "UserManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>
#include <ctime>
#include <random>

// Password policy constants
static constexpr int PASSWORD_VALID_DAYS = 90;
static constexpr int PASSWORD_WARN_DAYS = 10;

// Constructor
UserManager::UserManager(std::string file) : filename(file) {}

// Computes difference between two timestamps in days
int UserManager::daysBetween(std::time_t from, std::time_t to) const
{
    return static_cast<int>((to - from) / (60 * 60 * 24));
}
// Legacy userId generation using timestamp (not currently used)
std::string UserManager::generateUserId()
{
    std::stringstream ss;
    ss << std::time(nullptr);
    return ss.str();
}
// Generates a random 10-digit numeric user ID
std::string UserManager::generate10DigitUserId()
{
    static std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_int_distribution<long long> dist(1000000000LL, 9999999999LL);
    return std::to_string(dist(rng));
}

// Registers a new user and appends them to users.csv
bool UserManager::registerUser(User& user, const std::string& password)
{
    if (userExists(user.fullName, user.email))
        return false;

    user.userId = generate10DigitUserId();
    user.passwordHash = std::hash<std::string>{}(password);
    user.passwordLastUpdated = std::time(nullptr);   

    std::ofstream out(filename, std::ios::app);
    out << user.userId << ","
        << user.fullName << ","
        << user.email << ","
        << user.passwordHash << ","
        << user.passwordLastUpdated << "\n";

    return true;
}
// Authenticates a user during login
bool UserManager::loginUser(User& user)
{
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string id, name, email, hashStr, timeStr;

        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, email, ',');
        std::getline(ss, hashStr, ',');
        std::getline(ss, timeStr, ',');

                // Skip non-matching user IDs
        if (id != user.userId)
            continue;

        std::size_t storedHash = std::stoull(hashStr);
        if (storedHash != user.passwordHash)
            return false;

                    // Read last password update time
        std::time_t lastUpdated =
            timeStr.empty() ? std::time(nullptr) : std::stoll(timeStr);

        int daysUsed = daysBetween(lastUpdated, std::time(nullptr));
        int daysLeft = PASSWORD_VALID_DAYS - daysUsed;

        if (daysLeft <= 0)
        {
            std::cout << " Password expired. Please reset your password.\n";
            return false;
        }

        if (daysLeft <= PASSWORD_WARN_DAYS)
        {
            std::cout << "  Password expires in "
                << daysLeft << " days. Please reset it soon.\n";
        }

        user.fullName = name;
        user.email = email;
        user.passwordLastUpdated = lastUpdated;

        return true;
    }
    return false;
}

// Checks whether a user already exists based on name or email
bool UserManager::userExists(const std::string& fullName,
    const std::string& email)
{
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string id, name, mail;

        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, mail, ',');

        if (name == fullName || mail == email)
            return true;
    }
    return false;
}
// Checks whether a userId exists in users.csv
bool UserManager::userIdExists(const std::string& userId)
{
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string id;
        std::getline(ss, id, ',');

        if (id == userId)
            return true;
    }
    return false;
}

// Resets a user's password and rewrites users.csv
bool UserManager::resetPassword(const std::string& userId,
    const std::string& newPassword)
{
    std::ifstream in(filename);
    if (!in)
        return false;

    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    while (std::getline(in, line))
    {
        std::stringstream ss(line);
        std::string id, name, email, hashStr, timeStr;

        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, email, ',');
        std::getline(ss, hashStr, ',');
        std::getline(ss, timeStr, ',');

        if (id == userId)
        {
            std::size_t newHash =
                std::hash<std::string>{}(newPassword);
            std::time_t now = std::time(nullptr);

            std::ostringstream updatedLine;
            updatedLine << id << ","
                << name << ","
                << email << ","
                << newHash << ","
                << now;

            lines.push_back(updatedLine.str());
            updated = true;
        }
        else
        {
            lines.push_back(line);
        }
    }
    in.close();

    if (!updated)
        return false;

           // Rewrite entire file with updated contents
    std::ofstream out(filename, std::ios::trunc);
    for (const auto& l : lines)
        out << l << "\n";

    return true;
}
