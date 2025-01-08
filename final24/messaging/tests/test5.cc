#include <string>
#include <assert.h>

#include "../messaging.h"

static unsigned int count = 0;
static std::string S = "";
static struct receiver * R = 0;

void deliver_count(struct receiver * r, const char * message) {
    count += 1;
    S = message;
    R = r;
}

static unsigned int count2 = 0;
static std::string S2 = "";
static struct receiver * R2 = 0;

void deliver_count2(struct receiver * r, const char * message) {
    count2 += 1;
    S2 = message;
    R2 = r;
}

int main() {
    struct server * s;
    struct server * s2;
    
    assert((s = server_new()));
    assert((s2 = server_new()));

    struct receiver sub = { .deliver = deliver_count };
    struct receiver sub2 = { .deliver = deliver_count2 };
    struct receiver sub3 = { .deliver = deliver_count2 };

    add_interest(s, &sub, "#ciao");
    add_interest(s, &sub, "@mamma");
    add_interest(s, &sub, "miao");

    add_interest(s, &sub2, "#bellaciao");
    add_interest(s, &sub2, "#ciao");
    add_interest(s, &sub2, "xyz");

    add_interest(s, &sub3, "blah");
    
    assert(count == 0);
    assert(count2 == 0);

    assert(R == 0);
    assert(R2 == 0);

    send(s, "ciao mamma");
    assert(count == 0);
    assert(count2 == 0);
    assert(R == 0);
    assert(R2 == 0);

    send(s, "#ciao @mamma");
    assert(count == 1);
    assert(count2 == 1);
    assert(R == &sub);
    assert(R2 == &sub2);
    assert(S == "#ciao @mamma");
    assert(S2 == "#ciao @mamma");

    send(s, "ciao @mamma");
    assert(count == 2);
    assert(count2 == 1);
    assert(R == &sub);
    assert(R2 == &sub2);
    assert(S == "ciao @mamma");
    assert(S2 == "#ciao @mamma");

    clear_all(s2);
    clear_receiver(s2, &sub2);

    send(s, "#ciao mamma");
    assert(count == 3);
    assert(count2 == 2);
    assert(R == &sub);
    assert(R2 == &sub2);
    assert(S == "#ciao mamma");
    assert(S2 == "#ciao mamma");

    send(s, "blah blah beh, boh!");
    assert(count == 3);
    assert(count2 == 3);
    assert(R == &sub);
    assert(R2 == &sub3);

    send(s, "blah blah #ciao beh, boh!");
    assert(count == 4);
    assert(count2 == 5);

    remove_interest(s, &sub, "#ciao");

    count = 3;
    count2 = 2;
    send(s, "bella ciao #bellaciao!");
    assert(count == 3);
    assert(count2 == 3);

    assert(S == "blah blah #ciao beh, boh!");
    assert(S2 == "bella ciao #bellaciao!");

    send(s, "bella ciao #bella ciao!");
    assert(count == 3);
    assert(count2 == 3);

    assert(S == "blah blah #ciao beh, boh!");
    assert(S2 == "bella ciao #bellaciao!");

    remove_interest(s, &sub2, "@mamma");
    remove_interest(s, &sub, "xyz");

    send(s, "bella @mamma, xyz, ciaociao!");
    assert(count == 4);
    assert(count2 == 4);

    assert(S == "bella @mamma, xyz, ciaociao!");
    assert(S2 == "bella @mamma, xyz, ciaociao!");

    send(s, "bella @mamma, 'miao'!");
    assert(count == 5);
    assert(count2 == 4);

    remove_interest(s, &sub, "@mamma");

    send(s, "bella @mamma, ciaociao!");
    assert(count == 5);
    assert(count2 == 4);

    clear_receiver(s, &sub2);
    
    send(s, "ciao-miao-bao #bellaciao xyz #ciao ooooh!");
    assert(count == 6);
    assert(count2 == 4);

    clear_receiver(s, &sub);

    send(s, "bella @mamma, ciaociao!");
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

    server_delete(s);
    server_delete(s2);
}
