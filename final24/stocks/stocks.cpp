#include <list>
#include <cstring>

#include "stocks.h"

static const int MAX_STOCK_LEN = 5;

struct trade {
    double time;
    char symbol[MAX_STOCK_LEN + 1];
    double price;
    unsigned int quantity;
};

struct trades_log {
    std::list<trade> trades;
    double window;
    trades_log(): window(60) {};
    void trim();
};

struct trades_log * new_log() {
    return new trades_log();
}

void delete_log(struct trades_log * l) {
    delete(l);
}