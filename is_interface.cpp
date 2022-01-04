#include <iostream>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

namespace traits {

template<typename T>
consteval bool is_interface_impl() {
    constexpr meta::info refl = ^T;
    if constexpr (meta::is_class(refl)) {
        template for (constexpr meta::info e : meta::members_of(refl)) {
            // interfaces SHALL NOT have data members
            if constexpr (meta::is_data_member(e)) {
                return false;
            }
            // every user function in interfaces SHOULD BE public and pure virtual
            if constexpr (meta::is_function(e) && !meta::is_special_member_function(e)) {
                if constexpr (!meta::is_public(e) || !meta::is_pure_virtual(e)) {
                    return false;
                }
            }
            // the destructor SHOULD BE virtual and defaulted
            if constexpr (meta::is_function(e) && meta::is_destructor(e)) {
                if constexpr (!meta::is_public(e) || !meta::is_defaulted(e) || !meta::is_virtual(e)) {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

template<typename T>
constexpr bool is_interface = is_interface_impl<T>();

} // namespace traits

// IS NOT abstract, IS NOT interface
class foo {
public:
    void foo_void();
private:
    int _foo_int;
};
static_assert(not std::is_abstract_v<foo>);
static_assert(not traits::is_interface<foo>);

// IS abstract, IS NOT interface
class bar {
public:
    virtual void bar_void() = 0;
    std::string bar_string();
private:
    int _foo_int;
};
static_assert(    std::is_abstract_v<bar>);
static_assert(not traits::is_interface<foo>);

// IS abstract, IS NOT interface
class fizz {
public:
    virtual void fizz_void() = 0;
    std::string fizz_string();
};
static_assert(    std::is_abstract_v<fizz>);
static_assert(not traits::is_interface<fizz>);

// IS abstract, IS NOT interface
class buzz {
public:
    virtual void buzz_void() = 0;
    virtual std::string buzz_string() = 0;
};
static_assert(    std::is_abstract_v<buzz>);
static_assert(not traits::is_interface<buzz>);

// IS abstract, IS NOT interface
class biba {
public:
    virtual ~biba() { /* ... not defaulted dtor ... */ };
    virtual void biba_void() = 0;
    virtual std::string biba_string() = 0;
};
static_assert(    std::is_abstract_v<biba>);
static_assert(not traits::is_interface<biba>);

// IS abstract, IS interface
class boba {
public:
    virtual ~boba() = default;
    virtual void boba_void() = 0;
    virtual std::string boba_string() = 0;
};
static_assert(    std::is_abstract_v<boba>);
static_assert(    traits::is_interface<boba>);


int main() {
    std::cout << "compiled!" << std::endl;
}
