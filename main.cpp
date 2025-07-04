#include "unified_shell.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>

// Global shell instance for signal handling
UnifiedShell* g_shell = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully...\n";
    if (g_shell) {
        // Perform cleanup
        std::cout << "Goodbye!\n";
    }
    exit(0);
}

// Setup signal handlers
void setupSignalHandlers() {
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // Termination request
#ifndef _WIN32
    signal(SIGQUIT, signalHandler);  // Quit signal (Unix only)
    signal(SIGHUP, signalHandler);   // Hangup signal (Unix only)
#endif
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        bool debugMode = false;
        bool helpRequested = false;
        std::string configFile;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            
            if (arg == "--help" || arg == "-h") {
                helpRequested = true;
            } else if (arg == "--debug" || arg == "-d") {
                debugMode = true;
            } else if (arg == "--config" || arg == "-c") {
                if (i + 1 < argc) {
                    configFile = argv[++i];
                } else {
                    std::cerr << "Error: --config requires a file path\n";
                    return 1;
                }
            } else if (arg == "--version" || arg == "-v") {
                std::cout << "Unified Shell v1.0.0\n";
                std::cout << "Cross-platform command execution environment\n";
                std::cout << "Supports Windows, Linux, and WSL commands\n";
                return 0;
            } else {
                std::cerr << "Unknown option: " << arg << "\n";
                std::cerr << "Use --help for usage information\n";
                return 1;
            }
        }
        
        if (helpRequested) {
            std::cout << "Unified Shell - Cross-Platform Command Execution Environment\n\n";
            std::cout << "Usage: " << argv[0] << " [OPTIONS]\n\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help       Show this help message\n";
            std::cout << "  -v, --version    Show version information\n";
            std::cout << "  -d, --debug      Enable debug mode\n";
            std::cout << "  -c, --config     Specify custom config file\n\n";
            std::cout << "Features:\n";
            std::cout << "  • Execute Windows and Linux commands on any platform\n";
            std::cout << "  • Automatic OS detection and command routing\n";
            std::cout << "  • WSL integration for Linux commands on Windows\n";
            std::cout << "  • Command history with persistent storage\n";
            std::cout << "  • Tab completion and arrow key navigation\n";
            std::cout << "  • Background job execution\n";
            std::cout << "  • Configurable command modes\n\n";
            std::cout << "Examples:\n";
            std::cout << "  " << argv[0] << "               # Start interactive shell\n";
            std::cout << "  " << argv[0] << " --debug      # Start with debug output\n";
            std::cout << "  " << argv[0] << " --version    # Show version info\n\n";
            return 0;
        }
        
        // Setup signal handlers for graceful shutdown
        setupSignalHandlers();
        
        // Create and initialize the unified shell
        UnifiedShell shell;
        g_shell = &shell;
        
        if (debugMode) {
            std::cout << "Debug mode enabled\n";
            std::cout << "OS: " << OSDetector::getOSName() << "\n";
            std::cout << "WSL available: " << (OSDetector::hasWSL() ? "Yes" : "No") << "\n";
            std::cout << "CMD available: " << (OSDetector::hasCmd() ? "Yes" : "No") << "\n";
            std::cout << "PowerShell available: " << (OSDetector::hasPowerShell() ? "Yes" : "No") << "\n";
            std::cout << "------------------------\n";
        }
        
        // Initialize the shell
        shell.initialize();
        
        // Run the main shell loop
        shell.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}