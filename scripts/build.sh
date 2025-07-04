#!/bin/bash
# Unified Shell Build Script
# Cross-platform build automation

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
INSTALL=false
CLEAN=false
USE_CMAKE=false
VERBOSE=false

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show help
show_help() {
    cat << EOF
Unified Shell Build Script

Usage: $0 [OPTIONS]

Options:
    -h, --help          Show this help message
    -d, --debug         Build in debug mode
    -r, --release       Build in release mode (default)
    -i, --install       Install after building
    -c, --clean         Clean build artifacts before building
    -m, --cmake         Use CMake instead of Make
    -v, --verbose       Enable verbose output
    --check-deps        Check dependencies and exit
    --setup-dev         Setup development environment

Examples:
    $0                      # Build release version with Make
    $0 --debug --cmake      # Build debug version with CMake
    $0 --clean --install    # Clean, build, and install
    $0 --check-deps         # Check if all dependencies are available

EOF
}

# Function to detect OS
detect_os() {
    case "$(uname -s)" in
        Linux*)     OS="Linux";;
        Darwin*)    OS="macOS";;
        CYGWIN*)    OS="Windows";;
        MINGW*)     OS="Windows";;
        MSYS*)      OS="Windows";;
        *)          OS="Unknown";;
    esac
    print_info "Detected OS: $OS"
}

# Function to check dependencies
check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for compiler
    if command -v g++ &> /dev/null; then
        print_success "g++ compiler found: $(g++ --version | head -n1)"
    elif command -v clang++ &> /dev/null; then
        print_success "clang++ compiler found: $(clang++ --version | head -n1)"
    else
        print_error "No C++ compiler found! Please install g++ or clang++"
        return 1
    fi
    
    # Check for make or cmake
    if [ "$USE_CMAKE" = true ]; then
        if command -v cmake &> /dev/null; then
            print_success "CMake found: $(cmake --version | head -n1)"
        else
            print_error "CMake not found! Please install CMake"
            return 1
        fi
    else
        if command -v make &> /dev/null; then
            print_success "Make found: $(make --version | head -n1)"
        else
            print_error "Make not found! Please install make"
            return 1
        fi
    fi
    
    # Check for readline
    if pkg-config --exists readline 2>/dev/null; then
        print_success "Readline found via pkg-config"
    elif [ -f "/usr/include/readline/readline.h" ] || [ -f "/usr/local/include/readline/readline.h" ]; then
        print_success "Readline headers found"
    else
        print_warning "Readline not found. The shell will have limited functionality."
        case "$OS" in
            Linux)
                print_info "Install with: sudo apt-get install libreadline-dev"
                ;;
            macOS)
                print_info "Install with: brew install readline"
                ;;
            Windows)
                print_info "Install with: pacman -S mingw-w64-ucrt-x86_64-readline"
                ;;
        esac
    fi
    
    print_success "Dependency check complete!"
}

# Function to setup development environment
setup_dev() {
    print_info "Setting up development environment for $OS..."
    
    case "$OS" in
        Linux)
            print_info "Installing dependencies for Linux..."
            if command -v apt-get &> /dev/null; then
                sudo apt-get update
                sudo apt-get install -y build-essential libreadline-dev cmake
            elif command -v yum &> /dev/null; then
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y readline-devel cmake
            elif command -v pacman &> /dev/null; then
                sudo pacman -S --needed base-devel readline cmake
            else
                print_error "Unknown package manager. Please install build tools manually."
                return 1
            fi
            ;;
        macOS)
            print_info "Installing dependencies for macOS..."
            if command -v brew &> /dev/null; then
                brew install readline cmake
            else
                print_error "Homebrew not found. Please install Homebrew first."
                return 1
            fi
            ;;
        Windows)
            print_info "Installing dependencies for Windows (MSYS2)..."
            if command -v pacman &> /dev/null; then
                pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-readline mingw-w64-ucrt-x86_64-cmake
            else
                print_error "MSYS2 not found. Please install MSYS2 first."
                return 1
            fi
            ;;
        *)
            print_error "Unknown OS. Please install dependencies manually."
            return 1
            ;;
    esac
    
    print_success "Development environment setup complete!"
}

# Function to clean build artifacts
clean_build() {
    print_info "Cleaning build artifacts..."
    
    if [ "$USE_CMAKE" = true ]; then
        rm -rf build/
        print_success "CMake build directory cleaned"
    else
        make clean 2>/dev/null || true
        print_success "Make artifacts cleaned"
    fi
}

# Function to build with Make
build_with_make() {
    print_info "Building with Make ($BUILD_TYPE mode)..."
    
    if [ "$BUILD_TYPE" = "Debug" ]; then
        make debug ${VERBOSE:+-j$(nproc)}
    else
        make ${VERBOSE:+-j$(nproc)}
    fi
    
    print_success "Build completed successfully!"
}

# Function to build with CMake
build_with_cmake() {
    print_info "Building with CMake ($BUILD_TYPE mode)..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure
    cmake_args="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    if [ "$VERBOSE" = true ]; then
        cmake_args="$cmake_args -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi
    
    cmake .. $cmake_args
    
    # Build
    if [ "$VERBOSE" = true ]; then
        cmake --build . --config "$BUILD_TYPE" -- -j$(nproc)
    else
        cmake --build . --config "$BUILD_TYPE"
    fi
    
    cd ..
    print_success "CMake build completed successfully!"
}

# Function to install
install_binary() {
    print_info "Installing unified_shell..."
    
    if [ "$USE_CMAKE" = true ]; then
        cd build
        cmake --install .
        cd ..
    else
        make install
    fi
    
    print_success "Installation completed!"
}

# Function to run post-build tests
run_tests() {
    print_info "Running basic tests..."
    
    binary_path="./unified_shell"
    if [ "$USE_CMAKE" = true ]; then
        binary_path="./build/unified_shell"
    fi
    
    if [ -f "$binary_path" ]; then
        # Test version
        if $binary_path --version &>/dev/null; then
            print_success "Version test passed"
        else
            print_warning "Version test failed"
        fi
        
        # Test help
        if $binary_path --help &>/dev/null; then
            print_success "Help test passed"
        else
            print_warning "Help test failed"
        fi
    else
        print_warning "Binary not found at $binary_path"
    fi
}

# Main function
main() {
    print_info "Starting Unified Shell build process..."
    
    # Detect OS
    detect_os
    
    # Clean if requested
    if [ "$CLEAN" = true ]; then
        clean_build
    fi
    
    # Check dependencies
    check_dependencies
    
    # Build
    if [ "$USE_CMAKE" = true ]; then
        build_with_cmake
    else
        build_with_make
    fi
    
    # Run tests
    run_tests
    
    # Install if requested
    if [ "$INSTALL" = true ]; then
        install_binary
    fi
    
    print_success "Build process completed successfully!"
    print_info "You can now run the unified shell with: ./unified_shell"
    if [ "$INSTALL" = true ]; then
        print_info "Or simply: unified_shell (if installed to PATH)"
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -m|--cmake)
            USE_CMAKE=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --check-deps)
            detect_os
            check_dependencies
            exit 0
            ;;
        --setup-dev)
            detect_os
            setup_dev
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Run main function
main