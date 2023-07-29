#include "cli.hpp"
#include "path.hpp"
#include "config.hpp"

std::string getSpaces(int spaces, const std::string& str)
{
    int actual_spaces = spaces - str.size();
    if(actual_spaces < 5) {
        actual_spaces = 5;
    }
    return std::string(actual_spaces, ' ');
}

void printHelp(const CLI& cli)
{
    std::string program = cli.getProgramName();
    std::string subcmd = cli.getActiveSubcommand();
    if(subcmd.empty()) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << program << " <options> <values>" << std::endl;
        std::cout << "  " << program << " <subcommand> <options>" << std::endl;
        std::cout << std::endl;
        std::cout << "Subcommands: " << std::endl;
        std::cout << "  init                             Initializes a template project" << std::endl;
        std::cout << "  remove                           Remove an existing template" << std::endl;
        std::cout << "  rename                           Rename an existing template" << std::endl;
        std::cout << "  edit                             Edit an existing template" << std::endl;
        std::cout << std::endl;
        std::cout << "Options: " << std::endl;
        std::cout << "  -h, --help                       Print help menu" << std::endl;
        std::cout << "  --version                        Print program version" << std::endl;
        std::cout << "  --set-template-directory         Set the directory to look for templates" << std::endl; 
        std::cout << "  --set-template-editor            Set the editor to use when editing templates" << std::endl;
        std::cout << "  --setup                          Setup the program" << std::endl;
    } else if(subcmd == "init") {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <template> <options> [-p <path>]" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <options>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help                       Print help menu" << std::endl;
        std::cout << "  -l, --list                       Print all existing templates" << std::endl;
        std::cout << "  -p, --path                       The path the template will be initialized on (default is the current path)" << std::endl;
        std::cout << "  -s, --skip-existing              Skip all existing files" << std::endl; 
        std::cout << "  -o, --overwrite-existing         Overwrites all existing files" << std::endl;
        std::cout << "  -f, --force                      Overwrites the whole directory" << std::endl;
    } else if(subcmd == "add") {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <new-template> [-p <path>]" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <options>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help                       Print help menu" << std::endl;
        std::cout << "  -p, --path                       The directory to add as a template (default is the current directory)" << std::endl;
        std::cout << "  -d, --desc                       Add a description" << std::endl;
    } else if(subcmd == "remove") {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <existing-template>" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <options>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help                       Print help menu" << std::endl;
        std::cout << "  -l, --list                       Print all existing templates" << std::endl;
    } else if(subcmd == "edit") {
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <options>" << std::endl;
        std::cout << "  " << program << " " << subcmd << " <existing-template> <options> <values>" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help                       Print help menu" << std::endl;
        std::cout << "  -l, --list                       Print all existing templates" << std::endl;
        std::cout << "  -d, --desc                       Edit the description of a template" << std::endl;
        std::cout << "  -r, --rename                     Rename the template" << std::endl;
    } else {
        throw CLIException("[Error] " + subcmd + " is not a valid subcommand");
    }
}

std::string getTemplateDirectory(const Config& config)
{
    std::string config_template_dir = config.getValue("template_directory");
    return path::isAbsolutePath(config_template_dir) ? config_template_dir : path::joinPath(path::sourcePath(), config_template_dir);
}

void setup()
{
    Config config;
    std::string temp;

    std::cout << "=====SETUP MENU=====" << std::endl;
    std::cout << "The template directory is where the templates will be saved (eg: D:/Documents/My Templates)" << std::endl;
    std::cout << "Enter template directory: ";
    getline(std::cin, temp);
    config.addKeyValue("template_directory", temp);

    std::cout << std::endl;
    std::cout << "The template editor is the program to be used when editing templates (eg: D:/Programs/VS Code)" << std::endl;
    std::cout << "Enter path to template editor: ";
    getline(std::cin, temp);
    config.addKeyValue("template_editor", temp);

    config.saveConfigToFile(path::joinPath(path::sourcePath(), "config.txt"));
}

void listTemplates(const Config& config)
{
    std::string config_template_dir = config.getValue("template_directory");
    std::filesystem::path template_path = getTemplateDirectory(config);

    std::cout << "Templates:" << std::endl;
    for(const auto& entry : std::filesystem::directory_iterator(template_path)) {
        std::string template_name = entry.path().filename().string();
        std::string info_file = path::joinPath(entry.path(), ".template");
        Config info;

        if(path::exists(info_file)) {
            info.setConfigFromFile(info_file);
        }

        std::cout << "  " << template_name;
        if(info.doesKeyExist("description")) {
            std::cout << getSpaces(35, template_name) << info.getValue("description");
        }
        std::cout << std::endl;
    }
}

