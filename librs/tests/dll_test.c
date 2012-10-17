/**
 * @file dll_test.c
 * @date 30 juil. 2012
 * @author nicolas.carrier@parrot.com
 * @brief Unit tests for rs_dll module
 *
 * Copyright (C) 2012 Parrot S.A.
 */
#include <limits.h>
#include <inttypes.h>

#include <CUnit/Basic.h>

#include "../include/rs_dll.h"
#include "../src/rs_utils.h"

#include "tests_common.h"

struct int_node {
	int val;
	rs_node_t node;
};

typedef struct int_node int_node_t;

#define to_int_node(p) container_of(p, int_node_t, node)

static int dll_test_equals(rs_node_t *a, const rs_node_t *b)
{
	int_node_t *int_node_a = to_int_node(a);
	int_node_t *int_node_b = to_int_node(b);

	if (NULL == a || NULL == b)
		return 0;

	return 0 == (int_node_a->val - int_node_b->val);
}

static const rs_dll_vtable_t dll_test_vtable = {
		.equals = dll_test_equals,
};

static void testRS_DLL_INIT(void)
{
	rs_dll_t dll;
	int f_ret = 0;

	/* normal use cases */
	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL(f_ret, 0);
	CU_ASSERT_EQUAL(dll.count, 0);
	CU_ASSERT_EQUAL(dll.cur, NULL);
	CU_ASSERT_EQUAL(dll.head, NULL);
	CU_ASSERT_EQUAL(dll.vtable.equals, dll_test_equals);
	CU_ASSERT_PTR_NOT_NULL(dll.vtable.print);
	f_ret = rs_dll_init(&dll, NULL);
	CU_ASSERT_EQUAL(f_ret, 0);
	CU_ASSERT_PTR_NOT_NULL(dll.vtable.equals);
	CU_ASSERT_PTR_NOT_NULL(dll.vtable.print);

	/* error use case */
	f_ret = rs_dll_init(NULL, &dll_test_vtable);
	CU_ASSERT_NOT_EQUAL(f_ret, 0);
}

static void testRS_DLL_PUSH(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	rs_dll_t dll;
	int two_times_cb(rs_node_t *node, void *data)
	{
		int_node_t *in = to_int_node(node);

		CU_ASSERT_EQUAL_FATAL((uint32_t)data, 0xdeadbeef);
		CU_ASSERT_PTR_NOT_NULL(node);

		in->val *= 2;

		return 0;
	};
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_foreach(&dll, two_times_cb, (void *)0xdeadbeef);
	CU_ASSERT_EQUAL(f_ret, 0);
	CU_ASSERT_EQUAL(int_node_a.val, 34);
	CU_ASSERT_EQUAL(int_node_b.val, 84);
	CU_ASSERT_EQUAL(int_node_c.val, 1332);

	/* error use case */
	f_ret = rs_dll_push(NULL, &(int_node_a.node));
	CU_ASSERT_NOT_EQUAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, NULL);
	CU_ASSERT_NOT_EQUAL(f_ret, 0);
}

static void testRS_DLL_GET_COUNT(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	unsigned count;
	rs_dll_t dll;
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 0);

	rs_dll_push(&dll, &(int_node_a.node));
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 1);

	rs_dll_push(&dll, &(int_node_b.node));
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 2);

	rs_dll_push(&dll, &(int_node_c.node));
	count = rs_dll_get_count(&dll);
	CU_ASSERT_EQUAL(count, 3);

	/* error use case */
	count = rs_dll_get_count(NULL);
	CU_ASSERT_EQUAL(count, UINT_MAX);
}

