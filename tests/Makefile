CC ?= cc
CPPFLAGS ?= -I../inc
CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -g
LDFLAGS ?=

TEST_BUFFER := test_buffer
TEST_LOGGER := test_logger

.PHONY: all test ubsan sanitizers clean

all: test

test: $(TEST_BUFFER) $(TEST_LOGGER)
	./$(TEST_BUFFER)
	./$(TEST_LOGGER)

$(TEST_BUFFER): test_buffer.c ../inc/buffer.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ test_buffer.c

$(TEST_LOGGER): test_logger.c ../src/logger.c ../inc/logger.h ../inc/buffer.h ../inc/channel.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ test_logger.c ../src/logger.c

ubsan: CFLAGS += -fsanitize=undefined -fno-omit-frame-pointer
ubsan: LDFLAGS += -fsanitize=undefined
ubsan: clean test

sanitizers: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
sanitizers: LDFLAGS += -fsanitize=address,undefined
sanitizers: clean test

clean:
	rm -f $(TEST_BUFFER) $(TEST_LOGGER)
