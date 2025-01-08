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

void trades_log::trim() {
    for (auto i = trades.begin(); i != trades.end() && i->time < (trades.back().time - window);
        i = trades.begin())
        trades.erase(i);
}

int new_trade(trades_log * l, double t, const char * s, double p, unsigned int q) {
    if (q <= 0 || p <= 0) return 0;
    if (!l->trades.empty() && t < l->trades.back().time) return 0;

    trade tr;
    tr.time = t;
    tr.price = p;
    tr.quantity = q;

    int i;
    for (i = 0; i < MAX_STOCK_LEN && s[i] != 0; ++i)
        tr.symbol[i] = s[i];
    tr.symbol[i] = 0;

    l->trades.push_back(tr);
    l->trim();
    return 1;
}

void clear(trades_log * l) {
    l->trades.clear();
}

