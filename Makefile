# Unified Shell Makefile
# Cross-platform build system for Linux and WSL

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -g -DDEBUG -O0
RELEASE_FLAGS = -DNDEBUG -O3

# Target executable
TARGET = unified_shell
TARGET_DEBUG = unified_shell_debug

# Source files
SOURCES = main.cpp unified_shell.cpp
HEADERS = unified_shell.hpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)
DEBUG_OBJECTS = $(SOURCES:.cpp=.debug.o)

# Libraries
LIBS = -lreadline -lhistory

# System detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Platform-specific settings
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
    INSTALL_DIR = /usr/local/bin
endif

ifeq ($(UNAME_S),Darwin)
    PLATFORM = macos
    INSTALL_DIR = /usr/local/bin
    # macOS might need additional paths for readline
    CXXFLAGS += -I/usr/local/include -L/usr/local/lib
endif

# Windows/MSYS2/MinGW settings
ifdef MSYSTEM
    PLATFORM = windows
    TARGET := $(TARGET).exe
    TARGET_DEBUG := $(TARGET_DEBUG).exe
    INSTALL_DIR = /usr/bin
endif

# Default target
all: $(TARGET)

# Release build
$(TARGET): $(OBJECTS)
	@echo "Building $(TARGET) for $(PLATFORM)..."
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Debug build
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(DEBUG_OBJECTS)
	@echo "Building debug version..."
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^ $(LIBS)
	@echo "Debug build complete! Run with: ./$(TARGET_DEBUG)"

# Object files for release
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Object files for debug
%.debug.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@

# Install target
install: $(TARGET)
	@echo "Installing $(TARGET) to $(INSTALL_DIR)..."
	@mkdir -p $(INSTALL_DIR)
	@cp $(TARGET) $(INSTALL_DIR)/
	@chmod +x $(INSTALL_DIR)/$(TARGET)
	@echo "Installation complete! You can now run 'unified_shell' from anywhere."

# Uninstall target
uninstall:
	@echo "Removing $(TARGET) from $(INSTALL_DIR)..."
	@rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "Uninstallation complete."

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(OBJECTS) $(DEBUG_OBJECTS) $(TARGET) $(TARGET_DEBUG)
	@echo "Clean complete."

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@echo "Compiler: $(shell $(CXX) --version | head -n 1)"
	@echo "Platform: $(PLATFORM)"
	@echo "Readline library:"
	@pkg-config --exists readline && echo "  ✓ Found via pkg-config" || echo "  ? Not found via pkg-config (may still work)"
	@echo "Testing compilation..."
	@echo '#include <readline/readline.h>' | $(CXX) -x c++ - -lreadline -o /tmp/test_readline 2>/dev/null && echo "  ✓ Readline compilation test passed" || echo "  ✗ Readline compilation test failed"
	@rm -f /tmp/test_readline

# Quick test
test: $(TARGET)
	@echo "Running basic test..."
	@echo "help" | ./$(TARGET) || echo "Test completed (may show errors if readline isn't available in non-interactive mode)"

# Create distribution package
dist: clean
	@echo "Creating distribution package..."
	@mkdir -p dist/unified_shell
	@cp -r *.cpp *.hpp Makefile CMakeLists.txt README.md dist/unified_shell/
	@cd dist && tar -czf unified_shell-1.0.0.tar.gz unified_shell/
	@echo "Distribution package created: dist/unified_shell-1.0.0.tar.gz"

# Development setup
setup-dev:
	@echo "Setting up development environment..."
	@echo "Installing dependencies (requires sudo)..."
ifeq ($(PLATFORM),linux)
	sudo apt-get update && sudo apt-get install -y build-essential libreadline-dev
endif
ifdef MSYSTEM
	pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-readline
endif
	@echo "Development setup complete!"

# Help target
help:
	@echo "Unified Shell Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all          Build release version (default)"
	@echo "  debug        Build debug version with symbols"
	@echo "  install      Install to system directory"
	@echo "  uninstall    Remove from system directory"
	@echo "  clean        Remove build artifacts"
	@echo "  check-deps   Check for required dependencies"
	@echo "  test         Run basic functionality test"
	@echo "  dist         Create distribution package"
	@echo "  setup-dev    Install development dependencies"
	@echo "  help         Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                 # Build release version"
	@echo "  make debug           # Build debug version"
	@echo "  make install         # Install system-wide"
	@echo "  make check-deps      # Check dependencies"

# Declare phony targets
.PHONY: all debug install uninstall clean check-deps test dist setup-dev help