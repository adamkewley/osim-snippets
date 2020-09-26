#include <iostream>
#include <cstring>

static const char* usage = R"(usage: osim-snippets <command>

commands:
    show         show an osim file in a GUI
    sizes        print memory usage of various OpenSim objects
    expt_cable   cable wrapping experiment
    expt_pendu   pendulum experiment
    expt_wrapp   wrapping experiment
)";

int oss_show(int argc, char** argv);
int oss_sizes(int argc, char** argv);
int oss_expt_cable(int argc, char** argv);
int oss_expt_pendu(int argc, char** argv);
int oss_expt_wrapp(int argc, char** argv);
int oss_expt_party(int argc, char** argv);

struct Cmd final {
    const char* name;
    int (*main)(int, char**);
};

static const Cmd cmds[] = {
    { "expt_wrap", oss_expt_wrapp },
    { "show", oss_show },
};

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr << usage << std::endl;
        return -1;
    }

    for (Cmd const& cmd : cmds) {
        if (std::strcmp(cmd.name, argv[1]) == 0) {
            return cmd.main(argc, argv);
        }
    }

    std::cerr << argv[0] << ": invalid arguments passed" << std::endl;
    std::cerr << usage << std::endl;
    return -1;
}
