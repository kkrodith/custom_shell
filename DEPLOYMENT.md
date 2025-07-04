# Unified Shell - Deployment Guide

## 🚀 Overview

The Unified Shell is a cross-platform command execution environment that allows you to run both Windows and Linux commands regardless of your underlying operating system. This guide will walk you through building, deploying, and using the system.

## 📋 System Requirements

### Minimum Requirements
- **OS**: Windows 10/11, Linux (Ubuntu 18.04+, CentOS 7+), macOS 10.15+
- **RAM**: 512 MB
- **Storage**: 50 MB free space
- **Compiler**: C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Dependencies
- **readline library** (for command history and tab completion)
- **Build tools**: make or CMake
- **For Windows**: WSL 2 (optional, for Linux command execution)

## 🛠️ Building from Source

### Option 1: Quick Build (Linux/macOS/WSL)

```bash
# Clone or extract the source
cd unified_shell

# Make the build script executable
chmod +x scripts/build.sh

# Build with default settings
./scripts/build.sh

# Or build with specific options
./scripts/build.sh --debug --cmake --install
```

### Option 2: Windows Build

```cmd
REM For Windows Command Prompt or PowerShell
cd unified_shell
scripts\build.bat

REM For MSYS2 terminal
./scripts/build.sh
```

### Option 3: Manual Build

#### Using Make (Linux/macOS/WSL)

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential libreadline-dev

# Build release version
make

# Build debug version
make debug

# Install system-wide
sudo make install
```

#### Using CMake (Cross-platform)

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Install (optional)
cmake --install .
```

#### Windows with MSYS2

```bash
# Install MSYS2 dependencies
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-readline make

# Build
make

# Or use CMake
mkdir build && cd build
cmake .. -G "MSYS Makefiles"
cmake --build .
```

## 📦 Pre-built Binaries

### Download Options

1. **GitHub Releases**: Check the releases page for pre-compiled binaries
2. **Package Managers** (planned):
   - Linux: `apt install unified-shell` (Ubuntu), `yum install unified-shell` (CentOS)
   - macOS: `brew install unified-shell`
   - Windows: `winget install unified-shell`

### Installation from Binary

```bash
# Linux/macOS
sudo cp unified_shell /usr/local/bin/
sudo chmod +x /usr/local/bin/unified_shell

# Windows
copy unified_shell.exe C:\Windows\System32\
# Or add to PATH
```

## 🔧 Configuration

### First Run Setup

1. **Launch the shell**:
   ```bash
   ./unified_shell
   # or if installed: unified_shell
   ```

2. **Choose command mode**:
   - Select Windows, Linux, or Auto-detect mode
   - Your choice will be saved for future sessions

3. **Configure settings**:
   ```
   [AUTO]> config
   ```

### Configuration File

The shell creates a configuration file at:
- **Linux/macOS**: `~/.unified_shell_config`
- **Windows**: `%USERPROFILE%\.unified_shell_config`

Example configuration:
```ini
# Unified Shell Configuration
default_mode=auto_detect
auto_detect=true
save_preferences=true
```

## 🚀 Deployment Scenarios

### 1. Development Environment

```bash
# Setup development environment
./scripts/build.sh --setup-dev

# Build in debug mode
./scripts/build.sh --debug

# Run with debug output
./unified_shell --debug
```

### 2. Production Server

```bash
# Build optimized release
./scripts/build.sh --release --clean

# Install system-wide
sudo ./scripts/build.sh --install

# Create systemd service (Linux)
sudo cp scripts/unified-shell.service /etc/systemd/system/
sudo systemctl enable unified-shell
```

### 3. Container Deployment

#### Dockerfile Example

```dockerfile
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libreadline-dev \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . /app
WORKDIR /app

# Build
RUN make && make install

# Create non-root user
RUN useradd -m shelluser
USER shelluser

# Set entrypoint
ENTRYPOINT ["unified_shell"]
```

#### Build and Run Container

```bash
# Build container
docker build -t unified-shell .

# Run interactively
docker run -it unified-shell

# Run with volume mount
docker run -it -v $(pwd):/workspace unified-shell
```

### 4. CI/CD Integration

#### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    
    runs-on: ${{ matrix.os }}
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup dependencies (Linux)
      if: runner.os == 'Linux'
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential libreadline-dev
    
    - name: Setup dependencies (macOS)
      if: runner.os == 'macOS'
      run: brew install readline
    
    - name: Setup dependencies (Windows)
      if: runner.os == 'Windows'
      uses: msys2/setup-msys2@v2
      with:
        install: mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-readline
    
    - name: Build (Unix)
      if: runner.os != 'Windows'
      run: |
        chmod +x scripts/build.sh
        ./scripts/build.sh --cmake
    
    - name: Build (Windows)
      if: runner.os == 'Windows'
      shell: msys2 {0}
      run: ./scripts/build.sh --cmake
    
    - name: Test
      run: |
        ./unified_shell --version
        ./unified_shell --help
```

## 🔌 Integration with Existing Terminals

### 1. Terminal Emulator Integration

#### For Bash/Zsh
```bash
# Add to ~/.bashrc or ~/.zshrc
alias ushell='unified_shell'

