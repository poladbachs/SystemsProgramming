#ifndef MESSAGING_H_INCLUDED
#define MESSAGING_H_INCLUDED

struct server;

struct server * server_new(); /* return 0 on failure */
void server_delete(struct server * s);

struct receiver {
    void (*deliver)(struct receiver * r, const char * message);
};

/* return 0 on failure */
int add_interest(struct server * srv, struct receiver * r, const char * interest);
void remove_interest(struct server * srv, struct receiver * r, const char * interest);

void clear_receiver(struct server * srv, struct receiver * r);
void clear_all(struct server * srv);

void send(const struct server * srv, const char * message);

#endif
