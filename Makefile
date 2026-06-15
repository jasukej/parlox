CC = gcc
CFLAGS = -Wall -Wextra -Isrc -g -fsanitize=address
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, out/%.o, $(SRC))
TARGET = out/binterpreter
CPPCHECK ?= cppcheck
CPPCHECK_FLAGS ?= --enable=warning,style,performance,portability --error-exitcode=1 --inline-suppr

GC_LIB = gc/target/debug/libgc.a

$(GC_LIB): 
	cargo build --manifest-path gc/Cargo.toml

$(TARGET): $(OBJ) $(GC_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -ldl

out/%.o: src/%.c | out
	$(CC) $(CFLAGS) -c $< -o $@

out:
	mkdir -p out

TEST_TARGET = out/binterpreter-test
TEST_OBJ = $(patsubst src/%.c, out/test_%.o, $(SRC))

$(TEST_TARGET): $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

out/test_%.o: src/%.c | out
	$(CC) $(CFLAGS) -DNDEBUG_TRACE -c $< -o $@

test: $(TEST_TARGET)
	@BINTERPRETER=$(TEST_TARGET) ./tests/run_tests.sh

lint: lint-c

lint-c: 
	$(CPPCHECK) $(CPPCHECK_FLAGS) src

clean:
	rm -rf out
