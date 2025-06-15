#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <csignal>
#include <cerrno>
#include <cstring>

// —————— Tokenize ——————
std::vector<std::string> tokenize(const std::string &line) {
    std::istringstream ss(line);
    std::string tok;
    std::vector<std::string> out;
    while (ss >> tok) out.push_back(tok);
    return out;
}

// —————— PIPE split ——————
std::vector<std::string> splitByPipe(const std::string &input) {
    std::vector<std::string> commands;
    std::istringstream ss(input);
    std::string segment;
    while (std::getline(ss, segment, '|')) commands.push_back(segment);
    return commands;
}

// —————— History ——————
std::vector<std::string> history;
const size_t MAX_HISTORY = 1000;
std::string getHistoryPath() {
    const char *home = getenv("HOME");
    if (!home) home = getpwuid(getuid())->pw_dir;
    return std::string(home) + "/.myshell_history";
}
void loadHistory() {
    std::ifstream in(getHistoryPath());
    std::string line;
    while (history.size() < MAX_HISTORY && std::getline(in, line))
        if (!line.empty()) history.push_back(line);
}
void appendHistory(const std::string &line) {
    std::ofstream out(getHistoryPath(), std::ios::app);
    out << line << "\n";
}

// —————— Redirection ——————
void handleRedirection(std::vector<std::string> &args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == ">" || args[i] == ">>" || args[i] == "<") {
            int flags = 0, fd;
            if (i + 1 >= args.size()) continue;
            if (args[i] == ">")
                flags = O_WRONLY | O_CREAT | O_TRUNC;
            else if (args[i] == ">>")
                flags = O_WRONLY | O_CREAT | O_APPEND;
            else
                flags = O_RDONLY;

            fd = open(args[i + 1].c_str(), flags, 0644);
            if (args[i] == "<") dup2(fd, STDIN_FILENO);
            else dup2(fd, STDOUT_FILENO);
            close(fd);
            args.erase(args.begin() + i, args.begin() + i + 2);
            i--;
        }
    }
}

// —————— Built-ins ——————
bool handleBuiltins(std::vector<std::string> &args) {
    if (args.empty()) return true;
    if (args[0] == "cd") {
        const char *dir = (args.size() > 1 ? args[1].c_str() : getenv("HOME"));
        if (chdir(dir) != 0) perror("cd failed");
        return true;
    }
    if (args[0] == "export") {
        if (args.size() == 2) putenv(const_cast<char *>(args[1].c_str()));
        else std::cerr << "Usage: export KEY=VALUE\n";
        return true;
    }
    if (args[0] == "history") {
        for (size_t i = 0; i < history.size(); ++i)
            std::cout << i + 1 << "  " << history[i] << "\n";
        return true;
    }
    if (args[0] == "help") {
        std::cout <<
            "myshell: built-in commands:\n"
            "basic shell commands like ls, cat, grep, sort etc.\n"
            "  cd [dir]       – change directory\n"
            "  export K=V     – set env var\n"
            "  history        – show command history\n"
            "  jobs, fg, bg   – job control\n"
            "  help           – this message\n"
            "  exit/$Q        – quit shell\n"
            "Also supports pipelines (|), redirection (<, >, >>), background (&)\n";
        return true;
    }
    if (args[0] == "exit" || args[0] == "$Q") exit(0);
    return false;
}

// —————— Jobs ——————
struct Job {
    int id;
    pid_t pid;
    std::string cmdline;
    bool running;
};

std::vector<Job> jobs;
static int nextJobId = 1;

void sigchldHandler(int) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        for (auto &job : jobs) {
            if (job.pid != pid) continue;
            if (WIFEXITED(status) || WIFSIGNALED(status)) job.running = false;
            else if (WIFSTOPPED(status)) job.running = false;
            else if (WIFCONTINUED(status)) job.running = true;
        }
    }
}
void initJobControl() {
    struct sigaction sa{};
    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, nullptr);
}
bool handleJobBuiltins(std::vector<std::string> &args) {
    if (args.empty()) return false;
    if (args[0] == "jobs") {
        for (auto &job : jobs) {
            std::cout << "[" << job.id << "] "
                      << (job.running ? "Running" : "Stopped/Done")
                      << "  " << job.cmdline << "\n";
        }
        return true;
    }
    if ((args[0] == "fg" || args[0] == "bg") && args.size() == 2) {
        int jid = std::stoi(args[1]);
        for (auto it = jobs.begin(); it != jobs.end(); ++it) {
            if (it->id == jid) {
                kill(it->pid, SIGCONT);
                if (args[0] == "fg") {
                    waitpid(it->pid, nullptr, 0);
                    jobs.erase(it);
                } else {
                    it->running = true;
                }
                return true;
            }
        }
        std::cerr << args[0] << ": job not found: " << jid << "\n";
        return true;
    }
    return false;
}

