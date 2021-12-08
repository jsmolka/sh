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

// sh::formattable
template <typename T>
auto repr(const T& value) -> std::string {
  if constexpr (std::convertible_to<T, std::string_view>) {
    return fmt::format("\"{}\"", std::string_view{value});
  } else {
    return fmt::format("{}", value);
  }
}

// Todo: better
auto split(std::string_view string, std::string_view delims) -> std::vector<std::string_view> {
  std::vector<std::string_view> output;
  size_t first = 0;

  while (first < string.size()) {
    const auto second = string.find_first_of(delims, first);

    if (first != second) {
      output.emplace_back(string.substr(first, second - first));
    }

    if (second == std::string_view::npos) {
      break;
    }

    first = second + 1;
  }

  return output;
}

}  // namespace detail

class help : public std::string_view {
 public:
  using std::string_view::basic_string_view;
};

class argument {
 public:
  argument(const std::vector<std::string_view>& options) : options(options) {}

  auto required() const -> bool {
    return !default_value.has_value();
  }

  auto positional() const -> bool {
    return !options.front().starts_with('-');
  }

  virtual auto boolean() const -> bool = 0;
  virtual void parse(std::string_view) = 0;

  std::any value;
  std::any default_value;
  std::string default_value_repr;
  std::string_view help;
  std::vector<std::string_view> options;
};

// sh::copy_constructible<T> && sh::parsable<T> && sh::formattable<T>
// std::string???
template <typename T>
concept argument_type = true;

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
      if (auto value = sh::parse<T>(data)) {
        this->value = std::move(*value);
      } else {
        throw std::runtime_error("fak parse");
      }
    }
  }
};

class argument_parser {
 public:
  template <argument_type T, std::convertible_to<std::string_view>... Options>
    requires(sizeof...(Options) > 0)
  auto add(Options&&... options) -> argument_t<T>& {
    auto values = std::initializer_list<std::string_view>{options...};
    auto unique = std::make_unique<argument_t<T>>(values);

    return *reinterpret_cast<argument_t<T>*>(args_.emplace_back(std::move(unique)).get());
  }

  void parse(int argc, const char* const* argv) {
    int idx = 1;
    int pos = 0;

    while (idx < argc) {
      auto arg = std::string_view(argv[idx++]);
      auto kvp = detail::split(arg, "=");  // Todo: split first

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
  T get(std::string_view option) const {
    for (auto& arg : args_) {
      if (contains(arg->options, option)) {
        if (arg->value.has_value()) {
          return std::any_cast<T>(arg->value);
        } else {
          return std::any_cast<T>(arg->default_value);
        }
      }
    }
    return {};
  }

 private:
  // Todo: better
  argument* find(std::string_view option) {
    for (auto& arg : args_) {
      if (contains(arg->options, option)) {
        return arg.get();
      }
    }
    return nullptr;
  }

  void validate() {
    for (const auto& arg : args_) {
      if (arg->required() && !(arg->value.has_value() || arg->default_value.has_value())) {
        throw std::runtime_error("fak validate");
      }
    }
  }

  std::vector<std::unique_ptr<argument>> args_;
};

}  // namespace sh
