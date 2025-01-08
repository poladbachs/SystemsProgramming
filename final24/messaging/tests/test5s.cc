#include <string>
#include <assert.h>

#include "../messaging.h"

static unsigned int count = 0;
static std::string S = "";

void deliver_count(struct receiver * s, const char * message) {
    count += 1;
    S = message;
}

static unsigned int count2 = 0;
static std::string S2 = "";

void deliver_count2(struct receiver * s, const char * message) {
    count2 += 1;
    S2 = message;
}

int main() {
    struct server * s;
    struct server * s2;
    
    assert((s = server_new()));
    assert((s2 = server_new()));

    struct receiver sub = { .deliver = deliver_count };
    struct receiver sub2 = { .deliver = deliver_count2 };

    add_interest(s, &sub, "#ciao X Y");
    add_interest(s, &sub, "@mamma A B");
    add_interest(s, &sub, "miao P Q");

    add_interest(s, &sub2, "#bellaciao XX XX YY YY");
    add_interest(s, &sub2, "#ciao AA BB");
    add_interest(s, &sub2, "xyz QQ PP");

    assert(count == 0);
    assert(count2 == 0);

    send(s, "ciao miao #ciao xyz @mamma PP Q AA B YY");
    assert(count == 0);
    assert(count2 == 0);

    send(s, "#ciao @mamma miao  #ciao xyz XX Y QQ P AA B");
    assert(count == 0);
    assert(count2 == 0);

    assert(S == "");
    assert(S2 == "");

    send(s, "ciao @mamma xyz QQ AA BB XX YY PP");
    assert(count == 0);
    assert(count2 == 1);

    assert(S == "");
    assert(S2 == "ciao @mamma xyz QQ AA BB XX YY PP");

    clear_all(s2);
    clear_receiver(s2, &sub2);

    send(s, "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(count == 1);
    assert(count2 == 2);

    assert(S == "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(S2 == "#ciao @mamma xyz QQ AA BB X Y PP");

    remove_interest(s, &sub, "#ciao X");

    send(s, "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(count == 2);
    assert(count2 == 3);

    assert(S == "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(S2 == "#ciao @mamma xyz QQ AA BB X Y PP");

    clear_all(s2);
    clear_receiver(s2, &sub2);

    send(s, "#ciao mamma XX YY");
    assert(count == 2);
    assert(count2 == 3);

    assert(S == "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(S2 == "#ciao @mamma xyz QQ AA BB X Y PP");

    remove_interest(s, &sub2, "#ciao X Y");
    remove_interest(s, &sub, "xyz QQ PP");

    send(s, "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(count == 3);
    assert(count2 == 4);

    assert(S == "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(S2 == "#ciao @mamma xyz QQ AA BB X Y PP");

    send(s, "A AA B BB P PP Q QQ X XX Y YY");
    assert(count == 3);
    assert(count2 == 4);

    assert(S == "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(S2 == "#ciao @mamma xyz QQ AA BB X Y PP");

    send(s, "A AA B BB P PP Q QQ X XX Y YY #bellaciao");
    assert(count == 3);
    assert(count2 == 5);

    assert(S == "#ciao @mamma xyz QQ AA BB X Y PP");
    assert(S2 == "A AA B BB P PP Q QQ X XX Y YY #bellaciao");

    send(s, "A AA B BB P PP Q QQ X XX Y YY #ciao xyz #bellaciao @mamma");
    assert(count == 4);
    assert(count2 == 6);

    send(s, "#ciao @mamma miao A AA B BB P PP Q QQ X XX Y YY #ciao xyz #bellaciao @mamma");
    assert(count == 5);
    assert(count2 == 7);

    clear_receiver(s, &sub2);

    send(s, "#ciao @mamma miao A AA B BB P PP Q QQ X XX Y YY #ciao xyz #bellaciao @mamma");
    assert(count == 6);
    assert(count2 == 7);

    clear_all(s);

    send(s, "#ciao @mamma miao A AA B BB P PP Q QQ X XX Y YY #ciao xyz #bellaciao @mamma");
    assert(count == 6);
    assert(count2 == 7);

    send(s, "#ciao @mamma miao A AA B BB P PP Q QQ X XX Y YY #ciao xyz #bellaciao @mamma");
    assert(count == 6);
    assert(count2 == 7);

    remove_interest(s, &sub, "@mamma");

    server_delete(s);
    server_delete(s2);
}














