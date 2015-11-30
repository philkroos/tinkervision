#include <unistd.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

#include "module_loader.hh"
#include "tinkervision_defines.h"

static void directory_changes(std::string const& dir, std::string const& file,
                              Dirwatch::Event event) {
    std::cout << "In " << dir << ": " << file << " was ";
    if (event == Dirwatch::Event::FILE_CREATED) {
        std::cout << "created";
    } else {
        std::cout << "deleted";
    }
    std::cout << std::endl;
}

static void modify_dir(std::string const& dir) {
    std::string f1(dir + "ignored");
    // New 11-30-2015: test.so will be ignored because it is not a valid module.
    std::string f2(dir + "test.so");

    std::remove(f1.c_str());
    std::remove(f2.c_str());

    std::ofstream ignored(f1);
    std::ofstream lib(f2);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ignored << "test" << std::endl;
    ignored.close();
    std::cout << "Created " << f1 << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    lib << "test" << std::endl;
    lib.close();
    std::cout << "Created " << f2 << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

namespace tv {
// Needing this to access Environment, which has a private constructor.
class Api {
    Environment env;

public:
    Environment& environment(void) { return env; }
};
}

int main() {

    if (getuid() != 0) {
        std::cout << "This test might have to be run as root. It checks for "
                     "updates in and manipulates the user-module-path. Most "
                     "likely the path has been created as root, so a regular "
                     "user can't create or delete files there, which this test "
                     "relies on." << std::endl;
        std::cout
            << "This currently does not work with the dummy module due to "
               "unresolved symbols.  This is specific for this test and can "
               "be ignored by deleting the dummy.so file." << std::endl;
    }

    tv::Api api;
    auto& env = api.environment();
    if (not env.set_user_prefix(USR_PREFIX)) {
        std::cout << "Could not set user prefix to " << USR_PREFIX << std::endl;
    }

    tv::ModuleLoader loader(env);
    auto modules = std::vector<std::string>();
    auto paths = std::vector<std::string>();

    loader.list_available_modules(paths, modules);
    std::cout << "Found " << modules.size() << " modules." << std::endl;
    for (size_t i = 0; i < modules.size(); ++i) {
        std::cout << modules[i] << " from " << paths[i] << std::endl;
    }

    std::cout << "Registering module-change listener" << std::endl;

    loader.update_on_changes(directory_changes);

    std::thread(&modify_dir, env.user_modules_path()).detach();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
