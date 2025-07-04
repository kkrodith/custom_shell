#include "unified_shell.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include <iomanip>
#include <readline/readline.h>
#include <readline/history.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #define popen _popen
    #define pclose _pclose
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

// ==================== OS Detection ====================

OSType OSDetector::detectOS() {
#ifdef _WIN32
    if (isWSL()) {
        return OSType::WSL;
    }
    return OSType::WINDOWS;
#elif __linux__
    if (isWSL()) {
        return OSType::WSL;
    }
    return OSType::LINUX;
#else
    return OSType::UNKNOWN;
#endif
}

bool OSDetector::isWSL() {
    // Check if we're running under WSL
    std::ifstream version("/proc/version");
    if (version.is_open()) {
        std::string line;
        std::getline(version, line);
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        return line.find("microsoft") != std::string::npos;
    }
    return false;
}

std::string OSDetector::getOSName() {
    switch (detectOS()) {
        case OSType::WINDOWS: return "Windows";
        case OSType::LINUX: return "Linux";
        case OSType::WSL: return "WSL (Windows Subsystem for Linux)";
        default: return "Unknown";
    }
}

bool OSDetector::hasWSL() {
#ifdef _WIN32
    FILE* pipe = popen("wsl --list --quiet 2>nul", "r");
    if (pipe) {
        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return !result.empty();
    }
#else
    // On Linux/WSL, check if wsl.exe is available
    FILE* pipe = popen("which wsl.exe 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return !result.empty();
    }
#endif
    return false;
}

bool OSDetector::hasCmd() {
#ifdef _WIN32
    return true;
#else
    FILE* pipe = popen("which cmd.exe 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return !result.empty();
    }
    return false;
#endif
}

bool OSDetector::hasPowerShell() {
#ifdef _WIN32
    FILE* pipe = popen("where powershell 2>nul", "r");
#else
    FILE* pipe = popen("which powershell.exe 2>/dev/null", "r");
#endif
    if (pipe) {
        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return !result.empty();
    }
    return false;
}

// ==================== Shell Configuration ====================

ShellConfig::ShellConfig() 
    : defaultMode(CommandMode::AUTO_DETECT)
    , autoDetectCommands(true)
    , savePreferences(true) {
    
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
    if (!home) home = getenv("HOME");
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
#endif
    
    if (home) {
        configPath = std::string(home) + "/.unified_shell_config";
    } else {
        configPath = ".unified_shell_config";
    }
}

void ShellConfig::loadConfig() {
    std::ifstream config(configPath);
    if (!config.is_open()) return;
    
    std::string line;
    while (std::getline(config, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "default_mode") {
            if (value == "windows") defaultMode = CommandMode::WINDOWS;
            else if (value == "linux") defaultMode = CommandMode::LINUX;
            else defaultMode = CommandMode::AUTO_DETECT;
        } else if (key == "auto_detect") {
            autoDetectCommands = (value == "true");
        } else if (key == "save_preferences") {
            savePreferences = (value == "true");
        }
    }
}

void ShellConfig::saveConfig() {
    if (!savePreferences) return;
    
    std::ofstream config(configPath);
    if (!config.is_open()) return;
    
    config << "# Unified Shell Configuration\n";
    config << "default_mode=";
    switch (defaultMode) {
        case CommandMode::WINDOWS: config << "windows"; break;
        case CommandMode::LINUX: config << "linux"; break;
        default: config << "auto_detect"; break;
    }
    config << "\n";
    config << "auto_detect=" << (autoDetectCommands ? "true" : "false") << "\n";
    config << "save_preferences=" << (savePreferences ? "true" : "false") << "\n";
}

// ==================== Windows Executor ====================

WindowsExecutor::WindowsExecutor(bool usePowerShell) : usesPowerShell(usePowerShell) {}

