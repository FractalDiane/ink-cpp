#include <gtest/gtest.h>

TEST(TestTests, TestOne) {
	ASSERT_TRUE(true);
}

int main() {
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
