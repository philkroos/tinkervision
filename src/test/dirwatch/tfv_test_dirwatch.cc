#include "dirwatch.hh"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

std::ostream& operator<<(std::ostream& os, tv::Dirwatch::Event event) {
    if (event == tv::Dirwatch::Event::FILE_CREATED) {
        os << "File Created";
    }
    else if (event == tv::Dirwatch::Event::FILE_DELETED) {
        os << "File Deleted";
    }
    else if (event == tv::Dirwatch::Event::DIR_DELETED) {
        os << "Dir Deleted";
    }

    else {
        os << "Unknown event";
    }
    return os;
}

void dirwatch_callback(tv::Dirwatch::Event event, std::string const& dir, std::string const& file) {
    std::cout << "Dirwatch event for " << dir << "/" << file << ": " << event << std::endl;
}

int main() {

    system("mkdir ./watched-dir");

    tv::Dirwatch dirwatch(dirwatch_callback);
    dirwatch.watch("./watched-dir");
    dirwatch.add_watched_extension("so");

    system("touch ./watched-dir/test.so");
    system("rm ./watched-dir/test.so");
    system("touch ./watched-dir/test.so");
    system("mv ./watched-dir/test.so ./");
    system("mv ./test.so ./watched-dir");
    system("rm -rf ./watched-dir");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}