int WindowsExecutor::execute(const std::vector<std::string>& args, bool background) {
    if (args.empty()) return -1;
    
    std::string command = buildCommand(args);
    
#ifdef _WIN32
    if (background) {
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        
        if (CreateProcess(NULL, const_cast<char*>(command.c_str()), NULL, NULL, FALSE, 
                         CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cout << "Background process started with PID: " << pi.dwProcessId << std::endl;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return 0;
        }
        return -1;
    } else {
        return system(command.c_str());
    }
#else
    // On Linux/WSL, execute Windows commands through cmd.exe or powershell.exe
    std::string execCommand;
    if (usesPowerShell) {
        execCommand = "powershell.exe -c \"" + command + "\"";
    } else {
        execCommand = "cmd.exe /c \"" + command + "\"";
    }
    
    if (background) {
        execCommand += " &";
    }
    
    return system(execCommand.c_str());
#endif
}

bool WindowsExecutor::isAvailable() {
#ifdef _WIN32
    return true;
#else
    return OSDetector::hasCmd() || OSDetector::hasPowerShell();
#endif
}

std::string WindowsExecutor::getName() {
    return usesPowerShell ? "PowerShell" : "Windows CMD";
}

bool WindowsExecutor::canHandleCommand(const std::string& command) {
    return isWindowsCommand(command);
}

std::string WindowsExecutor::buildCommand(const std::vector<std::string>& args) {
    std::ostringstream oss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) oss << " ";
        // Quote arguments that contain spaces
        if (args[i].find(' ') != std::string::npos) {
            oss << "\"" << args[i] << "\"";
        } else {
            oss << args[i];
        }
    }
    return oss.str();
}

bool WindowsExecutor::isWindowsCommand(const std::string& command) {
    static const std::unordered_set<std::string> windowsCommands = {
        "dir", "copy", "move", "del", "md", "rd", "cd", "type", "cls", "echo",
        "set", "path", "prompt", "title", "color", "date", "time", "ver",
        "vol", "tree", "attrib", "comp", "fc", "find", "findstr", "sort",
        "more", "xcopy", "robocopy", "tasklist", "taskkill", "net", "ping",
        "ipconfig", "netstat", "systeminfo", "driverquery", "reg", "sc",
        "powershell", "cmd", "notepad", "calc", "mspaint", "explorer"
    };
    
    return windowsCommands.find(command) != windowsCommands.end();
}

// ==================== Linux Executor ====================

int LinuxExecutor::execute(const std::vector<std::string>& args, bool background) {
    if (args.empty()) return -1;
    
#ifdef _WIN32
    // On Windows, we need to use WSL to execute Linux commands
    std::string command = "wsl ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) command += " ";
        if (args[i].find(' ') != std::string::npos) {
            command += "'" + args[i] + "'";
        } else {
            command += args[i];
        }
    }
    
    if (background) {
        command += " &";
    }
    
    return system(command.c_str());
#else
    // Native Linux execution
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::vector<char*> argv;
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        
        execvp(argv[0], argv.data());
        perror("execvp failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        } else {
            std::cout << "Background process started with PID: " << pid << std::endl;
            return 0;
        }
    } else {
        perror("fork failed");
        return -1;
    }
#endif
}

bool LinuxExecutor::isAvailable() {
#ifdef _WIN32
    return OSDetector::hasWSL();
#else
    return true;
#endif
}

std::string LinuxExecutor::getName() {
#ifdef _WIN32
    return "WSL (Linux commands)";
#else
    return "Linux Native";
#endif
}

bool LinuxExecutor::canHandleCommand(const std::string& command) {
    return isLinuxCommand(command);
}

bool LinuxExecutor::isLinuxCommand(const std::string& command) {
    static const std::unordered_set<std::string> linuxCommands = {
        "ls", "cat", "grep", "awk", "sed", "sort", "uniq", "head", "tail",
        "wc", "cut", "tr", "find", "locate", "which", "whereis", "file",
        "chmod", "chown", "chgrp", "umask", "ln", "cp", "mv", "rm", "mkdir",
        "rmdir", "pwd", "cd", "pushd", "popd", "du", "df", "mount", "umount",
        "ps", "top", "htop", "kill", "killall", "jobs", "fg", "bg", "nohup",
        "screen", "tmux", "man", "info", "less", "more", "nano", "vim", "emacs",
        "tar", "gzip", "gunzip", "zip", "unzip", "curl", "wget", "ssh", "scp",
        "rsync", "ping", "netstat", "ifconfig", "iptables", "systemctl", "service"
    };
    
    return linuxCommands.find(command) != linuxCommands.end();
}

// ==================== WSL Executor ====================

WSLExecutor::WSLExecutor(const std::string& distro) : distribution(distro) {
    if (distribution.empty()) {
        distribution = getDefaultDistribution();
    }
}

int WSLExecutor::execute(const std::vector<std::string>& args, bool background) {
    if (args.empty()) return -1;
    
    std::string command = "wsl";
    if (!distribution.empty()) {
        command += " -d " + distribution;
    }
    
    for (const auto& arg : args) {
        command += " ";
        if (arg.find(' ') != std::string::npos) {
            command += "'" + arg + "'";
        } else {
            command += arg;
        }
    }
    
    if (background) {
        command += " &";
    }
    
    return system(command.c_str());
}

