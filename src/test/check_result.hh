#include <iostream>
#include <string>
#include <type_traits>

#include <tinkervision/tinkervision.h>

template<typename T> std::string string(T object) {
    return std::to_string(object);
}

template<>
std::string string<const char*>(const char* object) {
    return object;
}

void get_args(std::string& sargs) {}

template <typename T, typename... Args>
void get_args(std::string& sargs, T const& t) {
    sargs += string(t);
}

template <typename T, typename... Args>
void get_args(std::string& sargs, T const& t, Args... args) {

    sargs += string(t) + ",";
    get_args(sargs, args...);
}

template <typename... Args>
bool check(int16_t result, std::string const& function, Args... args) {
    if (result) {
        std::string sargs;
        get_args(sargs, args...);
        std::cout << "Function failed with " << result << ": " << function
                  << "(" << sargs << "): " << tv_result_string(result) << std::endl;
    }
    return result == TV_OK;
}

template<typename Func, typename... Args>
bool checked(Func function, std::string const& name, Args const&... args) {
    auto result = function(args...);
    check(result, name, args...);

    return result == TV_OK;
}
