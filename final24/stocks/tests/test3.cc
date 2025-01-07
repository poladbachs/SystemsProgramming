#include <assert.h>
#include <string.h>
#include "basic_testing.h"

#include "../stocks.h"

TEST(adding_same_name_buf) {
    trades_log * l = new_log();

    char buf[100];
    strcpy(buf, "GOOGL");
    assert_int(new_trade(l,1.0,buf,40.0,100), ==, 1);
    strcpy(buf, "XYZ");
    assert_int(new_trade(l,2.0,buf,1000.0,200), ==, 1);
    assert_uint(total_quantity(l), ==, 300);

    assert_uint(stock_low_price(l,"GOOGL"), ==, 40);
    assert_uint(stock_high_price(l,"GOOGL"), ==, 40);
    assert_uint(stock_low_price(l,"XYZ"), ==, 1000);
    assert_uint(stock_high_price(l,"XYZ"), ==, 1000);
    assert_uint(stock_low_price(l,"ABC"), ==, 0);
    assert_uint(stock_high_price(l,"ABC"), ==, 0);

    strcpy(buf, "ABC");
    assert_uint(stock_low_price(l,buf), ==, 0);
    assert_uint(stock_high_price(l,buf), ==, 0);

    strcpy(buf, "XYZ");
    assert_uint(stock_low_price(l,buf), ==, 1000);
    assert_uint(stock_high_price(l,buf), ==, 1000);

    delete_log(l);
    TEST_PASSED;
}

TEST(price_ranges) {
    trades_log * l = new_log();
    double t = 1;
    for (double p = 100; p <= 200; p += 5) {
	assert_int(new_trade(l,t,"GOOGL",p,10), ==, 1);
	assert_int(new_trade(l,t,"XYZ",p+100,20), ==, 1);
    }
    assert_uint(stock_low_price(l,"GOOGL"), ==, 100);
    assert_uint(stock_high_price(l,"GOOGL"), ==, 200);
    assert_uint(stock_avg_price(l,"GOOGL"), ==, 150);

    assert_uint(stock_low_price(l,"XYZ"), ==, 200);
    assert_uint(stock_high_price(l,"XYZ"), ==, 300);
    assert_uint(stock_avg_price(l,"XYZ"), ==, 250);

    delete_log(l);
    TEST_PASSED;
}

int main() {
    RUN_TEST(adding_same_name_buf);
    RUN_TEST(price_ranges);
    PRINT_TEST_RESULTS;
    assert(ALL_TESTS_PASSED);
}
