#include <set>
#include <map>
#include <string>
#include <algorithm>
#include "messaging.h"

using namespace std;

struct server {
    map<receiver *, set<set<string>>> subscriptions;
};

struct server * server_new() {
    return new server();
}

void server_delete(struct server * s) {
    delete(s);
}

static void read_tags(const char * m, set<string> & s) {
    const char * begin;
    s.clear();

    init_state:
        if (*m == '#' || *m == '@' || isalpha(*m)) {
            begin = m;
            m += 1;
            goto reading_tag;
        }
        if (*m == 0)
            return;
        m += 1;
        goto reading_tag;
    reading_tag:
        if(isalpha(*m)) {
            m += 1;
            goto reading_tag;
        }
        string tag(begin, m - begin);
        s.insert(tag);
        goto init_state;
}