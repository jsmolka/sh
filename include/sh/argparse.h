#pragma once

#include <any>
#include <memory>
#include <string_view>
#include <vector>

#include <sh/concepts.h>
#include <sh/filesystem.h>
#include <sh/fmt.h>
#include <sh/parse.h>
#include <sh/ranges.h>

namespace sh {

namespace {

template <formattable T>
auto repr(const T& value) -> std::string {
  if constexpr (std::convertible_to<T, std::string_view>) {
    return fmt::format("\"{}\"", std::string_view{value});
  } else if constexpr (std::same_as<T, filesystem::path>) {
    return fmt::format("\"{}\"", value);
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
struct value_type {
  using type = T;
};

template <typename T>
struct value_type<std::optional<T>> {
  using type = T;
};

template <typename T>
using value_type_t = typename value_type<T>::type;

template <typename T>
concept serializable = parsable<value_type_t<T>> && formattable<value_type_t<T>>;

}  // namespace

class description : public std::string_view {};

class basic_argument {
 public:
  basic_argument(const std::vector<std::string_view>& options) : names(options) {}

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
  std::string_view description;
  std::vector<std::string_view> names;
};

template <serializable T>
class argument final : public basic_argument {
 public:
  using basic_argument::basic_argument;

  auto operator|(sh::description description) -> argument<T>& {
    this->description = description;
    return *this;
  }

  template <std::convertible_to<T> U>
  auto operator|(const U& data) -> argument<T>& {
    const T value(data);
    default_value_repr = repr(value);
    default_value = value;
    return *this;
  }

  auto boolean() const -> bool {
    return std::same_as<T, bool>;
  }

  auto optional() const -> bool {
    return false;
  }

  void parse(std::string_view data) {
    if (data == std::string_view()) {
      if constexpr (std::same_as<T, bool>) {
        value = true;
      } else {
        throw std::runtime_error(fmt::format("expected data for argument: {}", names.front()));
      }
    } else {
      data = trim(data);
      if (const auto result = sh::parse<T>(data)) {
        value = *result;
      } else {
        throw std::runtime_error(fmt::format("cannot parse: {}", data));
      }
    }
  }
};

template <typename T>
class argument<std::optional<T>> final : public basic_argument {
 public:
  using basic_argument::basic_argument;

  auto operator|(sh::description description) -> argument<std::optional<T>>& {
    this->description = description;
    return *this;
  }

  template <std::convertible_to<T> U>
  auto operator|(const U& data) -> argument<std::optional<T>>& {
    const T value(data);
    default_value_repr = repr(value);
    default_value = std::optional(value);
    return *this;
  }

  auto boolean() const -> bool {
    return std::same_as<T, bool>;
  }

  auto optional() const -> bool {
    return true;
  }

  void parse(std::string_view data) {
    if (data == std::string_view()) {
      if constexpr (std::same_as<T, bool>) {
        value = std::optional(true);
      } else {
        throw std::runtime_error(fmt::format("expected data for argument: {}", names.front()));
      }
    } else {
      data = trim(data);
      if (const auto result = sh::parse<T>(data)) {
        value = result;
      } else {
        throw std::runtime_error(fmt::format("expected data for argument: {}", names.front()));
      }
    }
  }
};

class argument_parser {
 public:
  argument_parser(std::string_view program) : program_(program) {}

  template <serializable T, std::convertible_to<std::string_view>... Names>
    requires not_empty<Names...>
  auto add(Names&&... names) -> argument<T>& {
    const auto views = {trim(names)...};
    arguments_.push_back(std::make_unique<argument<T>>(views));
    return *static_cast<argument<T>*>(arguments_.back().get());
  }

  void parse(int argc, const char* const* argv) {
    auto index = 1;
    auto positional = 0;
    while (index < argc) {
      const auto data = trim(argv[index++]);
      const auto pair = split(data, '=');
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

  template <serializable T>
  auto get(std::string_view name) const -> T {
    if (const auto argument = find(name)) {
      if (argument->value.has_value()) {
        return std::any_cast<T>(argument->value);
      } else if (argument->default_value.has_value()) {
        return std::any_cast<T>(argument->default_value);
      }
    }
    return {};
  }

  auto help() const -> std::string {
    std::string help(fmt::format("usage:\n  {}", program_));

    std::size_t widest = 0;
    std::vector<std::tuple<std::string, std::string, bool>> arguments;
    for (const auto& argument : arguments_) {
      std::string usage(argument->names.front());
      if (argument->positional()) {
        usage = "<" + usage + ">";
      } else if (!argument->boolean()) {
        usage.append(" <value>");
      }
      if (argument->optional()) {
        usage = "[" + usage + "]";
      }
      help.append(" ");
      help.append(usage);

      const auto names = fmt::format("{}", fmt::join(argument->names, ", "));
      widest = std::max(widest, names.size());

      std::string description(argument->description);
      if (argument->default_value.has_value()) {
        description.append(fmt::format(" [default: {}]", argument->default_value_repr));
      } else if (argument->optional()) {
        description.append(" [optional]");
      }
      arguments.emplace_back(names, description, argument->positional());
    }

    std::string keyword;
    std::string positional;
    for (const auto& [names, description, is_positional] : arguments) {
      auto& group = is_positional ? positional : keyword;
      group.append(fmt::format("\n  {:<{}}{}", names, widest + 4, description));
    }

    if (!keyword.empty()) {
      help.append("\n\nkeyword arguments:");
      help.append(keyword);
    }
    if (!positional.empty()) {
      help.append("\n\npositional arguments:");
      help.append(positional);
    }
    return help;
  }

 private:
  auto find(std::string_view name) const -> basic_argument* {
    name = trim(name);
    for (const auto& argument : arguments_) {
      if (contains(argument->names, name)) {
        return argument.get();
      }
    }
    return nullptr;
  }

  auto find(std::size_t position) const -> basic_argument* {
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

  std::string_view program_;
  std::vector<std::unique_ptr<basic_argument>> arguments_;
};

}  // namespace sh
