#pragma once
#include <string>
#include "User.h"

class UserManager
{
public:
//Construct UserManager with path to users CSV file
    UserManager(std::string file);

    bool registerUser(User& user, const std::string& password);
    bool loginUser(User& user);
    bool userIdExists(const std::string& userId);

    
    bool resetPassword(const std::string& userId,
        const std::string& newPassword);

private:
    std::string filename; // CSV file used for persistent user storage


    //Checks if a user already exists based on
     //full namee / email  address
    bool userExists(const std::string& fullName,
        const std::string& email);

    std::string generateUserId();
    std::string generate10DigitUserId();

    int daysBetween(std::time_t from, std::time_t to) const;
};
