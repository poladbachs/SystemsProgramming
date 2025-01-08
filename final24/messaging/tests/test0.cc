#include <assert.h>

#include "../messaging.h"

int main() {
    struct server * s1;
    struct server * s2;
    
    assert((s1 = server_new()));
    server_delete(s1);

    assert((s2 = server_new()));
    server_delete(s2);

    assert((s1 = server_new()));
    assert((s2 = server_new()));
    server_delete(s1);
    server_delete(s2);

    assert((s1 = server_new()));
    assert((s2 = server_new()));

    clear_receiver(s1, 0);
    clear_all(s1);

    clear_receiver(s2, 0);
    clear_all(s2);

    server_delete(s1);
    server_delete(s2);
}
