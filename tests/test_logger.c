#include <stdbool.h>
#include <stdio.h>

#include "logger.h"

#define TEST_ASSERT(condition)                                             \
    do {                                                                    \
        if (!(condition)) {                                                  \
            fprintf(stderr, "%s:%d: assertion failed: %s\n",               \
                    __FILE__, __LINE__, #condition);                       \
            return false;                                                    \
        }                                                                    \
    } while (0)

static int init_calls;
static int signal_calls;

static op_res_t fake_init(void)
{
    ++init_calls;
    return SUCCESS;
}

static void fake_start(char *destination)
{
    (void)destination;
}

static void fake_stop(void)
{
}

static op_res_t fake_write(data_t *data)
{
    (void)data;
    return SUCCESS;
}

static void fake_wait(void)
{
}

static void fake_signal(void)
{
    ++signal_calls;
}

static void reset_fixture(base_logger_t *logger,
                          data_buffer_t *buffer,
                          base_channel_t **channels)
{
    *buffer = (data_buffer_t){0};
    *logger = (base_logger_t){
        ._init = fake_init,
        ._start = fake_start,
        ._stop = fake_stop,
        ._write = fake_write,
        ._wait = fake_wait,
        ._signal = fake_signal,
        ._lock = NULL,
        ._unlock = NULL,
        .channels = channels,
        .write_buffer = buffer,
    };
    init_calls = 0;
    signal_calls = 0;
}

static bool test_channel_registration_and_reuse(void)
{
    base_logger_t logger;
    data_buffer_t buffer;
    base_channel_t *channels[3] = {0};
    base_channel_t first = {0};
    base_channel_t second = {0};
    base_channel_t third = {0};
    base_channel_t replacement = {0};

    reset_fixture(&logger, &buffer, channels);
    logger.write_buffer = &buffer;
    logger.channels = channels;
    logger_init(&logger, 3);

    TEST_ASSERT(init_calls == 1);
    TEST_ASSERT(logger.status == IDLE);
    TEST_ASSERT(logger_register(&logger, "first", &first) == &first);
    TEST_ASSERT(logger_register(&logger, "second", &second) == &second);
    TEST_ASSERT(logger_register(&logger, "third", &third) == &third);
    TEST_ASSERT(first.id == 1);
    TEST_ASSERT(second.id == 2);
    TEST_ASSERT(third.id == 3);
    TEST_ASSERT(logger_register(&logger, "full", &replacement) == NULL);

    logger_unregister(&logger, &second);
    TEST_ASSERT(second.id == 0);
    TEST_ASSERT(channels[1] == NULL);
    TEST_ASSERT(logger_register(&logger, "replacement", &replacement) == &replacement);
    TEST_ASSERT(replacement.id == 2);
    TEST_ASSERT(channels[1] == &replacement);

    return true;
}

static bool test_async_enqueue_uses_circular_buffer(void)
{
    base_logger_t logger;
    data_buffer_t buffer;
    base_channel_t *channels[1] = {0};
    uint8_t payload = 42;
    data_t queued = {0};

    reset_fixture(&logger, &buffer, channels);
    logger_init(&logger, 1);
    cb_init(buffer, data_t, WRITE_BUFFER_SIZE);

    logger_write_async(&logger, 1, &payload, sizeof(payload), NULL);

    TEST_ASSERT(signal_calls == 1);
    TEST_ASSERT(cb_occupation(buffer) == 1);
    cb_pop(buffer, queued);
    TEST_ASSERT(queued.id == 1);
    TEST_ASSERT(queued.data == &payload);
    TEST_ASSERT(queued.size == sizeof(payload));

    return true;
}

struct test_case {
    const char *name;
    bool (*run)(void);
};

int main(void)
{
    const struct test_case tests[] = {
        {"channel_registration_and_reuse", test_channel_registration_and_reuse},
        {"async_enqueue_uses_circular_buffer", test_async_enqueue_uses_circular_buffer},
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

    printf("%zu/%zu logger tests passed\n", passed,
           sizeof(tests) / sizeof(tests[0]));
    return 0;
}
