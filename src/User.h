#pragma once
#include <string>
#include <ctime>

class User
{
public:
    std::string userId;
    std::string fullName;
    std::string email;
    std::size_t passwordHash;
    std::time_t passwordLastUpdated;   // NEW

    User();
    User(std::string uid,
        std::string fn,
        std::string em,
        std::size_t ph,
        std::time_t lastUpdated);
};