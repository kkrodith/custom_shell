# custom_shell

🚀 Features
✅ Command Execution
Execute external programs using fork(), execvp(), and waitpid() just like a standard shell.

📂 Built-in Commands
Supports internal commands like:

cd <dir> – change directory

export VAR=value – set environment variables

exit / $Q – exit the shell

help – list available commands

🧠 Smart Typo Suggestions
Suggests correct commands when user makes a typo (e.g., sl → ls, pod → pwd).

🔁 Command History (Persistent)
Remembers commands across sessions using GNU Readline. Navigate with ↑ / ↓ arrows.

⚙️ Background Job Control
Execute commands in the background using & (e.g., sleep 5 &). Prepares for jobs, fg, bg.

🧠 Tab Completion
Supports auto-completion via Tab using Readline's rl_complete.

📦 App Launching
Cross-platform support to launch applications (e.g., notepad.exe, calc).


///📂 Installation & Setup

(Warning- must have wsl install if run on windows).

1. Clone repo-
git clone https://github.com/kkrodith/custom_shell.git
cd custom_shell

2. Install Dependencies-
///  Ubuntu/Debian:
sudo apt update
sudo apt install g++ libreadline-dev

/// Arch Linux:
sudo pacman -S g++ readline base-devel

/// Windows (MSYS2):
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-readline

3. Build and Run
g++ shell.cpp -lreadline -o myshell
./myshell



// /📁 Project Structure
bash
Copy
Edit
custom-shell/
│
├── shell.cpp              # Main entry and shell logic
├── parser.hpp/.cpp        # Tokenizer and parser functions
├── executor.hpp/.cpp      # Command execution, I/O, jobs
├── history.hpp/.cpp       # Command history management
├── typo.hpp/.cpp          # Typo suggestion logic
├── README.md
