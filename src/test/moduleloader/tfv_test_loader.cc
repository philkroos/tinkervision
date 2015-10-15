#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

#include "module_loader.hh"

static void directory_changes(std::string const& dir, std::string const& file,
                              bool created) {
    std::cout << "In " << dir << ": " << file << " was "
              << (created ? "created" : "deleted") << std::endl;
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
    tv::ModuleLoader loader("/usr/lib/tinkervision", "/tmp/lib/tinkervision/");
    auto contents = std::vector<std::string>();

    loader.list_available_modules(contents);
    for (auto const& file : contents) {
        std::cout << file << std::endl;
    }

    loader.update_on_changes(directory_changes);

    std::thread(&modify_dir).detach();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
