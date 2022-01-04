#include <iostream>
#include <optional>
#include <vector>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

class book {
public:
    book(std::string name)
        : _name{std::move(name)}
    {}

    const std::string& get_name() const {
        return _name;
    }

private:
    std::string _name;
};

class library {
public:
    void add_book(book b) {
        _books.push_back(std::move(b));
    }

    bool has_book(std::string_view name) const {
        return std::any_of(_books.begin(), _books.end(), [name](const book& b) {
            return b.get_name() == name;
        });
    };

    size_t get_books_count() const {
        return _books.size();
    }

private:
    std::vector<book> _books;
};

template<typename T> requires std::is_class_v<T>
constexpr bool hasattr(std::string_view name) {
    template for (constexpr meta::info e : meta::members_of(^T)) {
        if (meta::name_of(e) == name) {
            return true;
        }
    }
    return false;
}

template<typename T> requires std::is_class_v<T>
consteval meta::info getattr(std::string_view name) {
    template for (constexpr meta::info e : meta::members_of(^T)) {
        if (meta::name_of(e) == name && meta::is_public(e)) {
            return e;
        }
    }
    throw std::runtime_error("No such attribute");
}

template<typename T> requires std::is_class_v<T>
consteval meta::info getattr(const T& /* t */, std::string_view name) {
    return getattr<T>(name);
}

static_assert(    hasattr<library>("add_book"));
static_assert(not hasattr<library>("remove_book"));
static_assert(    hasattr<library>("get_books_count"));
static_assert(not hasattr<library>("you_dont_have_this_attr"));

int main() {
    library l;
    l.add_book(book("Book1"));
    l.add_book(book("Book2"));
    l.add_book(book("Book3"));

    std::cout << "Count: " << l.get_books_count() << std::endl;
    std::cout << "Count: " << l.[:getattr<library>("get_books_count"):]() << std::endl;
    std::cout << "Count: " << l.[:getattr<decltype(l)>("get_books_count"):]() << std::endl;
    std::cout << "Count: " << l.[:getattr(l, "get_books_count"):]() << std::endl;
}
