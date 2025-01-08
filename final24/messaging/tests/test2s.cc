#include <assert.h>

#include "../messaging.h"

static unsigned int count = 0;

void deliver_count(struct receiver * s, const char * message) {
    count += 1;
}

int main() {
    struct server * s;
    
    assert((s = server_new()));

    struct receiver sub = { .deliver = deliver_count };

    add_interest(s, &sub, "mamma ciao");
    add_interest(s, &sub, "bella ciao");
    add_interest(s, &sub, "miao");

    assert(count == 0);

    send(s, "ciao mamma");
    assert(count == 1);

    send(s, "#ciao @mamma");
    assert(count == 1);

    send(s, "ciao @mamma");
    assert(count == 1);

    send(s, "#ciao mamma");
    assert(count == 1);

    remove_interest(s, &sub, "#ciao");

    send(s, "ciao #bellaciao bella!");
    assert(count == 2);

    send(s, "bella mamma ciao!");
    assert(count == 3);

    send(s, "bella @mamma, ciaociao!");
    assert(count == 3);

    send(s, "bella @mamma, 'miao'!");
    assert(count == 4);

    remove_interest(s, &sub, "ciao mamma");

    send(s, "bella mamma ciao!");
    assert(count == 5);

    send(s, "mamma, ciao ciao!");
    assert(count == 5);

    send(s, "ciao mamma, ciao mamma ciao!");
    assert(count == 5);

    send(s, "ciao-miao-bao ooooh!");
    assert(count == 6);

    server_delete(s);
}