# Or set as default shell (advanced)
# sudo usermod -s /usr/local/bin/unified_shell $USER
```

#### For PowerShell
```powershell
# Add to PowerShell profile
Set-Alias -Name ushell -Value unified_shell

# Function to launch unified shell
function Start-UnifiedShell {
    & unified_shell @args
}
```

### 2. IDE Integration

#### Visual Studio Code
Create a new terminal profile in `settings.json`:
```json
{
    "terminal.integrated.profiles.linux": {
        "Unified Shell": {
            "path": "/usr/local/bin/unified_shell"
        }
    },
    "terminal.integrated.profiles.windows": {
        "Unified Shell": {
            "path": "C:\\Program Files\\UnifiedShell\\unified_shell.exe"
        }
    }
}
```

#### JetBrains IDEs
1. Go to Settings → Tools → Terminal
2. Set Shell path to unified_shell executable

### 3. Real-time Terminal Sessions

#### Screen/Tmux Integration
```bash
# Create a persistent session
screen -S unified_session unified_shell
# or
tmux new-session -s unified_session 'unified_shell'

# Attach to session
screen -r unified_session
# or
tmux attach -t unified_session
```

#### SSH Server Setup
```bash
# Install and configure SSH server
sudo apt-get install openssh-server

# Configure sshd to allow unified_shell
echo "Match User shelluser" | sudo tee -a /etc/ssh/sshd_config
echo "    ForceCommand /usr/local/bin/unified_shell" | sudo tee -a /etc/ssh/sshd_config

# Restart SSH service
sudo systemctl restart ssh
```

## 🐛 Troubleshooting

### Common Issues

#### 1. Build Errors
```bash
# Check dependencies
./scripts/build.sh --check-deps

# Clean and rebuild
./scripts/build.sh --clean --verbose
```

#### 2. Readline Issues
```bash
# Linux: Install readline development headers
sudo apt-get install libreadline-dev

# macOS: Install via Homebrew
brew install readline

# Windows/MSYS2: Install readline
pacman -S mingw-w64-ucrt-x86_64-readline
```

#### 3. WSL Integration Problems
```powershell
# Check WSL installation
wsl --list --verbose

# Install WSL if not present
wsl --install

# Update WSL
wsl --update
```

### Debug Mode

```bash
# Run with debug output
unified_shell --debug

# Check system status
[AUTO]> status

# View configuration
[AUTO]> config
```

### Log Files

- **Build logs**: `build.log` (when using build scripts)
- **Runtime logs**: Check system logs or run with `--debug`
- **Configuration**: `~/.unified_shell_config`

## 📊 Performance Tuning

### 1. Memory Optimization
- Adjust history size: Edit MAX_HISTORY in source
- Disable features: Compile with specific flags

### 2. Startup Time
- Use static linking for faster startup
- Pre-load common executors

### 3. Command Execution
- Use local executors when possible
- Cache WSL availability checks

## 🔒 Security Considerations

### 1. Permissions
- Run with minimal required privileges
- Avoid running as root/administrator when possible

### 2. Command Validation
- The shell validates commands before execution
- Unknown commands are rejected with suggestions

### 3. WSL Security
- WSL commands run in isolated environment
- File system access is controlled by WSL settings

## 📈 Monitoring and Logging

### 1. Command History
- All commands are logged to history file
- History persists across sessions

### 2. System Integration
```bash
# Monitor with systemd (Linux)
journalctl -u unified-shell -f

# Monitor with Event Viewer (Windows)
# Check Application logs for unified_shell events
```

### 3. Custom Logging
Modify source code to add custom logging:
```cpp
// Add to unified_shell.cpp
#ifdef DEBUG
    std::cout << "Debug: Executing command: " << command << std::endl;
#endif
```

## 🎯 Use Cases

### 1. Cross-platform Development
- Switch between Windows and Linux commands seamlessly
- Test scripts on different environments
- Unified development workflow

### 2. System Administration
- Manage mixed Windows/Linux environments
- Remote system administration
- Automated deployment scripts

### 3. Education and Training
- Learn both Windows and Linux commands
- Practice system administration
- Demonstrate cross-platform concepts

### 4. DevOps and CI/CD
- Unified build scripts
- Cross-platform testing
- Container deployment

## 🔮 Advanced Features

### 1. Scripting Support
```bash
# Create shell scripts that work on any platform
#!/usr/bin/env unified_shell
mode linux
ls -la
mode windows
dir
```

### 2. Plugin System (Future)
- Custom command executors
- Third-party integrations
- Extended functionality

### 3. Remote Execution (Future)
- Execute commands on remote systems
- Distributed command execution
- Cloud integration

## 🆘 Support and Contributing

### Getting Help
- **Documentation**: Check README.md and this guide
- **Issues**: Report bugs on GitHub Issues
- **Discussions**: Join GitHub Discussions

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

### Development Setup
```bash
# Setup development environment
./scripts/build.sh --setup-dev

# Build in debug mode
./scripts/build.sh --debug

# Run tests
make test  # or ctest for CMake builds
```

---

## 📝 License

This project is licensed under the MIT License. See LICENSE file for details.

## 🙏 Acknowledgments

- GNU Readline library for command-line editing
- WSL team for Windows-Linux integration
- Open source community for inspiration and feedback