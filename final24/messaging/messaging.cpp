#include <set>
#include <map>
#include <string>
#include <algorithm>
#include "messaging.h"

using namespace std;

struct server {
    map<receiver *, set<set<string>>> subscriptions;
};

