#include <assert.h>

#include "../messaging.h"

static unsigned int count = 0;

void deliver_count(struct receiver * s, const char * message) {
    count += 1;
}

static unsigned int count2 = 0;

void deliver_count2(struct receiver * s, const char * message) {
    count2 += 1;
}

int main() {
    struct server * s;
    struct server * s2;
    
    assert((s = server_new()));
    assert((s2 = server_new()));

    struct receiver sub = { .deliver = deliver_count };
    struct receiver sub2 = { .deliver = deliver_count2 };

    add_interest(s, &sub, "#ciao XX YY");
    add_interest(s, &sub, "@mamma");
    add_interest(s, &sub, "miao");

    add_interest(s, &sub2, "#bellaciao");
    add_interest(s, &sub2, "#ciao");
    add_interest(s, &sub2, "xyz");

    assert(count == 0);
    assert(count2 == 0);

    send(s, "ciao mamma");
    assert(count == 0);
    assert(count2 == 0);

    send(s, "#ciao @mamma");
    assert(count == 1);
    assert(count2 == 1);

    send(s, "ciao YY");
    assert(count == 1);
    assert(count2 == 1);

    clear_all(s2);
    clear_receiver(s2, &sub2);

    send(s, "#ciao YY mamma XX");
    assert(count == 2);
    assert(count2 == 2);

    remove_interest(s, &sub, "#ciao");

    send(s, "bella ciao #bellaciao!");
    assert(count == 2);
    assert(count2 == 3);

    send(s, "bella ciao #bella ciao!");
    assert(count == 2);
    assert(count2 == 3);

    send(s, "bella @mamma, xyz, YY/XX ciaociao!");
    assert(count == 3);
    assert(count2 == 4);

    send(s, "bella @mamma, 'miao'!");
    assert(count == 4);
    assert(count2 == 4);

    remove_interest(s, &sub, "@mamma");

    send(s, "bella @mamma, ciaociao!");
    assert(count == 4);
    assert(count2 == 4);

    remove_interest(s, &sub2, "#ciao");

    send(s, "YY/YY/XX bella mamma, #ciao ciaociao!");
    assert(count == 5);
    assert(count2 == 4);

    clear_receiver(s, &sub2);

    send(s, "ciao-miao-bao #bellaciao xyz #ciao ooooh!");
    assert(count == 6);
    assert(count2 == 4);

    clear_receiver(s, &sub);

    send(s, "bella @mamma, XX #ciao ciaociao!");
    assert(count == 6);
    assert(count2 == 4);

    remove_interest(s, &sub, "miao");

    send(s, "ciao-miao-bao ooooh!");
    assert(count == 6);
    assert(count2 == 4);

    send(s, "bella @mamma, ciaociao!");
    assert(count == 6);
    assert(count2 == 4);

    remove_interest(s, &sub, "@mamma");

    send(s, "#ciao @mamma miao ciao");
    assert(count == 6);
    assert(count2 == 4);

    server_delete(s);
    server_delete(s2);
}