// —————— Levenshtein Distance ——————
int levenshtein(const std::string &a, const std::string &b) {
    int n = a.size(), m = b.size();
    std::vector<std::vector<int>> dp(n+1, std::vector<int>(m+1));
    for (int i=0;i<=n;i++) dp[i][0]=i;
    for (int j=0;j<=m;j++) dp[0][j]=j;
    for (int i=1;i<=n;i++)
        for (int j=1;j<=m;j++)
            dp[i][j]=std::min({ dp[i-1][j]+1, dp[i][j-1]+1,
                dp[i-1][j-1] + (a[i-1]!=b[j-1]) });
    return dp[n][m];
}
void suggest(const std::string &cmd) {
    const std::vector<std::string> builtins = {
        "cd", "export", "exit", "help", "history", "jobs", "fg", "bg"
    };
    std::string best;
    int bestDist = 1e9;
    for (const auto &b : builtins) {
        int dist = levenshtein(cmd, b);
        if (dist < bestDist) {
            best = b;
            bestDist = dist;
        }
    }
    if (bestDist <= 2)
        std::cout << "Did you mean `" << best << "`?\n";
}

// —————— Pipeline Execution ——————
void executePipeline(const std::vector<std::string> &commands) {
    int n = commands.size();
    int pipefds[2 * (n - 1)];
    for (int i = 0; i < n - 1; ++i) pipe(pipefds + i * 2);

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            if (i > 0) dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            if (i < n - 1) dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            for (int j = 0; j < 2 * (n - 1); ++j) close(pipefds[j]);

            std::vector<std::string> args = tokenize(commands[i]);
            handleRedirection(args);
            char *argv[args.size() + 1];
            for (size_t k = 0; k < args.size(); ++k)
                argv[k] = const_cast<char *>(args[k].c_str());
            argv[args.size()] = nullptr;
            execvp(argv[0], argv);
            perror("exec failed");
            exit(1);
        }
    }
    for (int j = 0; j < 2 * (n - 1); ++j) close(pipefds[j]);
    for (int i = 0; i < n; ++i) wait(nullptr);
}

// —————— Command Execution ——————
void runCommand(std::string input) {
    bool background = false;
    if (input.back() == '&') {
        background = true;
        input.pop_back();
    }

    auto parts = splitByPipe(input);
    if (parts.size() > 1) {
        executePipeline(parts);
        return;
    }

    std::vector<std::string> args = tokenize(input);
    if (handleBuiltins(args)) return;
    if (handleJobBuiltins(args)) return;

    pid_t pid = fork();
    if (pid == 0) {
        handleRedirection(args);
        char *argv[args.size() + 1];
        for (size_t i = 0; i < args.size(); ++i)
            argv[i] = const_cast<char *>(args[i].c_str());
        argv[args.size()] = nullptr;
        execvp(argv[0], argv);
        std::cerr << "Error: `" << args[0] << "`: " << strerror(errno) << "\n";
        suggest(args[0]);
        exit(1);
    } else if (!background) {
        waitpid(pid, nullptr, 0);
    } else {
        jobs.push_back({ nextJobId++, pid, input, true });
        std::cout << "[" << jobs.back().id << "] " << pid << "\n";
    }
}

// —————— MAIN ——————
int main() {
    initJobControl();
    using_history();
    loadHistory();
    rl_bind_key('\t', rl_complete);

    std::cout << "Welcome to Custom Shell! Type 'exit' or '$Q' to quit.\n";
    while (true) {
        char *line = readline("myshell> ");
        if (!line) break;
        std::string input(line);
        free(line);
        if (input.empty()) continue;
        add_history(input.c_str());
        history.push_back(input);
        appendHistory(input);
        runCommand(input);
    }
    std::cout << "\nGoodbye!\n";
    return 0;
}
