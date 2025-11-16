# Makefile for inits - A lightweight UNIX init system
# Licensed under GNU GPL 3.0

# Compiler and flags
CC = cc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic
LDFLAGS =

# Directories
SRC_DIR = src
INCLUDE_DIR = include
SCRIPTS_DIR = scripts
BUILD_DIR = build

# Installation directories
PREFIX = /usr/local
SBINDIR = $(PREFIX)/sbin
SYSCONFDIR = /etc
LOGDIR = /var/log

# Target binary
TARGET = inits

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Wrapper scripts
WRAPPERS = inits0 inits1 inits2 inits3 inits4 inits5 inits6 inits7 inits8 inits9

# Default target
all: $(TARGET) wrappers

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Link main binary
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

# Generate wrapper scripts
wrappers:
	@mkdir -p $(SCRIPTS_DIR)
	@for i in 0 1 2 3 4 5 6 7 8 9; do \
		echo "#!/bin/sh" > $(SCRIPTS_DIR)/inits$$i; \
		echo "# inits$$i - Init system wrapper for runlevel $$i" >> $(SCRIPTS_DIR)/inits$$i; \
		echo "# Licensed under GNU GPL 3.0" >> $(SCRIPTS_DIR)/inits$$i; \
		echo "" >> $(SCRIPTS_DIR)/inits$$i; \
		echo "RUNLEVEL=$$i" >> $(SCRIPTS_DIR)/inits$$i; \
		echo "export RUNLEVEL" >> $(SCRIPTS_DIR)/inits$$i; \
		echo "exec $(SBINDIR)/inits" >> $(SCRIPTS_DIR)/inits$$i; \
		chmod +x $(SCRIPTS_DIR)/inits$$i; \
	done
	@echo "Generated wrapper scripts in $(SCRIPTS_DIR)/"

# Install target
install: all
	@echo "Installing inits to $(SBINDIR)..."
	install -d $(SBINDIR)
	install -m 755 $(TARGET) $(SBINDIR)/$(TARGET)
	@echo "Installing wrapper scripts to $(SBINDIR)..."
	@for i in 0 1 2 3 4 5 6 7 8 9; do \
		install -m 755 $(SCRIPTS_DIR)/inits$$i $(SBINDIR)/inits$$i; \
	done
	@echo "Creating $(SYSCONFDIR)/inits.d directory..."
	install -d $(SYSCONFDIR)/inits.d
	@echo "Creating $(LOGDIR) directory..."
	install -d $(LOGDIR)
	@echo "Installation complete."

# Uninstall target
uninstall:
	@echo "Removing inits from $(SBINDIR)..."
	rm -f $(SBINDIR)/$(TARGET)
	@echo "Removing wrapper scripts from $(SBINDIR)..."
	@for i in 0 1 2 3 4 5 6 7 8 9; do \
		rm -f $(SBINDIR)/inits$$i; \
	done
	@echo "Note: $(SYSCONFDIR)/inits.d and $(LOGDIR) left intact."
	@echo "Uninstallation complete."

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -rf $(SCRIPTS_DIR)
	@echo "Clean complete."

# Phony targets
.PHONY: all wrappers install uninstall clean
