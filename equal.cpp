#include <iostream>
#include <optional>
#include <vector>
#include <concepts>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

namespace bicycle {

template <class T>
constexpr bool equality_comparable = requires(const T& a, const T& b) {
    std::is_convertible_v<decltype(a == b), bool>;
};

template <class T>
constexpr bool iterable = requires(const T& t, size_t i) {
    t[i];
    std::begin(t);
    std::end(t);
    std::size(t);
};

} // namespace bicycle

namespace model {

struct book {
    std::string name;
    std::string author;
    int year;
};

struct latlon {
    double lat;
    double lon;
};

struct library {
    std::vector<book> books;
    std::optional<std::string> description;
    latlon address;
};

} // namespace model

namespace equal_util {

template<typename T>
bool equal(const T& a, const T& b) {
    if constexpr (bicycle::iterable<T>) {
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            if (!equal(a[i], b[i]))
                return false;
        }
        return true;
    } else if constexpr (bicycle::equality_comparable<T>) {
        return a == b;
    } else {
        template for (constexpr meta::info e : meta::members_of(^T)) {
            if constexpr (meta::is_data_member([:^e:])) {
                if (!equal(a.[:e:], b.[:e:])) {
                    return false;
                }
            }
        }
        return true;
    }
}

} // namespace equal_util

static_assert(    bicycle::equality_comparable<int>);
static_assert(    bicycle::equality_comparable<std::string>);
static_assert(    bicycle::equality_comparable<std::optional<std::string>>);
static_assert(    bicycle::equality_comparable<std::vector<model::book>>); // <<< :(
static_assert(not bicycle::equality_comparable<model::book>);
static_assert(not bicycle::equality_comparable<model::library>);

static_assert(not bicycle::iterable<model::book>);
static_assert(not bicycle::iterable<model::library>);
static_assert(    bicycle::iterable<std::vector<model::book>>);

int main() {
    model::library a, b;

    a.address = model::latlon{.lat = 51.507351, .lon = -0.127696};
    b.address = a.address;

    a.books.push_back(model::book{
        .name = "The Picture of Dorian Gray",
        .author = "Oscar Wilde",
        .year = 1890,
    });
    b.books = a.books;

    std::cout << std::boolalpha;
    std::cout << equal_util::equal(a, b) << std::endl;
    b.books.clear();
    std::cout << equal_util::equal(a, b) << std::endl;
}
