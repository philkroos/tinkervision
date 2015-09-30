#include "filesystem.hh"

#include <iostream>
#include <vector>

class ExtensionFilter {
    std::string extension_;

public:
    explicit ExtensionFilter(std::string const& extension)
        : extension_(extension) {}
    bool operator()(std::string const& fname, std::string const& extension,
                    bool is_file) {
        return is_file and extension == extension_;
    }
};

int main() {
    std::vector<std::string> contents;

    std::cout << "Get list of all .conf files in etc" << std::endl;
    tfv::list_directory_content("/etc/", contents, ExtensionFilter("conf"));

    for (auto const& file : contents) {
        std::cout << file << std::endl;
    }
    contents.clear();

    std::cout << std::endl << "Get list of all entries in tmp" << std::endl;
    tfv::list_directory_content("/tmp/", contents, nullptr);

    for (auto const& file : contents) {
        std::cout << file << std::endl;
    }
    contents.clear();

    std::cout << std::endl << "Get list of all FILES in tmp" << std::endl;
    tfv::list_directory_content("/tmp/", contents,
                                [](std::string const&, std::string const&,
                                   bool is_file) { return is_file; });

    for (auto const& file : contents) {
        std::cout << file << std::endl;
    }
    contents.clear();

    return 0;
}
