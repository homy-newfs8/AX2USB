#ifdef ARDUINO
#include <Arduino.h>
#endif
#include <unity.h>
#include "sq.hpp"

void
setUp(void) {
	// set stuff up here
}

void
tearDown(void) {
	// clean stuff up here
}

void
test_basic() {
	constexpr size_t size = 3;
	ax2usb::SQ<uint8_t, size> q{};
	uint8_t v;

	// initial status
	TEST_ASSERT_EQUAL(0, q.count());
	TEST_ASSERT_EQUAL(size, q.capacity());
	TEST_ASSERT_EQUAL(false, q.get(v));

	// simple put, get
	TEST_ASSERT_EQUAL(true, q.put(10));
	TEST_ASSERT_EQUAL(true, q.get(v));
	TEST_ASSERT_EQUAL(10, v);

	// empty
	TEST_ASSERT_EQUAL(false, q.get(v));

	// full then emtpy
	TEST_ASSERT_EQUAL(true, q.put(11));
	TEST_ASSERT_EQUAL(1, q.count());
	TEST_ASSERT_EQUAL(true, q.put(12));
	TEST_ASSERT_EQUAL(true, q.put(13));
	TEST_ASSERT_EQUAL(false, q.put(99));
	TEST_ASSERT_EQUAL(true, q.get(v));
	TEST_ASSERT_EQUAL(11, v);
	TEST_ASSERT_EQUAL(true, q.get(v));
	TEST_ASSERT_EQUAL(12, v);
	TEST_ASSERT_EQUAL(true, q.get(v));
	TEST_ASSERT_EQUAL(13, v);
	TEST_ASSERT_EQUAL(false, q.get(v));
}

void
run_tests() {
	UNITY_BEGIN();
	RUN_TEST(test_basic);
	UNITY_END();
}

#ifndef ARDUINO
int
main(int argc, char** argv) {
	run_tests();
	return 0;
}
#endif

void
setup() {
	Serial.begin(115200);
	delay(3000);
	run_tests();
}

void
loop() {
	delay(100);
}
