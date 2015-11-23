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

/* Copy the following into tv_py.py in $(HOME)/tv/scripts.

class Test:
    def __init__(self):
        self.my_dict = {}

    def set(self, string, value):
        self.my_dict[string] = value

    def get(self, string):
        if len(self.my_dict) == 0:
            return "Empty"
        return self.my_dict[string]

t = Test()

def tv_external_set(string, val):
    t.set(string, val)
    return "ok"

def tv_external_set2(val):
    t.set("Value", val)
    return "ok2"

def tv_external_get(string):
    return t.get(string)

*/

int main() {
    std::cout << "Getting context..." << std::endl;

    auto path = std::string(getenv("HOME"));

    tv::Api api;

    // this is the default prefix. Has to contain certain folders,
    // see Makefile of tinkervision, target user_paths.
    api.environment().set_user_prefix(path + "/tv");

    std::cout << tv::Environment::Python()
                     .load("tv_py")
                     .call("tv_external_set2", 3)
                     .call("tv_external_get", "Value")
                     .result() << std::endl;
}
