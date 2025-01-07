#include <assert.h>
#include <string.h>
#include "basic_testing.h"

#include "../stocks.h"

TEST(good_and_bad_times) {
    trades_log * l = new_log();

    assert_int(new_trade(l,1.0,"GOOGL",40.0,100), ==, 1);
    assert_int(new_trade(l,1.0,"XYZ",1000.0,200), ==, 1);
    assert_int(new_trade(l,0.5,"XYZ",1000.0,350), ==, 0);
    assert_int(new_trade(l,0.6,"XYZ",1000.0,350), ==, 0);
    assert_uint(total_quantity(l), ==, 300);
    delete_log(l);
    TEST_PASSED;
}

TEST(good_clear_good_times) {
    trades_log * l = new_log();

    assert_int(new_trade(l,1.0,"GOOGL",40.0,100), ==, 1);
    assert_int(new_trade(l,1.0,"XYZ",1000.0,200), ==, 1);
    clear(l);
    assert_int(new_trade(l,0.5,"XYZ",1000.0,350), ==, 1);
    assert_uint(total_quantity(l), ==, 350);
    delete_log(l);
    TEST_PASSED;
}

TEST(good_and_bad_prices) {
    trades_log * l = new_log();

    assert_int(new_trade(l,1.0,"GOOGL",40.0,100), ==, 1);
    assert_int(new_trade(l,1.0,"XYZ",1000.0,200), ==, 1);
    assert_int(new_trade(l,0.5,"XYZ",0,350), ==, 0);
    assert_int(new_trade(l,0.5,"XYZ",-1,350), ==, 0);
    assert_uint(total_quantity(l), ==, 300);
    delete_log(l);
    TEST_PASSED;
}

TEST(good_and_bad_quantities) {
    trades_log * l = new_log();

    assert_int(new_trade(l,1.0,"GOOGL",40.0,100), ==, 1);
    assert_int(new_trade(l,1.0,"XYZ",1000.0,200), ==, 1);
    assert_int(new_trade(l,0.5,"XYZ",200,-20), ==, 0);
    assert_int(new_trade(l,0.5,"XYZ",400,-10), ==, 0);
    assert_uint(total_quantity(l), ==, 300);
    delete_log(l);
    TEST_PASSED;
}

int main() {
    RUN_TEST(good_and_bad_times);
    RUN_TEST(good_clear_good_times);
    RUN_TEST(good_and_bad_prices);
    RUN_TEST(good_and_bad_quantities);
    PRINT_TEST_RESULTS;
    assert(ALL_TESTS_PASSED);
}
