#include <iostream>
#include <vector>

#include "module_loader.hh"

int main() {
    auto loader = tv::ModuleLoader("/usr/local/lib/tinkervision",
                                   "/tmp/lib/tinkervision/");
    auto contents = std::vector<std::string>();

    loader.list_available_modules(contents);
    for (auto const& file : contents) {
        std::cout << file << std::endl;
    }

    return 0;
}