bool WSLExecutor::isAvailable() {
    return OSDetector::hasWSL();
}

std::string WSLExecutor::getName() {
    return "WSL" + (distribution.empty() ? "" : " (" + distribution + ")");
}

bool WSLExecutor::canHandleCommand(const std::string& command) {
    // WSL can handle any Linux command
    return LinuxExecutor().canHandleCommand(command);
}

std::vector<std::string> WSLExecutor::getAvailableDistributions() {
    std::vector<std::string> distros;
    FILE* pipe = popen("wsl --list --quiet", "r");
    if (pipe) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            // Remove trailing newline and carriage return
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            if (!line.empty()) {
                distros.push_back(line);
            }
        }
        pclose(pipe);
    }
    return distros;
}

std::string WSLExecutor::getDefaultDistribution() {
    FILE* pipe = popen("wsl --list --quiet", "r");
    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            pclose(pipe);
            return line;
        }
        pclose(pipe);
    }
    return "";
}

// ==================== Command Detector ====================

CommandMode CommandDetector::detectCommandType(const std::string& command) {
    if (isBuiltinCommand(command)) {
        return CommandMode::AUTO_DETECT;
    }
    
    if (isWindowsCommand(command)) {
        return CommandMode::WINDOWS;
    }
    
    if (isLinuxCommand(command)) {
        return CommandMode::LINUX;
    }
    
    return CommandMode::AUTO_DETECT;
}

bool CommandDetector::isWindowsCommand(const std::string& command) {
    WindowsExecutor executor;
    return executor.canHandleCommand(command);
}

bool CommandDetector::isLinuxCommand(const std::string& command) {
    LinuxExecutor executor;
    return executor.canHandleCommand(command);
}

bool CommandDetector::isBuiltinCommand(const std::string& command) {
    static const std::unordered_set<std::string> builtins = {
        "help", "exit", "quit", "mode", "config", "status", "history", "clear", "cls"
    };
    return builtins.find(command) != builtins.end();
}

// ==================== Unified Shell ====================

UnifiedShell::UnifiedShell() : currentOS(OSDetector::detectOS()), currentMode(CommandMode::AUTO_DETECT) {
    config.loadConfig();
    currentMode = config.getDefaultMode();
}

void UnifiedShell::initialize() {
    // Initialize executors
    executors[CommandMode::WINDOWS] = std::make_unique<WindowsExecutor>();
    executors[CommandMode::LINUX] = std::make_unique<LinuxExecutor>();
    
    // Initialize readline
    using_history();
    loadHistory();
    rl_bind_key('\t', rl_complete);
    
    displayWelcome();
    
    // Prompt for mode if in auto-detect and first run
    if (currentMode == CommandMode::AUTO_DETECT) {
        currentMode = promptForMode();
        config.setDefaultMode(currentMode);
        config.saveConfig();
    }
}

void UnifiedShell::run() {
    while (true) {
        char* line = readline(getPrompt().c_str());
        if (!line) break;  // EOF (Ctrl+D)
        
        std::string input(line);
        free(line);
        
        if (input.empty()) continue;
        
        add_history(input.c_str());
        addToHistory(input);
        
        if (executeCommand(input) == -2) {  // Exit command
            break;
        }
    }
    
    saveHistory();
    config.saveConfig();
    std::cout << "\nGoodbye!\n";
}

CommandMode UnifiedShell::promptForMode() {
    std::cout << "\n🚀 Welcome to Unified Shell!\n";
    std::cout << "Running on: " << OSDetector::getOSName() << "\n\n";
    
    std::cout << "Choose your default command mode:\n";
    std::cout << "1. Windows commands (cmd/PowerShell style)\n";
    std::cout << "2. Linux commands (bash/sh style)\n";
    std::cout << "3. Auto-detect (recommended)\n";
    std::cout << "\nEnter your choice (1-3): ";
    
    std::string choice;
    std::getline(std::cin, choice);
    
    switch (choice[0]) {
        case '1': return CommandMode::WINDOWS;
        case '2': return CommandMode::LINUX;
        default: return CommandMode::AUTO_DETECT;
    }
}

void UnifiedShell::setMode(CommandMode mode) {
    currentMode = mode;
    std::cout << "Command mode set to: ";
    switch (mode) {
        case CommandMode::WINDOWS: std::cout << "Windows"; break;
        case CommandMode::LINUX: std::cout << "Linux"; break;
        case CommandMode::AUTO_DETECT: std::cout << "Auto-detect"; break;
    }
    std::cout << std::endl;
}

