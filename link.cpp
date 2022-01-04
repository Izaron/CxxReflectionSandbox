#include <iostream>
#include <string>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

template<meta::info R>
consteval void check_functions_linkage() {
    static_assert(meta::is_namespace(R));

    template for (constexpr meta::info e : meta::members_of(R)) {
        if constexpr (meta::is_function(e)) {
            __reflect_dump(e);
            if constexpr (meta::is_externally_linked(e)) {
                constexpr auto error_msg =
                    __concatenate("The method '", meta::name_of(e), "' is externally linked");
                __compiler_error(error_msg);
            }
        }

        if constexpr (meta::is_namespace(e)) {
            check_functions_linkage<e>();
        }
    }
}

namespace outer {
    static bool foo(int i) { return i == 13; }
    static std::string bar(std::string s) { return s + s; };

    namespace inner {
        static double fizz() { return 3.14; }
    } // namespace inner
} // namespace outer

int main() {
    check_functions_linkage<^outer>();
    std::cout << "compiled!" << std::endl;
}
