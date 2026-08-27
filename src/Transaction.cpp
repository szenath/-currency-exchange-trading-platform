#include "Transaction.h"

Transaction::Transaction(std::string ts,
    std::string t,
    std::string p,
    double pr,
    double amt,
    double wb,
    std::string uid)
    : timestamp(ts),
    type(t),
    product(p),
    price(pr),
    amount(amt),
    walletBalance(wb),
    userId(uid)
{
}