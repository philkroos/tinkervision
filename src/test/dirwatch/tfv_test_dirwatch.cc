#include "dirwatch.hh"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

std::ostream& operator<<(std::ostream& os, tv::Dirwatch::Event event) {
    if (event == tv::Dirwatch::Event::FILE_CREATED) {
        os << "File Created";
    } else if (event == tv::Dirwatch::Event::FILE_DELETED) {
        os << "File Deleted";
    } else if (event == tv::Dirwatch::Event::DIR_DELETED) {
        os << "Dir Deleted";
    }

    else {
        os << "Unknown event";
    }
    return os;
}

void dirwatch_callback(tv::Dirwatch::Event event, std::string const& dir,
                       std::string const& file) {
    std::cout << "Dirwatch event for " << dir << "/" << file << ": " << event
              << std::endl;
}

int main() {

    system("mkdir ./watched-dir");

    tv::Dirwatch dirwatch(dirwatch_callback);
    dirwatch.watch("./watched-dir");
    dirwatch.add_watched_extension("so");

    // Creating a file
    std::cout << "--> Expecting Event::Create" << std::endl;
    system("touch ./watched-dir/test.so");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Deleting a file
    std::cout << "--> Expecting Event::Delete" << std::endl;
    system("rm ./watched-dir/test.so");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Moving into watched dir equals creating a file
    system("touch ./test.so");
    std::cout << "--> Expecting Event::Create" << std::endl;
    system("mv ./test.so ./watched-dir");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Moving from watched dir equals deleting a file
    std::cout << "--> Expecting Event::Delete" << std::endl;
    system("mv ./watched-dir/test.so ./");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Deleting the directory itself
    std::cout << "--> Expecting Event::DeleteDir" << std::endl;
    system("rm -rf ./watched-dir");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
