CC := cc
CSTD := -std=c11
WARN := -Wall -Wextra
INC := -Iinclude

RELFLAGS := $(CSTD) $(WARN) $(INC) -O2
SANFLAGS := $(CSTD) $(WARN) $(INC) -g -O0 -fsanitize=address,undefined

BUILD := build
OBJ_REL := $(BUILD)/obj
OBJ_SAN := $(BUILD)/obj-sanitize

SRC := $(wildcard src/*.c)
MAIN_SRC := src/main.c
LIB_SRC := $(filter-out $(MAIN_SRC),$(SRC))

LIB_OBJ_REL := $(patsubst src/%.c,$(OBJ_REL)/%.o,$(LIB_SRC))
MAIN_OBJ_REL := $(patsubst src/%.c,$(OBJ_REL)/%.o,$(MAIN_SRC))

LIB_OBJ_SAN := $(patsubst src/%.c,$(OBJ_SAN)/%.o,$(LIB_SRC))
MAIN_OBJ_SAN := $(patsubst src/%.c,$(OBJ_SAN)/%.o,$(MAIN_SRC))

TEST_SRC := $(wildcard test/*.c)
TEST_BIN := $(patsubst test/%.c,$(BUILD)/%,$(TEST_SRC))

.PHONY: all sanitize test clean

all: $(BUILD)/db

$(BUILD)/db: $(LIB_OBJ_REL) $(MAIN_OBJ_REL) | $(BUILD)
	$(CC) $(RELFLAGS) $^ -o $@

sanitize: $(LIB_OBJ_SAN) $(MAIN_OBJ_SAN) | $(BUILD)
	$(CC) $(SANFLAGS) $^ -o $(BUILD)/db

$(OBJ_REL)/%.o: src/%.c | $(OBJ_REL)
	$(CC) $(RELFLAGS) -c $< -o $@

$(OBJ_SAN)/%.o: src/%.c | $(OBJ_SAN)
	$(CC) $(SANFLAGS) -c $< -o $@

$(BUILD)/test_%: test/test_%.c $(LIB_OBJ_SAN) | $(BUILD)
	$(CC) $(SANFLAGS) $< $(LIB_OBJ_SAN) -o $@

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do \
		echo "== $$t =="; \
		$$t || exit 1; \
	done

$(BUILD) $(OBJ_REL) $(OBJ_SAN):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
