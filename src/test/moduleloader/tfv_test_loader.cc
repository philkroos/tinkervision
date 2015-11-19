#include <unistd.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

#include "module_loader.hh"

static void directory_changes(std::string const& dir, std::string const& file,
                              Dirwatch::Event event) {
    std::cout << "In " << dir << ": " << file << " was ";
    if (event == Dirwatch::Event::FILE_CREATED) {
        std::cout << "created";
    }
    else {
        std::cout << "deleted";
    }
    std::cout << std::endl;
}

static void modify_dir(void) {
    std::string f1("/tmp/lib/tinkervision/ignored");
    std::string f2("/tmp/lib/tinkervision/test.so");

    std::remove(f1.c_str());
    std::remove(f2.c_str());

    std::ofstream ignored(f1);
    std::ofstream lib(f2);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    ignored << "test" << std::endl;
    ignored.close();
    std::cout << "Closed " << f2 << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
    lib << "test" << std::endl;
    lib.close();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::remove(f1.c_str());
    std::remove(f2.c_str());
}

int main() {

    if (getuid() != 0) {
        std::cout << "This test might have to be run as root. It checks for "
                     "updates in and manipulates the user-module-path. Most "
                     "likely the path has been created as root, so a regular "
                     "user can't create or delete files there, which this test "
                     "relies on." << std::endl;
    }

    tv::ModuleLoader loader("/usr/lib/tinkervision", "/tmp/lib/tinkervision/");
    auto modules = std::vector<std::string>();
    auto paths = std::vector<std::string>();

    loader.list_available_modules(paths, modules);
    std::cout << "Found " << modules.size() << " modules." << std::endl;
    for (size_t i = 0; i < modules.size(); ++i) {
        std::cout << modules[i] << " from " << paths[i] << std::endl;
    }

    std::cout << "Registering module-change listener" << std::endl;

    loader.update_on_changes(directory_changes);

    std::thread(&modify_dir).detach();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
