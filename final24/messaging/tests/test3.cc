#include <assert.h>

#include "../messaging.h"

static unsigned int count = 0;

void deliver_count(struct receiver * s, const char * message) {
    count += 1;
}

int main() {
    struct server * s;
    struct server * s2;
    
    assert((s = server_new()));
    assert((s2 = server_new()));

    struct receiver sub = { .deliver = deliver_count };

    add_interest(s, &sub, "#ciao");
    add_interest(s, &sub, "@mamma");
    add_interest(s, &sub, "miao");

    clear_receiver(s2, &sub);

    assert(count == 0);

    send(s, "ciao mamma");
    assert(count == 0);

    send(s, "#ciao @mamma");
    assert(count == 1);

    send(s, "ciao @mamma");
    assert(count == 2);

    clear_all(s2);

    send(s, "#ciao mamma");
    assert(count == 3);

    remove_interest(s, &sub, "#ciao");

    send(s, "bella ciao #bellaciao!");
    assert(count == 3);

    send(s, "bella ciao #bella #ciao!");
    assert(count == 3);

    send(s, "bella @mamma, ciaociao!");
    assert(count == 4);

    send(s, "bella @mamma, 'miao'!");
    assert(count == 5);

    remove_interest(s, &sub, "@mamma");

    send(s, "bella @mamma, ciaociao!");
    assert(count == 5);

    send(s, "ciao-miao-bao ooooh!");
    assert(count == 6);

    clear_receiver(s, &sub);

    send(s, "bella @mamma, ciaociao!");
    assert(count == 6);

    remove_interest(s, &sub, "miao");

    send(s, "ciao-miao-bao ooooh!");
    assert(count == 6);

    send(s, "bella @mamma, ciaociao!");
    assert(count == 6);

    remove_interest(s, &sub, "@mamma");

    send(s, "#ciao @mamma miao ciao");
    assert(count == 6);

    server_delete(s);
    server_delete(s2);
}
