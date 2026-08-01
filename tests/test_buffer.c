#include <stdbool.h>
#include <stdio.h>

#include "buffer.h"

#define TEST_ASSERT(condition)                                             \
    do {                                                                    \
        if (!(condition)) {                                                  \
            fprintf(stderr, "%s:%d: assertion failed: %s\n",               \
                    __FILE__, __LINE__, #condition);                       \
            return false;                                                    \
        }                                                                    \
    } while (0)

static bool test_fifo_order(void)
{
    circular_buffer_struct(int, 3) buffer = {0};
    int value = 0;

    cb_init(buffer, int, 3);
    TEST_ASSERT(cb_empty(buffer));
    TEST_ASSERT(cb_size(buffer) == 3);

    cb_push(buffer, 10);
    cb_push(buffer, 20);
    cb_push(buffer, 30);

    TEST_ASSERT(cb_full(buffer));
    TEST_ASSERT(cb_occupation(buffer) == 3);

    cb_pop(buffer, value);
    TEST_ASSERT(value == 10);
    cb_pop(buffer, value);
    TEST_ASSERT(value == 20);
    cb_pop(buffer, value);
    TEST_ASSERT(value == 30);
    TEST_ASSERT(cb_empty(buffer));

    return true;
}

static bool test_overwrite_oldest(void)
{
    circular_buffer_struct(int, 3) buffer = {0};
    int value = 0;

    cb_init(buffer, int, 3);
    cb_push(buffer, 1);
    cb_push(buffer, 2);
    cb_push(buffer, 3);
    cb_push(buffer, 4);

    TEST_ASSERT(cb_full(buffer));
    TEST_ASSERT(cb_occupation(buffer) == 3);

    cb_pop(buffer, value);
    TEST_ASSERT(value == 2);
    cb_pop(buffer, value);
    TEST_ASSERT(value == 3);
    cb_pop(buffer, value);
    TEST_ASSERT(value == 4);
    TEST_ASSERT(cb_empty(buffer));

    return true;
}

static bool test_reset(void)
{
    circular_buffer_struct(int, 2) buffer = {0};
    int value = 0;

    cb_init(buffer, int, 2);
    cb_push(buffer, 7);
    cb_reset(buffer);

    TEST_ASSERT(cb_empty(buffer));
    TEST_ASSERT(cb_occupation(buffer) == 0);
    cb_pop(buffer, value);
    TEST_ASSERT(cb_empty(buffer));

    return true;
}

struct test_case {
    const char *name;
    bool (*run)(void);
};

int main(void)
{
    const struct test_case tests[] = {
        {"fifo_order", test_fifo_order},
        {"overwrite_oldest", test_overwrite_oldest},
        {"reset", test_reset},
    };
    size_t passed = 0;

    for (size_t index = 0; index < sizeof(tests) / sizeof(tests[0]); ++index) {
        if (!tests[index].run()) {
            fprintf(stderr, "FAIL %s\n", tests[index].name);
            return 1;
        }
        ++passed;
        printf("PASS %s\n", tests[index].name);
    }

    printf("%zu/%zu buffer tests passed\n", passed,
           sizeof(tests) / sizeof(tests[0]));
    return 0;
}
