#ifndef STOCKS_H_INCLUDED
#define STOCKS_H_INCLUDED

struct trades_log;

struct trades_log * new_log();		/* constructor */
void delete_log(struct trades_log * l);	/* destructor */

int new_trade(struct trades_log * l, double time,
	      const char * symbol, double price, unsigned int quantity);
void clear(struct trades_log * l);

void set_time_window(struct trades_log * l, double w);

unsigned int total_quantity(const struct trades_log * l);

double high_price(const struct trades_log * l);
double low_price(const struct trades_log * l);

double stock_high_price(const struct trades_log * l, const char * symbol);
double stock_low_price(const struct trades_log * l, const char * symbol);
double stock_avg_price(const struct trades_log * l, const char * symbol);

#endif
