#include <assert.h>
#include "basic_testing.h"

#include "../stocks.h"

TEST(compilation) {
    TEST_PASSED;
}

TEST(constructor_destructor) {
    trades_log * l = new_log();
    delete_log(l);
    TEST_PASSED;
}

TEST(clear_call) {
    trades_log * l = new_log();
    clear(l);
    delete_log(l);
    TEST_PASSED;
}

TEST(empty_log) {
    trades_log * l = new_log();
    assert_uint(total_quantity(l), ==, 0);
    assert_double(high_price(l), ==, 0);
    assert_double(low_price(l), ==, 0);

    const char * symbol = "GOOGL";
    assert_double(stock_high_price(l, symbol), == , 0);
    assert_double(stock_low_price(l, symbol), == , 0);
    assert_double(stock_avg_price(l, symbol), == , 0);

    delete_log(l);
    TEST_PASSED;
}

TEST(window_up_down) {
    trades_log * l = new_log();
    set_time_window(l,100);
    set_time_window(l,40);
    set_time_window(l,60);
    delete_log(l);
    TEST_PASSED;
}

int main() {
    RUN_TEST(compilation);
    RUN_TEST(constructor_destructor);
    RUN_TEST(clear_call);
    RUN_TEST(empty_log);
    RUN_TEST(window_up_down);
    PRINT_TEST_RESULTS;
    assert(ALL_TESTS_PASSED);
}
