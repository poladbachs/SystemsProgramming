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

int add_interest(struct server * srv, struct receiver * s, const char * tagset) {
    set<string> tags;
    read_tags(tagset, tags);
    srv->subscriptions[s].insert(tags);
    return 1;
}

void remove_interest(struct server * srv, struct receiver * s, const char * tagset) {
    auto i = srv->subscriptions.find(s);
    if (i != srv->subscriptions.end()) {
        set<string> tags;
        read_tags(tagset, tags);
        i->second.erase(tags);
    }
}

void clear_receiver(struct server * srv, struct receiver * s) {
    srv->subscriptions.erase(s);
}

void clear_all (struct server * srv) {
    srv->subscriptions.clear();
}