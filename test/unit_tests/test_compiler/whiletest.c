#include "whiletest.h"
#include "yats.h"

SETUP_YATS();

static void test_while() {
	unsigned char expected[] = {
		0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BCONST_T,
		O_BRF_8,
		0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BCONST_T,
		O_ECHO,
		O_BR_8,
		0xEB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "while true { echo true; };");
}

static void test_while_fold_false() {
	unsigned char expected[] = {
		0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "while false { echo true; };");
}

static void test_continue() {
	unsigned char expected[] = {
		0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0,
		C_INT_1, 0x0A,
		C_INT_1, 5,
		O_LIT, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x01,
		O_LT,
		O_BRF_8,
		0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x02,
		O_EQ,
		O_BRF_8,
		0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BR_8,
		0xDB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_LLOAD, 0x00,
		O_ECHO,
		O_BR_8,
		0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "let i = 0; while i < 10 { if i == 5 { continue; }; echo i; };");
}

static void test_break() {
	unsigned char expected[] = {
		0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0,
		C_INT_1, 0x0A,
		C_INT_1, 5,
		O_LIT, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x01,
		O_LT,
		O_BRF_8,
		0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x02,
		O_EQ,
		O_BRF_8,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BCONST_F,
		O_BR_8,
		0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_LLOAD, 0x00,
		O_ECHO,
		O_BR_8,
		0xCE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "let i = 0; while i < 10 { if i == 5 { break; }; echo i; };");
}

TEST(whiletest) {
	test_while();
	test_while_fold_false();
	test_continue();
	test_break();
	return NUM_FAILED;
}
