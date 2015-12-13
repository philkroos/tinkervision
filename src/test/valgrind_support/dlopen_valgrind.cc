#include <iostream>
#include <chrono>
#include <thread>
#include <dlfcn.h>

int main() {
    std::string libname{"colormatch"};
    auto handle = dlopen((LIB_ROOT + libname + ".so").c_str(), RTLD_LAZY);
    if (not handle) {
        std::cout << "Could not open " << LIB_ROOT << libname << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (dlclose(handle)) {
        std::cout << "Error from dlclose: " << errno << std::endl;
    }

    return 0;
}
