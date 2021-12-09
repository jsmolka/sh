#pragma once

#include <any>
#include <memory>
#include <string_view>
#include <vector>

#include <sh/concepts.h>
#include <sh/fmt.h>
#include <sh/parse.h>
#include <sh/ranges.h>

namespace sh {

namespace detail {

template <sh::formattable T>
auto repr(const T& value) -> std::string {
  if constexpr (std::convertible_to<T, std::string_view>) {
    return fmt::format("\"{}\"", std::string_view{value});
  } else {
    return fmt::format("{}", value);
  }
}

inline auto split(std::string_view str, char delim) -> std::vector<std::string_view> {
  const auto pos = str.find_first_of(delim);
  if (pos == std::string_view::npos) {
    return {str};
  } else {
    return {str.substr(0, pos), str.substr(pos + 1)};
  }
}

inline auto trim(std::string_view str) -> std::string_view {
  const auto first = str.find_first_not_of(' ');
  if (std::string_view::npos == first) {
    return str;
  }
  const auto last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

}  // namespace detail

class help : public std::string_view {};

class argument {
 public:
  argument(const std::vector<std::string_view>& options) : names(options) {}

  auto required() const -> bool {
    return !default_value.has_value();
  }

  auto positional() const -> bool {
    return !names.front().starts_with('-');
  }

  virtual auto boolean() const -> bool = 0;
  virtual void parse(std::string_view) = 0;

  std::any value;
  std::any default_value;
  std::string default_value_repr;
  std::string_view help;
  std::vector<std::string_view> names;
};

template <typename T>
concept argument_type = sh::copy_constructible<T> && sh::parsable<T> && sh::formattable<T>;

template <argument_type T>
class argument_t : public argument {
 public:
  using argument::argument;

  auto operator|(sh::help help) -> argument_t<T>& {
    this->help = help;
    return *this;
  }

  template <std::convertible_to<T> U>
  auto operator|(const U& value) -> argument_t<T>& {
    const T data(value);
    default_value_repr = detail::repr(data);
    default_value = std::move(data);
    return *this;
  }

  auto boolean() const -> bool {
    return std::same_as<T, bool>;
  }

  void parse(std::string_view data) {
    if (data == std::string_view()) {
      if constexpr (std::same_as<T, bool>) {
        value = true;
      } else {
        throw std::runtime_error("fak bool");
      }
    } else {
      if (auto value = sh::parse<T>(detail::trim(data))) {
        this->value = std::move(*value);
      } else {
        throw std::runtime_error("fak parse");
      }
    }
  }
};

class argument_parser {
 public:
  template <argument_type T, std::convertible_to<std::string_view>... Names>
    requires(sizeof...(Names) > 0)
  auto add(Names&&... names) -> argument_t<T>& {
    auto strings = std::initializer_list<std::string_view>{detail::trim(names)...};
    auto pointer = std::make_unique<argument_t<T>>(strings);

    return *reinterpret_cast<argument_t<T>*>(args_.emplace_back(std::move(pointer)).get());
  }

  void parse(int argc, const char* const* argv) {
    int idx = 1;
    int pos = 0;

    while (idx < argc) {
      auto arg = detail::trim(argv[idx++]);
      auto kvp = detail::split(arg, '=');  // Todo: split first

      if (auto value = find(kvp.front())) {
        if (kvp.size() == 2) {
          value->parse(kvp.back());
        } else if (idx < argc && !value->boolean() && !find(argv[idx])) {
          value->parse(argv[idx++]);
        } else {
          value->parse({});
        }
      } else {
        // Todo: throw if unmatched
        int i = 0;
        for (auto& arg_ : args_) {
          if (arg_->positional()) {
            if (i++ == pos) {
              arg_->parse(arg);
              pos++;
              break;
            }
          }
        }
      }
    }

    validate();
  }

  template <argument_type T>
  auto get(std::string_view name) const -> T {
    if (auto arg = find(detail::trim(name))) {
      if (arg->value.has_value()) {
        return std::any_cast<T>(arg->value);
      } else {
        return std::any_cast<T>(arg->default_value);
      }
    }
    return {};
  }

 private:
  auto find(std::string_view name) const -> argument* {
    for (const auto& arg : args_) {
      if (contains(arg->names, name)) {
        return arg.get();
      }
    }
    return nullptr;
  }

  void validate() {
    for (const auto& arg : args_) {
      if (arg->required() && !arg->value.has_value()) {
        throw std::runtime_error(fmt::format("missing required argument: {}", arg->names.front()));
      }
    }
  }

  std::vector<std::unique_ptr<argument>> args_;
};

}  // namespace sh