int UnifiedShell::executeCommand(const std::string& input) {
    auto args = tokenize(input);
    if (args.empty()) return 0;
    
    // Handle built-in commands first
    if (handleBuiltins(args)) {
        return 0;
    }
    
    // Check for special exit condition
    if (args[0] == "exit" || args[0] == "quit") {
        return -2;
    }
    
    // Select appropriate executor
    CommandExecutor* executor = selectExecutor(args[0]);
    if (!executor) {
        std::cerr << "Error: No suitable executor found for command '" << args[0] << "'\n";
        suggestCommand(args[0]);
        return -1;
    }
    
    // Check for background execution
    bool background = false;
    if (!args.empty() && args.back() == "&") {
        background = true;
        args.pop_back();
    }
    
    return executor->execute(args, background);
}

CommandExecutor* UnifiedShell::selectExecutor(const std::string& command) {
    if (currentMode != CommandMode::AUTO_DETECT) {
        auto it = executors.find(currentMode);
        if (it != executors.end() && it->second->isAvailable()) {
            return it->second.get();
        }
    }
    
    // Auto-detect or fallback
    CommandMode detectedMode = detectCommandType(command);
    if (detectedMode != CommandMode::AUTO_DETECT) {
        auto it = executors.find(detectedMode);
        if (it != executors.end() && it->second->isAvailable()) {
            return it->second.get();
        }
    }
    
    // Try all available executors
    for (auto& [mode, executor] : executors) {
        if (executor->isAvailable() && executor->canHandleCommand(command)) {
            return executor.get();
        }
    }
    
    return nullptr;
}

CommandMode UnifiedShell::detectCommandType(const std::string& command) {
    return CommandDetector::detectCommandType(command);
}

void UnifiedShell::suggestCommand(const std::string& command) {
    std::cout << "Command '" << command << "' not found.\n";
    std::cout << "Try:\n";
    std::cout << "  - 'help' for available commands\n";
    std::cout << "  - 'mode' to change command mode\n";
    std::cout << "  - Check spelling and try again\n";
}

std::vector<std::string> UnifiedShell::tokenize(const std::string& input) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string UnifiedShell::getPrompt() {
    std::string prompt = "[";
    switch (currentMode) {
        case CommandMode::WINDOWS: prompt += "WIN"; break;
        case CommandMode::LINUX: prompt += "LNX"; break;
        case CommandMode::AUTO_DETECT: prompt += "AUTO"; break;
    }
    prompt += "]> ";
    return prompt;
}

void UnifiedShell::displayWelcome() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    🚀 UNIFIED SHELL 🚀                      ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║        Cross-Platform Command Execution Environment         ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Features:                                                   ║\n";
    std::cout << "║    • Windows & Linux command support                        ║\n";
    std::cout << "║    • Automatic OS detection                                  ║\n";
    std::cout << "║    • WSL integration                                         ║\n";
    std::cout << "║    • Command history & tab completion                       ║\n";
    std::cout << "║    • Background job execution                               ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Type 'help' for commands, 'exit' to quit                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "System: " << OSDetector::getOSName() << "\n";
}

bool UnifiedShell::handleBuiltins(const std::vector<std::string>& args) {
    if (args.empty()) return false;
    
    const std::string& cmd = args[0];
    
    if (cmd == "help") {
        showHelp();
        return true;
    }
    
    if (cmd == "mode") {
        if (args.size() > 1) {
            const std::string& mode = args[1];
            if (mode == "windows" || mode == "win") {
                setMode(CommandMode::WINDOWS);
            } else if (mode == "linux" || mode == "lnx") {
                setMode(CommandMode::LINUX);
            } else if (mode == "auto") {
                setMode(CommandMode::AUTO_DETECT);
            } else {
                std::cout << "Invalid mode. Use: windows, linux, or auto\n";
            }
        } else {
            currentMode = promptForMode();
        }
        return true;
    }
    
    if (cmd == "status") {
        showStatus();
        return true;
    }
    
    if (cmd == "config") {
        configureShell();
        return true;
    }
    
    if (cmd == "history") {
        showHistory();
        return true;
    }
    
    if (cmd == "clear" || cmd == "cls") {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        return true;
    }
    
    return false;
}

