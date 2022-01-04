#include <iostream>
#include <string>
#include <concepts>
#include <queue>
#include <map>
#include <set>
#include <array>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;


namespace json {

template<typename T>
concept JsonNumber = std::is_arithmetic_v<T>;

template<typename T>
concept JsonString = std::same_as<std::string, T>;

template<typename T>
concept JsonBoolean = std::same_as<bool, T>;


static constexpr meta::info vector_refl = ^std::vector;
static constexpr meta::info array_refl = ^std::array;
static constexpr meta::info deque_refl = ^std::deque;
static constexpr meta::info list_refl = ^std::list;
static constexpr meta::info forward_list_refl = ^std::forward_list;

template<typename T>
consteval bool is_json_array_impl() {
    if constexpr (meta::is_specialization(^T)) {
        constexpr auto tmpl = meta::template_of(^T);
        constexpr bool result =
            tmpl == vector_refl || tmpl == array_refl ||
            tmpl == deque_refl || tmpl == list_refl ||
            tmpl == forward_list_refl;
        return result;
    }
    return false;
}

template<typename T>
concept JsonArray = is_json_array_impl<T>();

template<typename T>
concept JsonObject = std::is_class_v<T>;


static constexpr meta::info optional_refl = ^std::optional;

template<typename T>
consteval bool is_json_nullable_impl() {
    if constexpr (meta::is_specialization(^T)) {
        return meta::template_of(^T) == optional_refl;
    }
    return false;
}

template<typename T>
concept JsonNullable = is_json_nullable_impl<T>();


template<typename Out>
class json_writer {
public:
    json_writer(Out& out)
        : _out{out}
    {}

    template<typename T>
    void write(T const& t) {
        if constexpr (JsonNullable<T>) {
            write_nullable(t);
        } else if constexpr (JsonNumber<T>) {
            write_number(t);
        } else if constexpr (JsonString<T>) {
            write_string(t);
        } else if constexpr (JsonBoolean<T>) {
            write_boolean(t);
        } else if constexpr (JsonArray<T>) {
            write_array(t);
        } else if constexpr (JsonObject<T>) {
            write_object(t);
        }
    }

private:
    template<JsonNullable T>
    void write_nullable(T const& t) {
        if (t.has_value()) {
            write(*t);
        } else {
            _out << "null";
        }
    }

    template<JsonNumber T>
    void write_number(const T t) {
        _out << t;
    }

    template<JsonString T>
    void write_string(T const& t) {
        _out << '"' << t << '"';
    }

    template<JsonBoolean T>
    void write_boolean(const T t) {
        if (t) {
            _out << "true";
        } else {
            _out << "false";
        }
    }

    template<JsonArray T>
    void write_array(T const& t) {
        _out << '[';
        bool is_first_item = true;
        for (const auto& item : t) {
            if (is_first_item) {
                is_first_item = false;
            } else {
                _out << ',';
            }
            write(item);
        }
        _out << ']';
    }

    template<JsonObject T>
    void write_object(T const& t) {
        _out << '{';
        bool is_first_member = true;

        template for (constexpr meta::info e : meta::members_of(^T)) {
            if constexpr (meta::is_data_member([:^e:])) {
                if (is_first_member) {
                    is_first_member = false;
                } else {
                    _out << ',';
                }

                _out << '"' << meta::name_of(e) << '"';
                _out << ':';
                write(t.[:e:]);
            }
        }

        _out << '}';
    }

private:
    Out& _out;
};

} // namespace json


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


int main() {
    model::library l;
    l.address = model::latlon{.lat = 51.507351, .lon = -0.127696};
    l.books.push_back(model::book{
        .name = "The Picture of Dorian Gray",
        .author = "Oscar Wilde",
        .year = 1890,
    });
    l.books.push_back(model::book{
        .name = "Fahrenheit 451",
        .author = "Ray Bradbury",
        .year = 1953,
    });
    l.books.push_back(model::book{
        .name = "Roadside Picnic",
        .author = "Arkady and Boris Strugatsky",
        .year = 1972,
    });

    json::json_writer{std::cout}.write(l);
    std::cout << std::endl;
}
