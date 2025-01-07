#include <assert.h>
#include "basic_testing.h"

#include "../stocks.h"

TEST(adding_one) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    assert_int(new_trade(l,1.0,"GOOGL",40.0,1000), ==, 1);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

TEST(adding_two) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    assert_int(new_trade(l,1.0,"GOOGL",40.0,1000), ==, 1);
    assert_int(new_trade(l,2.0,"XYZ",3.4,20), ==, 1);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

TEST(compute_quantity) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    assert_int(new_trade(l,1.0,"GOOGL",40.0,1000), ==, 1);
    assert_int(new_trade(l,2.0,"XYZ",3.4,20), ==, 1);
    assert_uint(total_quantity(l), ==, 1020);
    assert_uint(total_quantity(l2), ==, 0);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

TEST(window_trim) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    assert_int(new_trade(l2,1.0,"GOOGL",40.0,1000), ==, 1);
    assert_int(new_trade(l2,2.0,"XYZ",3.4,20), ==, 1);
    assert_int(new_trade(l2,100.0,"GOOGL",50.0,500), ==, 1);
    assert_uint(total_quantity(l), ==, 0);
    assert_uint(total_quantity(l2), ==, 500);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

TEST(window_set_and_trim) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    assert_int(new_trade(l,10.0,"GOOGL",40.0,1000), ==, 1);
    assert_int(new_trade(l,30.0,"XYZ",3.4,20), ==, 1);
    assert_int(new_trade(l,40.0,"GOOGL",50.0,500), ==, 1);
    set_time_window(l,20);
    assert_uint(total_quantity(l), ==, 520);
    assert_uint(total_quantity(l2), ==, 0);
    set_time_window(l,30);
    assert_uint(total_quantity(l), ==, 520);
    assert_uint(total_quantity(l2), ==, 0);
    assert_int(new_trade(l,50.0,"XYZ",3.4,10), ==, 1);
    assert_uint(total_quantity(l), ==, 530);
    assert_uint(total_quantity(l2), ==, 0);
    assert_int(new_trade(l,81.0,"XYZ",3.4,2), ==, 1);
    assert_uint(total_quantity(l), ==, 2);
    assert_uint(total_quantity(l2), ==, 0);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

TEST(window_set_and_trim2) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    assert_int(new_trade(l,10.0,"GOOGL",40.0,1000), ==, 1);
    assert_int(new_trade(l,30.0,"XYZ",3.4,20), ==, 1);
    assert_int(new_trade(l,40.0,"GOOGL",50.0,500), ==, 1);
    set_time_window(l2,20);
    assert_uint(total_quantity(l), ==, 1520);
    assert_uint(total_quantity(l2), ==, 0);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

TEST(large_trim) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    for (double t = 0.0; t <= 1000; t += 5)
	assert_int(new_trade(l2,t,"GOOGL",40.0,1), ==, 1);

    assert_uint(total_quantity(l), ==, 0);
    assert_uint(total_quantity(l2), ==, 13);
    clear(l);
    assert_uint(total_quantity(l), ==, 0);
    assert_uint(total_quantity(l2), ==, 13);
    clear(l2);
    assert_uint(total_quantity(l), ==, 0);
    assert_uint(total_quantity(l2), ==, 0);
    delete_log(l);
    TEST_PASSED;
}

TEST(large_low_high) {
    trades_log * l = new_log();
    trades_log * l2 = new_log();
    for (double t = 0.0; t <= 1000; t += 5)
	assert_int(new_trade(l,t,"GOOGL",10 + t,1), ==, 1);

    assert_uint(total_quantity(l), ==, 13);
    assert_double(high_price(l), ==, 1010);
    assert_double(low_price(l), ==, 950);

    assert_uint(total_quantity(l2), ==, 0);
    assert_double(high_price(l2), ==, 0);
    assert_double(low_price(l2), ==, 0);
    delete_log(l);
    delete_log(l2);
    TEST_PASSED;
}

int main() {
    RUN_TEST(adding_one);
    RUN_TEST(adding_two);
    RUN_TEST(compute_quantity);
    RUN_TEST(window_trim);
    RUN_TEST(window_set_and_trim);
    RUN_TEST(window_set_and_trim2);
    RUN_TEST(large_trim);
    RUN_TEST(large_low_high);
    PRINT_TEST_RESULTS;
    assert(ALL_TESTS_PASSED);
}
