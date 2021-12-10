#pragma once

#include <any>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <sh/concepts.h>
#include <sh/fmt.h>
#include <sh/parse.h>
#include <sh/ranges.h>
#include <sh/traits.h>

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

template <typename T>
inline constexpr bool is_optional_v = is_specialization_v<T, std::optional>;

template <typename T>
concept type = (sh::value_constructible<T> && sh::copy_constructible<T> && sh::parsable<T> &&
                sh::formattable<T>);

template <typename T>
concept optional_type = is_optional_v<T> && type<typename T::value_type>;

}  // namespace detail

class help : public std::string_view {};

class argument {
 public:
  argument(const std::vector<std::string_view>& options) : names(options) {}

  auto required() const -> bool {
    return !(optional() || default_value.has_value());
  }

  auto positional() const -> bool {
    return !names.front().starts_with('-');
  }

  virtual auto boolean() const -> bool = 0;
  virtual auto optional() const -> bool = 0;
  virtual void parse(std::string_view) = 0;

  std::any value;
  std::any default_value;
  std::string default_value_repr;
  std::string_view help;
  std::vector<std::string_view> names;
};

template <typename T>
  requires detail::type<T> || detail::optional_type<T>
class argument_t : public argument {
 public:
  using argument::argument;

  auto operator|(sh::help help) -> argument_t<T>& {
    this->help = help;
    return *this;
  }

  template <std::convertible_to<T> U>
  auto operator|(const U& data) -> argument_t<T>& {
    const T value(data);
    default_value_repr = detail::repr(value);
    default_value = std::move(value);
    return *this;
  }

  auto boolean() const -> bool final {
    return std::same_as<T, bool>;
  }

  auto optional() const -> bool final {
    return false;
  }

  void parse(std::string_view data) {
    if (data == std::string_view()) {
      if constexpr (std::same_as<T, bool>) {
        value = true;
      } else {
        throw std::runtime_error("fak bool");
      }
    } else {
      data = detail::trim(data);
      if (const auto value = sh::parse<T>(data)) {
        this->value = std::move(*value);
      } else {
        throw std::runtime_error(fmt::format("cannot parse: {}", data));
      }
    }
  }
};

template <detail::type T>
class argument_t<std::optional<T>> : public argument {
 public:
  using argument::argument;

  auto operator|(sh::help help) -> argument_t<std::optional<T>>& {
    this->help = help;
    return *this;
  }

  template <std::convertible_to<T> U>
  auto operator|(const U& data) -> argument_t<std::optional<T>>& {
    const T value(data);
    default_value_repr = detail::repr(value);
    default_value = std::optional(std::move(value));
    return *this;
  }

  auto boolean() const -> bool final {
    return std::same_as<T, bool>;
  }

  auto optional() const -> bool final {
    return true;
  }

  void parse(std::string_view data) {
    if (data == std::string_view()) {
      if constexpr (std::same_as<T, bool>) {
        value = std::optional(true);
      } else {
        throw std::runtime_error("fak bool");
      }
    } else {
      data = detail::trim(data);
      if (const auto value = sh::parse<T>(data)) {
        this->value = value;
      } else {
        throw std::runtime_error(fmt::format("cannot parse: {}", data));
      }
    }
  }
};

class argument_parser {
 public:
  template <typename T, std::convertible_to<std::string_view>... Names>
    requires(sizeof...(Names) > 0 && (detail::type<T> || detail::optional_type<T>))
  auto add(Names&&... names) -> argument_t<T>& {
    auto strings = std::initializer_list<std::string_view>{detail::trim(names)...};
    auto pointer = std::make_unique<argument_t<T>>(strings);

    return *reinterpret_cast<argument_t<T>*>(arguments_.emplace_back(std::move(pointer)).get());
  }

  void parse(int argc, const char* const* argv) {
    auto index = 1;
    auto positional = 0;
    while (index < argc) {
      const auto data = detail::trim(argv[index++]);
      const auto pair = detail::split(data, '=');
      if (const auto argument = find(pair.front())) {
        if (pair.size() == 2) {
          argument->parse(pair.back());
        } else if (index < argc && !argument->boolean() && !find(argv[index])) {
          argument->parse(argv[index++]);
        } else {
          argument->parse({});
        }
      } else {
        if (const auto argument = find(positional++)) {
          argument->parse(data);
        } else {
          throw std::runtime_error(fmt::format("unmatched positional argument: {}", data));
        }
      }
    }
    validate();
  }

  template <typename T>
    requires detail::type<T> || detail::optional_type<T>
  auto get(std::string_view name) const -> T {
    if (auto argument = find(name)) {
      if (argument->value.has_value()) {
        return std::any_cast<T>(argument->value);
      } else if (argument->default_value.has_value()) {
        return std::any_cast<T>(argument->default_value);
      }
    }
    return {};
  }

 private:
  auto find(std::string_view name) const -> argument* {
    name = detail::trim(name);
    for (const auto& argument : arguments_) {
      if (contains(argument->names, name)) {
        return argument.get();
      }
    }
    return nullptr;
  }

  auto find(std::size_t position) const -> argument* {
    for (const auto& argument : arguments_) {
      if (argument->positional() && position-- == 0) {
        return argument.get();
      }
    }
    return nullptr;
  }

  void validate() {
    for (const auto& argument : arguments_) {
      if (argument->required() && !argument->value.has_value()) {
        throw std::runtime_error(
            fmt::format("missing required argument: {}", argument->names.front()));
      }
    }
  }

  std::vector<std::unique_ptr<argument>> arguments_;
};

}  // namespace sh
