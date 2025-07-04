#!/bin/bash
# Unified Shell Launcher Script
# Provides real-time terminal session support and easy deployment

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
SHELL_NAME="unified_shell"
CONFIG_DIR="$HOME/.unified_shell"
SESSION_DIR="$CONFIG_DIR/sessions"
LOG_DIR="$CONFIG_DIR/logs"

# Ensure directories exist
mkdir -p "$CONFIG_DIR" "$SESSION_DIR" "$LOG_DIR"

# Print functions
print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Help function
show_help() {
    cat << EOF
Unified Shell Launcher

Usage: $0 [COMMAND] [OPTIONS]

Commands:
    start [name]        Start a new shell session
    attach [name]       Attach to an existing session
    list                List all sessions
    kill [name]         Kill a session
    logs [name]         View session logs
    deploy              Deploy shell system-wide
    status              Show system status
    update              Update unified shell
    help                Show this help

Options:
    --debug             Enable debug mode
    --config [file]     Use custom config file
    --log-level [level] Set logging level (error|warn|info|debug)

Session Management:
    $0 start myproject          # Start session named 'myproject'
    $0 attach myproject         # Attach to 'myproject' session
    $0 list                     # List all active sessions
    $0 kill myproject           # Kill 'myproject' session

Real-time Features:
    - Persistent sessions with screen/tmux
    - Session logging and replay
    - Multi-user support
    - Remote access via SSH

Examples:
    $0 start dev                # Start development session
    $0 attach dev               # Attach to development session
    $0 deploy --system-wide     # Install system-wide
    $0 status                   # Check system status

EOF
}

# Detect session manager
detect_session_manager() {
    if command -v tmux >/dev/null 2>&1; then
        echo "tmux"
    elif command -v screen >/dev/null 2>&1; then
        echo "screen"
    else
        echo "none"
    fi
}

# Find unified shell binary
find_shell_binary() {
    local binary=""
    
    # Check current directory
    if [ -f "./$SHELL_NAME" ]; then
        binary="./$SHELL_NAME"
    elif [ -f "./build/$SHELL_NAME" ]; then
        binary="./build/$SHELL_NAME"
    elif command -v "$SHELL_NAME" >/dev/null 2>&1; then
        binary="$SHELL_NAME"
    elif [ -f "/usr/local/bin/$SHELL_NAME" ]; then
        binary="/usr/local/bin/$SHELL_NAME"
    else
        print_error "Unified shell binary not found!"
        print_info "Please build the shell first or ensure it's in PATH"
        exit 1
    fi
    
    echo "$binary"
}

# Start a new session
start_session() {
    local session_name="${1:-default}"
    local session_manager="$(detect_session_manager)"
    local shell_binary="$(find_shell_binary)"
    local log_file="$LOG_DIR/$session_name.log"
    
    print_info "Starting session '$session_name' with $session_manager..."
    
    case "$session_manager" in
        "tmux")
            if tmux has-session -t "$session_name" 2>/dev/null; then
                print_warning "Session '$session_name' already exists. Use 'attach' to connect."
                return 1
            fi
            
            # Create tmux session with logging
            tmux new-session -d -s "$session_name" \
                "script -f '$log_file' -c '$shell_binary'"
            
            print_success "Session '$session_name' started successfully!"
            print_info "Attach with: $0 attach $session_name"
            ;;
            
        "screen")
            if screen -list | grep -q "$session_name"; then
                print_warning "Session '$session_name' already exists. Use 'attach' to connect."
                return 1
            fi
            
            # Create screen session with logging
            screen -dmS "$session_name" script -f "$log_file" -c "$shell_binary"
            
            print_success "Session '$session_name' started successfully!"
            print_info "Attach with: $0 attach $session_name"
            ;;
            
        "none")
            print_warning "No session manager found. Starting shell directly..."
            print_info "Install tmux or screen for session management"
            
            # Log to file and run shell
            script -f "$log_file" -c "$shell_binary"
            ;;
    esac
}

# Attach to existing session
attach_session() {
    local session_name="${1:-default}"
    local session_manager="$(detect_session_manager)"
    
    print_info "Attaching to session '$session_name'..."
    
    case "$session_manager" in
        "tmux")
            if ! tmux has-session -t "$session_name" 2>/dev/null; then
                print_error "Session '$session_name' not found!"
                print_info "Start it with: $0 start $session_name"
                return 1
            fi
            
            tmux attach-session -t "$session_name"
            ;;
            
        "screen")
            if ! screen -list | grep -q "$session_name"; then
                print_error "Session '$session_name' not found!"
                print_info "Start it with: $0 start $session_name"
                return 1
            fi
            
            screen -r "$session_name"
            ;;
            
        "none")
            print_error "No session manager available for attach operation"
            return 1
            ;;
    esac
}

