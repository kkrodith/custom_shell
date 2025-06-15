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
