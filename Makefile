CC := gcc
CFLAGS := -Wall -Wextra -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -std=c11 -pedantic -g
CPPFLAGS := -MMD -MP

BUILD ?= debug

ifeq ($(BUILD),debug)
    CFLAGS += -O0 -g3 -DDEBUG
else ifeq ($(BUILD),release)
    CFLAGS += -O2 -DNDEBUG
else
    $(error Unknown BUILD=$(BUILD). Use 'debug' or 'release')
endif

ifeq ($(SANITIZE),1)
    CFLAGS += -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
endif

SRC_DIR := .
BUILD_DIR := .build
BACKUP_DIR := .backup
TARGET := clox

TEST_DIR := tests
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_TARGET := test_clox

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.DEFAULT_GOAL := all

-include $(DEPS)

.PHONY: all build run test debug release clean distclean backup restore clear-backup help

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	@mkdir -p $@

run: build
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(filter-out $(SRC_DIR)/main.c,$(SRCS)) $(TEST_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^


debug:
	$(MAKE) BUILD=debug

release:
	$(MAKE) BUILD=release

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)
	rm -f *.o *.d

distclean: clean

backup:
	@mkdir -p $(BACKUP_DIR)
	@cp $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(BACKUP_DIR)/
	@echo "Backup saved to $(BACKUP_DIR)/"

restore:
	@if [ ! -d "$(BACKUP_DIR)" ]; then \
		echo "Error: no backup found in $(BACKUP_DIR)/"; \
		exit 1; \
	fi
	@cp $(BACKUP_DIR)/*.c $(BACKUP_DIR)/*.h $(SRC_DIR)/ 2>/dev/null || true
	@echo "Restored source files from $(BACKUP_DIR)/"
	@$(MAKE) clean
	@$(MAKE) build

clear-backup:
	@rm -rf $(BACKUP_DIR)
	@echo "Backup directory $(BACKUP_DIR)/ removed"

help:
	@echo "Usage: make [target] [VAR=value...]"
	@echo ""
	@echo "Targets:"
	@echo "  all       Build clox (default)"
	@echo "  build     Same as all"
	@echo "  run       Build and run clox"
	@echo "  debug     Build with debug symbols and assertions"
	@echo "  release   Build optimized release binary"
	@echo "  clean     Remove build directory and executable"
	@echo "  distclean Same as clean"
	@echo "  backup    Save a copy of all .c/.h files to $(BACKUP_DIR)/"
	@echo "  restore   Restore source files from $(BACKUP_DIR)/ and rebuild"
	@echo "  clear-backup Remove the backup directory"
	@echo "  help      Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD=debug|release   Build mode (default: debug)"
	@echo "  SANITIZE=1            Enable ASan/UBSan"
	@echo "  CC=gcc                C compiler"
	@echo "  CFLAGS=...            Extra compiler flags"
