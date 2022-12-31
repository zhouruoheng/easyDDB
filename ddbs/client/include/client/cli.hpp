#pragma once

#include <map>
#include <string>
#include <functional>

namespace client {

using Fn = std::function<void(const std::string&)>;

enum class Prompt {
    SQL, CONF
};

class Shell {
public:
    char *line_read;
    Prompt prompt;

    std::map<std::string, Fn> shell_cmd;
    std::map<std::string, Fn> conf_cmd;
    Fn sql_cmd;

    Shell(Fn _cmd);
    ~Shell() = default;

    std::string parse_input();
    void run();
    void register_conf_cmd(const std::string &cmd_name, const Fn &fn);

};

}
