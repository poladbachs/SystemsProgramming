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