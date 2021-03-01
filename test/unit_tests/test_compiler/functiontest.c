#include "functiontest.h"
#include "yats.h"

SETUP_YATS();

static void test_simple() {
	unsigned char expected[] = {
		0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_FCONST,
		0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // len
		0x02,	// number of parameters
		O_LLOAD, 0x00,
		O_LLOAD, 0x01,
		O_ADD,
		O_RET,
		O_NCONST,
		O_RET,
		O_POP,
		O_POP,
		O_HALT,
	};

	ASSERT_GEN_BC_EQ(expected, "fn add(a, b) { return a + b; };");

}

TEST(functiontest) {
	test_simple();
	return NUM_FAILED;
}