void UnifiedShell::showHelp() {
    std::cout << "\n🚀 Unified Shell - Available Commands:\n\n";
    
    std::cout << "Built-in Commands:\n";
    std::cout << "  help                 - Show this help message\n";
    std::cout << "  mode [windows|linux|auto] - Change or set command mode\n";
    std::cout << "  status               - Show system and shell status\n";
    std::cout << "  config               - Configure shell settings\n";
    std::cout << "  history              - Show command history\n";
    std::cout << "  clear/cls            - Clear the screen\n";
    std::cout << "  exit/quit            - Exit the shell\n\n";
    
    std::cout << "Command Execution:\n";
    std::cout << "  • Windows commands: dir, copy, move, del, etc.\n";
    std::cout << "  • Linux commands: ls, cp, mv, rm, etc.\n";
    std::cout << "  • Add '&' at the end for background execution\n";
    std::cout << "  • Use Tab for auto-completion\n";
    std::cout << "  • Use ↑/↓ arrows for command history\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  ls -la               - List files (Linux style)\n";
    std::cout << "  dir /w               - List files (Windows style)\n";
    std::cout << "  ping google.com &    - Background ping\n";
    std::cout << "  mode windows         - Switch to Windows mode\n";
}

void UnifiedShell::showStatus() {
    std::cout << "\n📊 System Status:\n";
    std::cout << "  OS: " << OSDetector::getOSName() << "\n";
    std::cout << "  Current Mode: ";
    switch (currentMode) {
        case CommandMode::WINDOWS: std::cout << "Windows"; break;
        case CommandMode::LINUX: std::cout << "Linux"; break;
        case CommandMode::AUTO_DETECT: std::cout << "Auto-detect"; break;
    }
    std::cout << "\n";
    
    std::cout << "\n🔧 Available Executors:\n";
    for (auto& [mode, executor] : executors) {
        std::cout << "  " << executor->getName() << ": " 
                  << (executor->isAvailable() ? "✅ Available" : "❌ Not Available") << "\n";
    }
    
    std::cout << "\n💾 Configuration:\n";
    std::cout << "  Auto-detect commands: " << (config.getAutoDetectCommands() ? "Yes" : "No") << "\n";
    std::cout << "  Save preferences: " << (config.getSavePreferences() ? "Yes" : "No") << "\n";
    std::cout << "  History size: " << history.size() << "/" << MAX_HISTORY << "\n";
}

void UnifiedShell::configureShell() {
    std::cout << "\n⚙️  Shell Configuration:\n";
    std::cout << "1. Set default command mode\n";
    std::cout << "2. Toggle auto-detect commands\n";
    std::cout << "3. Toggle save preferences\n";
    std::cout << "4. Clear history\n";
    std::cout << "5. Reset to defaults\n";
    std::cout << "\nEnter choice (1-5): ";
    
    std::string choice;
    std::getline(std::cin, choice);
    
    switch (choice[0]) {
        case '1':
            config.setDefaultMode(promptForMode());
            break;
        case '2':
            config.setAutoDetectCommands(!config.getAutoDetectCommands());
            std::cout << "Auto-detect commands: " << (config.getAutoDetectCommands() ? "ON" : "OFF") << "\n";
            break;
        case '3':
            config.setSavePreferences(!config.getSavePreferences());
            std::cout << "Save preferences: " << (config.getSavePreferences() ? "ON" : "OFF") << "\n";
            break;
        case '4':
            history.clear();
            clear_history();
            std::cout << "Command history cleared.\n";
            break;
        case '5':
            config = ShellConfig();
            std::cout << "Configuration reset to defaults.\n";
            break;
        default:
            std::cout << "Invalid choice.\n";
    }
    
    config.saveConfig();
}

void UnifiedShell::loadHistory() {
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
    if (!home) home = getenv("HOME");
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
#endif
    
    if (home) {
        std::string historyPath = std::string(home) + "/.unified_shell_history";
        std::ifstream histFile(historyPath);
        std::string line;
        
        while (history.size() < MAX_HISTORY && std::getline(histFile, line)) {
            if (!line.empty()) {
                history.push_back(line);
            }
        }
    }
}

void UnifiedShell::saveHistory() {
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
    if (!home) home = getenv("HOME");
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
#endif
    
    if (home) {
        std::string historyPath = std::string(home) + "/.unified_shell_history";
        std::ofstream histFile(historyPath);
        
        for (const auto& cmd : history) {
            histFile << cmd << "\n";
        }
    }
}

void UnifiedShell::addToHistory(const std::string& command) {
    history.push_back(command);
    if (history.size() > MAX_HISTORY) {
        history.erase(history.begin());
    }
}

void UnifiedShell::showHistory() {
    std::cout << "\n📜 Command History:\n";
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << std::setw(4) << (i + 1) << "  " << history[i] << "\n";
    }
    std::cout << "\n";
}