#ifndef UNIFIED_SHELL_HPP
#define UNIFIED_SHELL_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// OS Detection
enum class OSType {
    WINDOWS,
    LINUX,
    WSL,
    UNKNOWN
};

// Command Mode
enum class CommandMode {
    WINDOWS,
    LINUX,
    AUTO_DETECT
};

// Forward declarations
class CommandExecutor;
class WindowsExecutor;
class LinuxExecutor;
class WSLExecutor;

// OS Detection and Management
class OSDetector {
public:
    static OSType detectOS();
    static bool isWSL();
    static std::string getOSName();
    static bool hasWSL();
    static bool hasCmd();
    static bool hasPowerShell();
};

// Configuration Management
class ShellConfig {
private:
    CommandMode defaultMode;
    bool autoDetectCommands;
    bool savePreferences;
    std::string configPath;
    
public:
    ShellConfig();
    void loadConfig();
    void saveConfig();
    
    CommandMode getDefaultMode() const { return defaultMode; }
    void setDefaultMode(CommandMode mode) { defaultMode = mode; }
    
    bool getAutoDetectCommands() const { return autoDetectCommands; }
    void setAutoDetectCommands(bool detect) { autoDetectCommands = detect; }
    
    bool getSavePreferences() const { return savePreferences; }
    void setSavePreferences(bool save) { savePreferences = save; }
};

// Command Executor Interface
class CommandExecutor {
public:
    virtual ~CommandExecutor() = default;
    virtual int execute(const std::vector<std::string>& args, bool background = false) = 0;
    virtual bool isAvailable() = 0;
    virtual std::string getName() = 0;
    virtual bool canHandleCommand(const std::string& command) = 0;
};

// Windows Command Executor
class WindowsExecutor : public CommandExecutor {
private:
    bool usesPowerShell;
    
public:
    WindowsExecutor(bool usePowerShell = false);
    int execute(const std::vector<std::string>& args, bool background = false) override;
    bool isAvailable() override;
    std::string getName() override;
    bool canHandleCommand(const std::string& command) override;
    
private:
    std::string buildCommand(const std::vector<std::string>& args);
    bool isWindowsCommand(const std::string& command);
};

// Linux Command Executor
class LinuxExecutor : public CommandExecutor {
public:
    int execute(const std::vector<std::string>& args, bool background = false) override;
    bool isAvailable() override;
    std::string getName() override;
    bool canHandleCommand(const std::string& command) override;
    
private:
    bool isLinuxCommand(const std::string& command);
};

// WSL Command Executor
class WSLExecutor : public CommandExecutor {
private:
    std::string distribution;
    
public:
    WSLExecutor(const std::string& distro = "");
    int execute(const std::vector<std::string>& args, bool background = false) override;
    bool isAvailable() override;
    std::string getName() override;
    bool canHandleCommand(const std::string& command) override;
    
private:
    std::vector<std::string> getAvailableDistributions();
    std::string getDefaultDistribution();
};

// Main Unified Shell Class
class UnifiedShell {
private:
    OSType currentOS;
    CommandMode currentMode;
    ShellConfig config;
    std::unordered_map<CommandMode, std::unique_ptr<CommandExecutor>> executors;
    
    // Original shell functionality
    std::vector<std::string> history;
    static const size_t MAX_HISTORY = 1000;
    
public:
    UnifiedShell();
    ~UnifiedShell() = default;
    
    void initialize();
    void run();
    
    // Mode management
    CommandMode promptForMode();
    void setMode(CommandMode mode);
    CommandMode getCurrentMode() const { return currentMode; }
    
    // Command execution
    int executeCommand(const std::string& input);
    CommandExecutor* selectExecutor(const std::string& command);
    
    // Command detection and suggestion
    CommandMode detectCommandType(const std::string& command);
    void suggestCommand(const std::string& command);
    
    // History management
    void loadHistory();
    void saveHistory();
    void addToHistory(const std::string& command);
    void showHistory();
    
    // Built-in commands
    bool handleBuiltins(const std::vector<std::string>& args);
    void showHelp();
    void showStatus();
    void configureShell();
    
    // Utility functions
    std::vector<std::string> tokenize(const std::string& input);
    std::string getPrompt();
    void displayWelcome();
};

// Command type detection utilities
class CommandDetector {
public:
    static CommandMode detectCommandType(const std::string& command);
    static bool isWindowsCommand(const std::string& command);
    static bool isLinuxCommand(const std::string& command);
    static bool isBuiltinCommand(const std::string& command);
    
private:
    static std::vector<std::string> getWindowsCommands();
    static std::vector<std::string> getLinuxCommands();
    static std::vector<std::string> getBuiltinCommands();
};

#endif // UNIFIED_SHELL_HPP