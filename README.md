# 🚀 Unified Shell - Cross-Platform Command Execution Environment

A unified shell system that executes both Windows and Linux commands seamlessly, regardless of your operating system. Perfect for developers, system administrators, and anyone working in mixed environments.

## ✨ Key Features

### 🌐 Cross-Platform Command Support
- **Windows Commands**: Execute `dir`, `copy`, `del`, PowerShell commands
- **Linux Commands**: Execute `ls`, `cp`, `rm`, and all standard Unix tools
- **Auto-Detection**: Automatically detects command type and routes to appropriate executor
- **Mode Switching**: Manual mode selection (Windows/Linux/Auto-detect)

### 🔧 Advanced Shell Features
- **Command History**: Persistent command history across sessions
- **Tab Completion**: Smart auto-completion using GNU Readline
- **Background Jobs**: Execute commands with `&` for background processing
- **Redirection**: Support for `>`, `>>`, `<` operators
- **Pipelines**: Full pipeline support with `|` operator
- **Built-in Commands**: `cd`, `export`, `history`, `help`, and more

### 🎯 Smart Command Execution
- **OS Detection**: Automatically detects Windows, Linux, WSL, or macOS
- **WSL Integration**: Seamless Linux command execution on Windows via WSL
- **Command Validation**: Validates commands before execution with helpful suggestions
- **Error Handling**: Graceful error handling with meaningful error messages

### 🛠️ Developer-Friendly
- **Real-time Sessions**: Support for tmux/screen integration
- **Session Management**: Persistent shell sessions with logging
- **Configuration**: Customizable settings and preferences
- **Debug Mode**: Comprehensive debugging and logging support

## 📋 System Requirements

### Minimum Requirements
- **OS**: Windows 10/11, Linux (Ubuntu 18.04+), macOS 10.15+
- **RAM**: 512 MB
- **Storage**: 50 MB free space
- **Compiler**: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)

### Dependencies
- **GNU Readline** (for command-line editing)
- **Build Tools**: make or CMake
- **WSL 2** (Windows only, for Linux command execution)

## 🚀 Quick Start

### Option 1: Automated Build (Recommended)

```bash
# Linux/macOS/WSL
./scripts/build.sh

# Windows
scripts\build.bat

# With options
./scripts/build.sh --debug --cmake --install
```

### Option 2: Manual Build

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential libreadline-dev

# Build
make
# or
cmake -B build && cmake --build build

# Run
./unified_shell
```

### Option 3: Session Management

```bash
# Start a named session
./scripts/launcher.sh start myproject

# Attach to session
./scripts/launcher.sh attach myproject

# List sessions
./scripts/launcher.sh list

# Deploy system-wide
./scripts/launcher.sh deploy
```

## 🎮 Usage Examples

### Basic Command Execution

```bash
# Auto-detect mode (default)
[AUTO]> ls -la          # Executes as Linux command
[AUTO]> dir /w          # Executes as Windows command

# Manual mode switching
[AUTO]> mode windows
[WIN]> dir
[WIN]> mode linux
[LNX]> ls -la
```

### Advanced Features

```bash
# Background execution
[AUTO]> ping google.com &

# Pipelines
[AUTO]> ls -la | grep ".cpp"

# Redirection
[AUTO]> ls > files.txt
[AUTO]> cat < input.txt > output.txt

# Command history
[AUTO]> history
[AUTO]> !5              # Execute command #5 from history
```

### Built-in Commands

```bash
[AUTO]> help            # Show available commands
[AUTO]> status          # Show system status
[AUTO]> config          # Configure shell settings
[AUTO]> clear           # Clear screen
[AUTO]> exit            # Exit shell
```

## 📁 Project Structure

```
unified_shell/
├── main.cpp                    # Application entry point
├── unified_shell.hpp           # Core shell interface
├── unified_shell.cpp           # Shell implementation
├── shell.cpp                   # Original shell (legacy)
├── Makefile                    # Build configuration
├── CMakeLists.txt             # CMake configuration
├── scripts/
│   ├── build.sh               # Cross-platform build script
│   ├── build.bat              # Windows build script
│   └── launcher.sh            # Session management script
├── DEPLOYMENT.md              # Detailed deployment guide
└── README.md                  # This file
```

## 🔧 Configuration

### First Run Setup
1. Launch: `./unified_shell`
2. Choose command mode (Windows/Linux/Auto-detect)
3. Settings are saved automatically

### Configuration File
- **Linux/macOS**: `~/.unified_shell_config`
- **Windows**: `%USERPROFILE%\.unified_shell_config`

```ini
# Example configuration
default_mode=auto_detect
auto_detect=true
save_preferences=true
```

## 🌍 Platform Support

| Platform | Windows Commands | Linux Commands | WSL Integration | Notes |
|----------|------------------|----------------|-----------------|-------|
| **Windows 10/11** | ✅ Native | ✅ via WSL | ✅ Automatic | Requires WSL for Linux commands |
| **Linux** | ✅ via Wine/cmd.exe | ✅ Native | ➖ N/A | Limited Windows command support |
| **macOS** | ⚠️ Limited | ✅ Native | ➖ N/A | Basic Windows command support |
| **WSL** | ✅ Native | ✅ Native | ✅ Built-in | Best of both worlds |

## 📖 Advanced Usage

### Real-time Terminal Sessions

```bash
# Using tmux
tmux new-session 'unified_shell'

# Using screen
screen -S shell_session unified_shell

# Using our launcher
./scripts/launcher.sh start development
./scripts/launcher.sh attach development
```

### Integration with IDEs

#### Visual Studio Code
```json
{
    "terminal.integrated.profiles.linux": {
        "Unified Shell": {
            "path": "/usr/local/bin/unified_shell"
        }
    }
}
```

#### JetBrains IDEs
Set shell path in Settings → Tools → Terminal

### Container Deployment

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential libreadline-dev
COPY . /app
WORKDIR /app
RUN make && make install
ENTRYPOINT ["unified_shell"]
```

## 🐛 Troubleshooting

### Common Issues

**Build Errors**
```bash
./scripts/build.sh --check-deps  # Check dependencies
./scripts/build.sh --clean --verbose  # Clean rebuild
```

**Readline Issues**
```bash
# Ubuntu/Debian
sudo apt-get install libreadline-dev

# macOS
brew install readline

# Windows/MSYS2
pacman -S mingw-w64-ucrt-x86_64-readline
```

**WSL Integration**
```powershell
wsl --install     # Install WSL if missing
wsl --update      # Update WSL
```

### Debug Mode
```bash
unified_shell --debug  # Enable debug output
[AUTO]> status         # Check system status
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

### Development Setup
```bash
./scripts/build.sh --setup-dev  # Install dependencies
./scripts/build.sh --debug      # Build debug version
make test                       # Run tests
```

## 📄 License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **GNU Readline** for command-line editing capabilities
- **WSL Team** for seamless Windows-Linux integration
- **Open Source Community** for inspiration and feedback

## 🔗 Links

- **Documentation**: [DEPLOYMENT.md](DEPLOYMENT.md)
- **Issues**: [GitHub Issues](https://github.com/your-repo/unified_shell/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-repo/unified_shell/discussions)

---

Made with ❤️ for cross-platform development
