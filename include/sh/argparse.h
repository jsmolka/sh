#pragma once

#include <any>
#include <memory>
#include <string_view>
#include <vector>

#include <sh/concepts.h>
#include <sh/fmt.h>
#include <sh/parse.h>

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

  auto operator|(const T& value) -> argument_t<T>& {
    default_value_repr = detail::repr(value);
    default_value = value;
    return *this;
  }

  auto boolean() const -> bool {
    return std::same_as<T, bool>;
  }

  void parse(std::string_view data) {}
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
          value->parse(std::string_view{});
        }
      } else {
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
  }

 private:
  // Todo: better
  argument* find(std::string_view option) {
    for (auto& arg : args_) {
      for (const auto& o : arg->options) {
        if (o == option) {
          return arg.get();
        }
      }
    }
    return nullptr;
  }

  std::vector<std::unique_ptr<argument>> args_;
};

}  // namespace sh
