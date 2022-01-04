#include <iostream>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

template<typename T, int N>
auto make_indirect_template() {
    if constexpr (N == 0) {
        return T{};
    } else {
        return make_indirect_template<T*, N - 1>();
    }
}

consteval meta::info make_pointer(meta::info type, int n) {
    for (int i = 0; i < n; ++i) {
        type = meta::add_pointer(type);
    }
    return type;
}

template<typename T, int N>
auto make_indirect_reflective() {
    return typename [:make_pointer(^T, N):]{};
}

int main() {
    auto ptr1 = make_indirect_template<int, 42>();
    std::cout << meta::name_of(meta::type_of(^ptr1)) << std::endl;

    auto ptr2 = make_indirect_reflective<int, 42>();
    std::cout << meta::name_of(meta::type_of(^ptr2)) << std::endl;
}
