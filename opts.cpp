#include <iostream>
#include <optional>

#include <experimental/meta>
#include <experimental/compiler>
using namespace std::experimental;

namespace opts_util {

namespace details {

template<typename T>
void parse_value(T& value, const std::string_view arg) {
    if constexpr (std::is_same_v<T, std::string>) {
        value = arg;
    } else if constexpr (std::is_same_v<T, int>) {
        value = std::atoi(arg.data());
    } else if constexpr (std::is_same_v<T, double>) {
        value = std::atof(arg.data());
    } else if constexpr (std::is_same_v<T, bool>) {
        value = (arg == "true" || arg == "1");
    } else {
        []<bool flag = false>(){ static_assert(flag, "met an unmatched type"); }();
    }
}

std::optional<std::string_view> try_parse_arg_value(const std::string_view member_name,
                                                    std::string_view arg)
{
    const std::string long_arg_pattern = "--" + std::string{member_name} + "=";
    const std::string short_arg_pattern = "-" + std::string{member_name[0]} + "=";
    for (const auto& pattern : {long_arg_pattern, short_arg_pattern}) {
        if (arg.starts_with(pattern)) {
            arg.remove_prefix(pattern.size());
            return arg;
        }
    }
    return {};
}

template<typename Opts>
void parse_opt(Opts& opts, const std::string_view arg) {
    template for (constexpr meta::info e : meta::members_of(^Opts)) {
        if constexpr (meta::is_nonstatic_data_member([:^e:])) {
            constexpr std::string_view member_name = meta::name_of(e);

            if (const auto arg_value = try_parse_arg_value(member_name, arg)) {
                parse_value(opts.[:e:], *arg_value);
                break;
            }
        }
    }
}

} // namespace details

template<typename Opts> requires std::is_class_v<Opts>
Opts parse(int argc, char** argv) {
    Opts opts{};
    for (int i = 0; i < argc; ++i) {
        details::parse_opt(opts, argv[i]);
    }
    return opts;
}

} // namespace opts_util

struct program_opts {
    std::string folder; // --folder or -f
    int level = 10; // --level or -l
    bool help = false; // --help or -h
    double leverage = 100.0; // --leverage
};

// example arguments:
// ./bin -ar -l=23 f --leverage=228 df -fd levels -f=fdf -h=true
int main(int argc, char** argv) {
    std::cout << "arguments count: " << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "arg #" << i << ": " << argv[i] << std::endl;
    }
    std::cout << std::endl;

    const auto opts = opts_util::parse<program_opts>(argc, argv);
    std::cout << "options are:" << std::endl;
    std::cout << "folder: " << opts.folder << std::endl;
    std::cout << "level: " << opts.level << std::endl;
    std::cout << "help: " << opts.help << std::endl;
    std::cout << "leverage: " << opts.leverage << std::endl;
}