void initTemplate(const CLI& cli, const Config& config)
{
    std::string template_name = cli.getAnyValue({"-p", "--path"});
    if(template_name.empty()) {
        throw CLIException("[Error] No template provided");
    }

    std::string template_path = path::joinPath(getTemplateDirectory(config), template_name);

    if(!path::exists(template_path)) {
        throw std::runtime_error("[Error] Template \"" + template_name + "\" does not exist");
    }

    path::CopyOption option = path::CopyOption::None;
    if(cli.isFlagActive({"-s", "--skip-existing"})) {
        option = path::CopyOption::SkipExisting;
    } else if(cli.isFlagActive({"-o", "--overwrite-existing"})) {
        option = path::CopyOption::OverwriteExisting;
    }

    std::string to = path::currentPath();
    if(cli.isFlagActive({"-p", "--path"})) {
        to = cli.getValueOf({"-p", "--path"});
    }

    if(cli.isFlagActive({"-f", "--force"})) {
        path::remove(to + '/');
    }
    
    for(const auto& entry : std::filesystem::directory_iterator(template_path)) {
        if(entry.path().filename() == ".template") {
            continue;
        }
        path::copy(entry.path(), to, option);
    }

    std::cout << "[Success] Template \"" + template_name + "\" has been initialized" << std::endl;
}

void addTemplate(const CLI& cli, const Config& config)
{
    std::string path_to_add;
    if(cli.isFlagActive({"-p", "--path"})) {
        path_to_add = cli.getValueOf({"-p", "--path"});
    }

    if(path_to_add.empty() || path_to_add == ".") {
        path_to_add = path::currentPath();
    }

    path_to_add.push_back('/');

    std::string template_name = cli.getValueOf();
    std::string template_path = path::joinPath(getTemplateDirectory(config), template_name);

    if(path::exists(template_path)) {
        char ch;
        std::cout << "[Warning] \"" + template_name + "\" already exists. Would you like to overwrite? [y/n]: ";
        std::cin >> ch;
        if(ch != 'y' && ch != 'Y') {
            return;
        }
    }

    path::createDirectory(template_path);
    path::copy(path_to_add, template_path, path::CopyOption::OverwriteAll);

    Config template_info;
    if(cli.isFlagActive({"-d", "--desc"})) {
        template_info.addKeyValue("description", cli.getValueOf({"-d", "--desc"}));
        template_info.saveConfigToFile(path::joinPath(template_path, ".template"));
    }

    std::cout << "[Success] Template \"" + template_name + "\" has been added" << std::endl;
}

void setAll(CLI& cli)
{
    cli.addSubcommands({"init", "add", "remove", "edit"});
    cli.addGlobalFlags({"-h", "--help"});
    cli.addGlobalFlags({"-l", "--list"}, {"add"});
    cli.addFlags({"--version", "--setup", "--set-template-directory", "--set-template-editor"});
    cli.addFlags("init", {"-l", "--list", "-s", "--skip-existing", 
    "-o", "--overwrite-existing", "-f", "--force", "-p", "--path"});
    cli.addFlags("add", {"-p", "--path", "-d", "--desc"});
    cli.addFlags("edit", {"-d", "--desc", "-r", "--rename"});

    cli.init();
}

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    //cli.setArguments({"template", "init", "-f", "cpp", "-p", "D:\\Documents\\Codes\\VS Code\\C++\\Tools\\Project-Template\\bin\\Debug\\sub"});
    std::string program_name = cli.getProgramName();
    try {
        setAll(cli);
        std::string subcmd = cli.getActiveSubcommand();

        if(cli.isFlagActive({"-h", "--help"})) {
            printHelp(cli);
            return 0;
        }

        if(subcmd.empty() && cli.isFlagActive("--setup")) {
            setup();
            return 0;
        }

        std::string config_path = path::joinPath(path::sourcePath(), "config.txt");
        Config config(config_path);

        if(subcmd != "add" && cli.isFlagActive({"-l", "--list"})) {
            listTemplates(config);
            return 0;
        }

        if(subcmd.empty()) {
            if(cli.isFlagActive("--set-template-directory")) {
                config.modifyKeyValue("template_directory", cli.getValueOf("--set-template-directory"));
                config.saveConfigToFile(config_path);
            }

            if(cli.isFlagActive("--set-template-editor")) {
                config.modifyKeyValue("template_editor", cli.getValueOf("--set-template-editor"));
                config.saveConfigToFile(config_path);
            }
        } else if(subcmd == "init") {
            initTemplate(cli, config);
        } else if(subcmd == "add") {
            addTemplate(cli, config);
        }
    } catch(const CLIException& e) {
        std::cout << e.what() << std::endl;
        printHelp(cli);
        return 1;
    } catch(const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        printHelp(cli);
        return 1;
    }

    return 0;
}