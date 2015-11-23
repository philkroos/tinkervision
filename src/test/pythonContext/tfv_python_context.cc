#include <iostream>

#include "environment.hh"

namespace tv {
// Needing this to access Environment, which has a private constructor.
class Api {
    Environment env;

public:
    Environment& environment(void) { return env; }
};
}

int main() {
    std::cout << "Getting context..." << std::endl;

    auto path = std::string(getenv("HOME"));

    tv::Api api;

    // this is the default prefix. Has to contain certain folders,
    // see Makefile of tinkervision, target user_paths.
    api.environment().set_user_prefix(path + "/tv");

    std::cout << tv::Environment::Python().load("tv_py").execute().result()
              << std::endl;
}