static void testRS_DLL_FIND(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	int_node_t int_needle;
	rs_node_t *node;
	rs_dll_t dll;
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	int_needle.val = 42;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	int_needle.val = 17;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	int_needle.val = 666;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	int_needle.val = 421;
	node = rs_dll_find(&dll, &(int_needle.node));
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_find(NULL, &(int_needle.node));
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_find(&dll, NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_FIND_MATCH(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	int match_cb(rs_node_t *node, const void *data)
	{
		int searched_value = (int)data;
		int_node_t *int_node = to_int_node(node);

		return int_node->val == searched_value;
	}
	rs_node_t *node;
	rs_dll_t dll;
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	node = rs_dll_find_match(&dll, match_cb, (void *)42);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_find_match(&dll, match_cb, (void *)17);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_find_match(&dll, match_cb, (void *)666);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_find_match(&dll, match_cb, (void *)421);
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_find_match(NULL, match_cb, (void *)17);
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_find_match(&dll, NULL, (void *)17);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_POP(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	rs_node_t *node;
	rs_dll_t dll;
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	node = rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_pop(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_pop(&dll);
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_pop(NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_NEXT(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	rs_node_t *node;
	rs_dll_t dll;
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 42);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);
	node = rs_dll_next(&dll);
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	/* error use case */
	node = rs_dll_next(NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_REMOVE(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	int_node_t int_node_needle = {.val = 42,};
	rs_node_t *node;
	rs_dll_t dll;
	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	node = rs_dll_remove(&dll, &(int_node_needle.node));
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);

	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);
	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);

	int_node_needle.val = 17;
	node = rs_dll_remove(&dll, &(int_node_needle.node));
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);

	node = rs_dll_next(&dll);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	int_node_needle.val = 666;
	node = rs_dll_remove(&dll, &(int_node_needle.node));
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);

	node = rs_dll_next(&dll);
	CU_ASSERT_PTR_NULL(node);

	/* error use case */
	node = rs_dll_remove(NULL, &(int_node_needle.node));
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_remove(&dll, NULL);
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_REMOVE_MATCH(void)
{
	int_node_t int_node_a = {.val = 17,};
	int_node_t int_node_b = {.val = 42,};
	int_node_t int_node_c = {.val = 666,};
	int_node_t needle = {.val = 42,};
	rs_node_t *node;
	int odd;
	int parity_cb(rs_node_t *node, const void *data)
	{
		int odd = *((int *)data);
		int_node_t *int_node = to_int_node(node);

		if (odd)
			return int_node->val % 2;
		else
			return !(int_node->val % 2);
	};
	rs_dll_t dll;

	int f_ret = 0;

	f_ret = rs_dll_init(&dll, &dll_test_vtable);
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	f_ret = rs_dll_push(&dll, &(int_node_a.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_b.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);
	f_ret = rs_dll_push(&dll, &(int_node_c.node));
	CU_ASSERT_EQUAL_FATAL(f_ret, 0);

	/* normal use cases */
	/* remove the first even node */
	odd = 0;
	node = rs_dll_remove_match(&dll, parity_cb, &odd);
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 666);

	/* remove the first odd node */
	odd = 1;
	node = rs_dll_remove_match(&dll, parity_cb, &odd);
	CU_ASSERT_PTR_NOT_NULL_FATAL(node);
	CU_ASSERT_EQUAL(to_int_node(node)->val, 17);

	/* error use case */
	node = rs_dll_remove_match(NULL, parity_cb, &(needle.node));
	CU_ASSERT_PTR_NULL(node);
	node = rs_dll_remove_match(&dll, NULL, &(needle.node));
	CU_ASSERT_PTR_NULL(node);
}

static void testRS_DLL_FOREACH(void)
{
	rs_dll_t dll;
	int cb(rs_node_t __attribute__((unused))*node,
			void __attribute__((unused))*data)
	{
		/* do nothing */
		return 0;
	};
	int f_ret = 0;

	/* normal use case is tested in testRS_DLL_PUSH */

	/* error use cases */
	f_ret = rs_dll_foreach(NULL, cb, (void *)0xB16B00B5);
	CU_ASSERT_NOT_EQUAL(f_ret, 0);
	f_ret = rs_dll_foreach(&dll, NULL, (void *)0xB16B00B5);
	CU_ASSERT_NOT_EQUAL(f_ret, 0);
}

static const test_t tests[] = {
		{
				.fn = testRS_DLL_INIT,
				.name = "rs_dll_init"
		},
		{
				.fn = testRS_DLL_PUSH,
				.name = "rs_dll_push"
		},
		{
				.fn = testRS_DLL_GET_COUNT,
				.name = "rs_dll_get_count"
		},
		{
				.fn = testRS_DLL_FIND,
				.name = "rs_dll_find"
		},
		{
				.fn = testRS_DLL_FIND_MATCH,
				.name = "rs_dll_find_match"
		},
		{
				.fn = testRS_DLL_POP,
				.name = "rs_dll_pop"
		},
		{
				.fn = testRS_DLL_NEXT,
				.name = "rs_dll_next"
		},
		{
				.fn = testRS_DLL_REMOVE,
				.name = "rs_dll_remove"
		},
		{
				.fn = testRS_DLL_REMOVE_MATCH,
				.name = "rs_dll_remove_match"
		},
		{
				.fn = testRS_DLL_FOREACH,
				.name = "rs_dll_foreach"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_dll_suite(void)
{
	return 0;
}

static int clean_dll_suite(void)
{

	return 0;
}

suite_t dll_suite = {
		.name = "rs_dll",
		.init = init_dll_suite,
		.clean = clean_dll_suite,
		.tests = tests,
};