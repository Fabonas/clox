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
BACKUP_ROOT := .backup
BACKUP_DIR := $(BACKUP_ROOT)
TARGET := clox

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.DEFAULT_GOAL := all

-include $(DEPS)

.PHONY: all build run debug release clean distclean backup restore list-backups clear-backup help

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


debug:
	$(MAKE) BUILD=debug

release:
	$(MAKE) BUILD=release

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f *.o *.d

distclean: clean

ifdef NAME
BACKUP_DIR := $(BACKUP_ROOT)/$(NAME)
endif

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

list-backups:
	@if [ ! -d "$(BACKUP_ROOT)" ]; then \
		echo "No backup directory found"; \
		exit 0; \
	fi
	@echo "Named backups in $(BACKUP_ROOT)/:"
	@found=0; \
	for entry in $(BACKUP_ROOT)/*; do \
		if [ -d "$$entry" ]; then \
			found=1; \
			echo "  $$(basename $$entry)"; \
		fi; \
	done; \
	if [ $$found -eq 0 ]; then \
		echo "  (none)"; \
	fi
	@echo "Default unnamed backup: $(BACKUP_ROOT)/"

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
	@echo "            Use NAME=<name> to create a named backup in $(BACKUP_ROOT)/<name>/"
	@echo "  restore   Restore source files from $(BACKUP_DIR)/"
	@echo "            Use NAME=<name> to restore from $(BACKUP_ROOT)/<name>/"
	@echo "  list-backups List all named backups"
	@echo "  clear-backup Remove the backup directory (or NAME=<name> for a named one)"
	@echo "  help      Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD=debug|release   Build mode (default: debug)"
	@echo "  SANITIZE=1            Enable ASan/UBSan"
	@echo "  CC=gcc                C compiler"
	@echo "  CFLAGS=...            Extra compiler flags"
