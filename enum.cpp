#include <iostream>
#include <optional>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

namespace enum_util {

template<typename T>
requires std::is_enum_v<T>
constexpr std::string_view to_string(T value) {
    template for (constexpr meta::info e : meta::members_of(^T)) {
        if ([:e:] == value) {
            return meta::name_of(e);
        }
    }
    throw std::runtime_error("Unknown enum value");
}

template<typename T>
requires std::is_enum_v<T>
constexpr std::optional<T> from_string(std::string_view value) {
    template for (constexpr meta::info e : meta::members_of(^T)) {
        if (meta::name_of(e) == value) {
            return [:e:];
        }
    }
    return {};
}

} // namespace enum_util

enum LightColor { Red, Green, Blue };

void print_name(LightColor color) {
    const std::string_view str = enum_util::to_string(color);
    std::cout << "The color name is " << str << std::endl;
}

int main() {
    print_name(Red);
    print_name(Green);
    print_name(Blue);
    try {
        print_name(static_cast<LightColor>(1337));
    } catch (const std::runtime_error& e) {
        std::cout << "exception is: " << e.what() << std::endl;
    }
}

static_assert(enum_util::from_string<LightColor>("Red").value() == Red);
static_assert(enum_util::from_string<LightColor>("Green").value() == Green);
static_assert(enum_util::from_string<LightColor>("Blue").value() == Blue);
static_assert(not enum_util::from_string<LightColor>("Magenta").has_value());
