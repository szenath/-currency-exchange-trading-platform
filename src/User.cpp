#include "User.h"

User::User()
    : userId(""),
    fullName(""),
    email(""),
    passwordHash(0),
    passwordLastUpdated(std::time(nullptr)) {
}

User::User(std::string uid,
    std::string fn,
    std::string em,
    std::size_t ph,
    std::time_t lastUpdated)
    : userId(uid),
    fullName(fn),
    email(em),
    passwordHash(ph),
    passwordLastUpdated(lastUpdated) {
}