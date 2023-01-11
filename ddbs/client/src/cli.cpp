#include <iostream>
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#include <client/cli.hpp>

namespace client {

Shell::Shell(Fn _cmd) : line_read(nullptr), prompt(Prompt::SQL), shell_cmd(), conf_cmd(), sql_cmd(_cmd) {
    shell_cmd.insert(std::make_pair(
        "conf",
        [&](const std::string &msg) -> void {
            prompt = Prompt::CONF;
        }
    ));
    shell_cmd.insert(std::make_pair(
        "sql",
        [&](const std::string &msg) -> void {
            prompt = Prompt::SQL;
        }
    ));
    shell_cmd.insert(std::make_pair(
        "exit",
        [&](const std::string &msg) -> void {
            printf("Goodbye.\n");
            exit(0);
        }
    ));
    rl_bind_key('\t', rl_insert);
}

std::string Shell::parse_input() {
    if (line_read)
    {
      free (line_read);
      line_read = nullptr;
    }

    line_read = readline((prompt == Prompt::SQL ? "sql> " : "conf> "));

    /* If the line has any text in it, save it on the history. */
    if (line_read && *line_read)
        add_history(line_read);
    std::string line = "";
    if (line_read) {
        line = std::string(line_read);
        line.erase(0, line.find_first_not_of(" "));
    }
    return line;
}

void Shell::run() {
    static auto parse_cmd_name = [](const std::string &s) -> std::string {
        return s.substr(0, s.find_first_of(' '));
    };
    static auto parse_cmd_content = [](const std::string &s) -> std::string {
        return s.substr(s.find_first_of(' ') + 1);
    };

    while (true) {
        std::string line = parse_input();
        bool cmd_found = false;
        if (line.size() == 0)
            continue;
        if (shell_cmd.count(line)) {
            shell_cmd[line](""); cmd_found = true;
        } else {
            if (prompt == Prompt::SQL) {
                sql_cmd(line); cmd_found = true;
            } else {
                std::string cmd_name = parse_cmd_name(line);
                std::string cmd_content = parse_cmd_content(line);
                if (conf_cmd.count(cmd_name)) {
                    conf_cmd[cmd_name](cmd_content); cmd_found = true;
                }
            }
        }
        if (!cmd_found)
            printf("Command not found: %s\n", line.c_str());
    }
}

void Shell::register_conf_cmd(const std::string &cmd_name, const Fn &fn) {
    conf_cmd.insert(std::make_pair(
        cmd_name,
        fn
    ));
}

}