#include "unoptest.h"
#include "yats.h"

SETUP_YATS();

static void test_neg() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo -16;");
}

static void test_not() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BCONST_F,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo !true;");
}

static void test_bnot() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST_M1,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo ^0x00;");
}

static void test_mul() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 2 * 3;");
}

static void test_idiv() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST_1,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 3 // 2;");
}

static void test_mod() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST_2,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 5 % 3;");
}

static void test_add() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 6 + 5;");
}

static void test_sub() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST_5,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 8 - 3;");
}

static void test_bshl() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 2 << 3;");
}

static void test_bshr() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST_2,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected,"echo 8 >> 2;");
}

static void test_band() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected,"echo 8 & 10;");
}

static void test_bandnot() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST_2,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected,"echo 2 &^ 12;");
}

static void test_bxor() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected,"echo 8 ^ 2;");
}

static void test_bor() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_ICONST,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_PRINT,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "echo 8 | 2;");
}

int foldingtest(void) {
	test_neg();
	test_not();
	test_bnot();

	test_mul();
	test_idiv();
	test_mod();
	test_add();
	test_sub();
	test_bshl();
	test_bshr();
	test_band();
	test_bandnot();
	test_bxor();
	test_bor();

	return __YASL_TESTS_FAILED__;
}