# List all sessions
list_sessions() {
    local session_manager="$(detect_session_manager)"
    
    print_info "Active sessions:"
    
    case "$session_manager" in
        "tmux")
            if tmux list-sessions 2>/dev/null; then
                echo
            else
                print_info "No active tmux sessions"
            fi
            ;;
            
        "screen")
            if screen -list | grep -v "No Sockets"; then
                echo
            else
                print_info "No active screen sessions"
            fi
            ;;
            
        "none")
            print_warning "No session manager available"
            ;;
    esac
    
    # Show log files
    print_info "Available session logs:"
    if ls "$LOG_DIR"/*.log >/dev/null 2>&1; then
        ls -la "$LOG_DIR"/*.log | awk '{print "  " $9 " (" $5 " bytes, " $6 " " $7 " " $8 ")"}'
    else
        print_info "  No session logs found"
    fi
}

# Kill a session
kill_session() {
    local session_name="${1:-default}"
    local session_manager="$(detect_session_manager)"
    
    print_info "Killing session '$session_name'..."
    
    case "$session_manager" in
        "tmux")
            if tmux has-session -t "$session_name" 2>/dev/null; then
                tmux kill-session -t "$session_name"
                print_success "Session '$session_name' killed"
            else
                print_error "Session '$session_name' not found"
                return 1
            fi
            ;;
            
        "screen")
            if screen -list | grep -q "$session_name"; then
                screen -S "$session_name" -X quit
                print_success "Session '$session_name' killed"
            else
                print_error "Session '$session_name' not found"
                return 1
            fi
            ;;
            
        "none")
            print_error "No session manager available for kill operation"
            return 1
            ;;
    esac
}

# View session logs
view_logs() {
    local session_name="${1:-default}"
    local log_file="$LOG_DIR/$session_name.log"
    
    if [ -f "$log_file" ]; then
        print_info "Viewing logs for session '$session_name':"
        echo "----------------------------------------"
        
        if command -v less >/dev/null 2>&1; then
            less "$log_file"
        else
            cat "$log_file"
        fi
    else
        print_error "Log file not found: $log_file"
        print_info "Available logs:"
        ls -la "$LOG_DIR"/ 2>/dev/null || print_info "  No logs available"
    fi
}

# Deploy system-wide
deploy_system() {
    local shell_binary="$(find_shell_binary)"
    local install_dir="/usr/local/bin"
    local launcher_dir="/usr/local/bin"
    
    print_info "Deploying unified shell system-wide..."
    
    # Check permissions
    if [ ! -w "$install_dir" ]; then
        print_warning "Need sudo privileges for system-wide installation"
        if ! sudo -v; then
            print_error "Failed to get sudo privileges"
            return 1
        fi
    fi
    
    # Copy shell binary
    print_info "Installing shell binary to $install_dir..."
    sudo cp "$shell_binary" "$install_dir/"
    sudo chmod +x "$install_dir/$SHELL_NAME"
    
    # Copy launcher script
    print_info "Installing launcher script to $launcher_dir..."
    sudo cp "$0" "$launcher_dir/ushell"
    sudo chmod +x "$launcher_dir/ushell"
    
    # Create desktop entry (Linux)
    if command -v update-desktop-database >/dev/null 2>&1; then
        print_info "Creating desktop entry..."
        sudo tee /usr/share/applications/unified-shell.desktop > /dev/null << EOF
[Desktop Entry]
Name=Unified Shell
Comment=Cross-platform command execution environment
Exec=ushell start
Icon=terminal
Type=Application
Categories=System;TerminalEmulator;
Terminal=true
EOF
        update-desktop-database /usr/share/applications/
    fi
    
    print_success "System-wide deployment completed!"
    print_info "You can now use:"
    print_info "  $SHELL_NAME                # Direct shell access"
    print_info "  ushell start [name]        # Session management"
    print_info "  ushell attach [name]       # Attach to sessions"
}

# Show system status
show_status() {
    local shell_binary="$(find_shell_binary)"
    local session_manager="$(detect_session_manager)"
    
    print_info "Unified Shell System Status"
    echo "----------------------------------------"
    echo "Shell binary: $shell_binary"
    echo "Session manager: $session_manager"
    echo "Config directory: $CONFIG_DIR"
    echo "Log directory: $LOG_DIR"
    echo "OS: $(uname -s) $(uname -r)"
    echo
    
    # Check shell functionality
    print_info "Checking shell functionality..."
    if "$shell_binary" --version >/dev/null 2>&1; then
        print_success "Shell version check passed"
    else
        print_error "Shell version check failed"
    fi
    
    # Check session manager
    print_info "Checking session manager..."
    case "$session_manager" in
        "tmux"|"screen")
            print_success "Session manager '$session_manager' available"
            ;;
        "none")
            print_warning "No session manager found"
            print_info "Install tmux or screen for better session management"
            ;;
    esac
    
    # Show active sessions
    echo
    list_sessions
}

# Update unified shell
update_shell() {
    print_info "Updating unified shell..."
    
    if [ -f "./scripts/build.sh" ]; then
        print_info "Building latest version..."
        ./scripts/build.sh --clean
        print_success "Shell updated successfully!"
    else
        print_error "Build script not found. Please run from source directory."
        return 1
    fi
}

# Main function
main() {
    local command="${1:-help}"
    shift 2>/dev/null || true
    
    case "$command" in
        "start")
            start_session "$@"
            ;;
        "attach")
            attach_session "$@"
            ;;
        "list")
            list_sessions
            ;;
        "kill")
            kill_session "$@"
            ;;
        "logs")
            view_logs "$@"
            ;;
        "deploy")
            deploy_system "$@"
            ;;
        "status")
            show_status
            ;;
        "update")
            update_shell
            ;;
        "help"|"--help"|"-h")
            show_help
            ;;
        *)
            print_error "Unknown command: $command"
            echo
            show_help
            exit 1
            ;;
    esac
}

# Run main function
main "$@"