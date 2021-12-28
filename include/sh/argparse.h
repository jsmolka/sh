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
#include <sh/vector.h>

namespace sh {

namespace {

template<formattable T>
auto repr(const T& value) -> std::string {
  if constexpr (std::convertible_to<T, std::string_view>) {
    return fmt::format(R"("{}")", std::string_view{value});
  } else if constexpr (std::same_as<T, filesystem::path>) {
    return fmt::format(R"("{}")", value);
  } else {
    return fmt::to_string(value);
  }
}

inline auto split(std::string_view str, char delimiter) -> vector<std::string_view, 2> {
  const auto pos = str.find_first_of(delimiter);
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
  return str.substr(first, last - first + 1);
}

template<typename T>
struct value_type {
  using type = T;
};

template<typename T>
struct value_type<std::optional<T>> {
  using type = T;
};

template<typename T>
using value_type_t = typename value_type<T>::type;

template<typename T>
concept argument_type = parsable<value_type_t<T>> && formattable<value_type_t<T>>;

}  // namespace

class description : public std::string_view {};

class basic_argument {
public:
  basic_argument(const std::vector<std::string_view>& names)
      : names(names) {}

  auto required() const -> bool {
    return !(optional() || default_value.has_value());
  }

  auto positional() const -> bool {
    return !names.front().starts_with('-');
  }

  auto usage() const -> std::string {
    std::string string(names.front());
    if (positional()) {
      string = fmt::format("<{}>", string);
    } else if (!boolean()) {
      string.append(" <value>");
    }
    if (!required()) {
      string = fmt::format("[{}]", string);
    }
    return string;
  }

  auto description() const -> std::string {
    std::vector<std::string> strings;
    if (!description_.empty()) {
      strings.emplace_back(description_);
    }
    if (default_value.has_value()) {
      strings.emplace_back(fmt::format("[default: {}]", default_value_repr_));
    } else if (optional()) {
      strings.emplace_back("[optional]");
    }
    return fmt::to_string(fmt::join(strings, " "));
  }

  virtual auto boolean() const -> bool = 0;
  virtual auto optional() const -> bool = 0;
  virtual void parse(std::string_view) = 0;

  std::any value;
  std::any default_value;
  std::vector<std::string_view> names;

protected:
  void expected_data() {
    throw std::runtime_error(fmt::format("expected data for argument: {}", names.front()));
  }

  void cannot_parse(std::string_view data) {
    throw std::runtime_error(fmt::format("cannot parse argument data: {}", data));
  }

  std::string default_value_repr_;
  std::string_view description_;
};

template<argument_type T>
class argument final : public basic_argument {
public:
  using basic_argument::basic_argument;

  auto operator|(sh::description description) -> argument<T>& {
    description_ = description;
    return *this;
  }

  template<std::convertible_to<T> U>
  auto operator|(const U& data) -> argument<T>& {
    const T value(data);
    default_value_repr_ = repr(value);
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
        expected_data();
      }
    } else {
      data = trim(data);
      if (const auto result = sh::parse<T>(data)) {
        value = *result;
      } else {
        cannot_parse(data);
      }
    }
  }
};

template<typename T>
class argument<std::optional<T>> final : public basic_argument {
public:
  using basic_argument::basic_argument;

  auto operator|(sh::description description) -> argument<std::optional<T>>& {
    description_ = description;
    return *this;
  }

  template<std::convertible_to<T> U>
  auto operator|(const U& data) -> argument<std::optional<T>>& {
    const T value(data);
    default_value_repr_ = repr(value);
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
        expected_data();
      }
    } else {
      data = trim(data);
      if (const auto result = sh::parse<T>(data)) {
        value = result;
      } else {
        cannot_parse(data);
      }
    }
  }
};

class argument_parser {
public:
  argument_parser(std::string_view program)
      : program_(program) {}

  template<argument_type T, std::convertible_to<std::string_view>... Names>
    requires not_empty<Names...>
  auto add(Names&&... names) -> argument<T>& {
    const auto views = {trim(names)...};
    arguments_.push_back(std::make_unique<argument<T>>(views));
    return *static_cast<argument<T>*>(arguments_.back().get());
  }

  void parse(int argc, const char* const* argv) {
    auto index = 1;
    auto positional_index = 0;
    auto positional_force = false;
    while (index < argc) {
      const auto data = trim(argv[index++]);
      if (data == "--" && !positional_force) {
        positional_force = true;
        continue;
      }

      const auto pair = split(data, '=');
      const auto argument = find(pair.front());
      if (argument && !argument->positional() && !positional_force) {
        if (pair.size() == 2) {
          argument->parse(pair.back());
        } else if (index < argc && !argument->boolean()) {
          argument->parse(argv[index++]);
        } else {
          argument->parse({});
        }
      } else {
        if (const auto argument = find(positional_index++)) {
          argument->parse(data);
        } else {
          throw std::runtime_error(fmt::format("unmatched positional argument: {}", data));
        }
      }
    }
    validate();
  }

  template<argument_type T>
  auto get(std::string_view name) const -> T {
    if (const auto argument = find(name)) {
      if (argument->value.has_value()) {
        return std::any_cast<T>(argument->value);
      } else if (argument->default_value.has_value()) {
        return std::any_cast<T>(argument->default_value);
      } else if constexpr (is_specialization_v<T, std::optional>) {
        return std::nullopt;
      }
      throw std::runtime_error(fmt::format("no argument data: {}", name));
    }
    throw std::runtime_error(fmt::format("unknown argument:", name));
  }

  auto help() const -> std::string {
    std::vector<std::string> usage;
    if (!program_.empty()) {
      usage.emplace_back(program_);
    }

    std::size_t widest = 0;
    std::vector<std::tuple<std::string, std::string, bool>> lines;
    for (const auto& argument : arguments_) {
      const auto names = fmt::to_string(fmt::join(argument->names, ", "));
      lines.emplace_back(names, argument->description(), argument->positional());
      usage.emplace_back(argument->usage());
      widest = std::max(widest, names.size());
    }

    std::string keyword;
    std::string positional;
    for (const auto& [names, description, is_positional] : lines) {
      auto& group = is_positional ? positional : keyword;
      group.append(fmt::format("\n  {:<{}}{}", names, widest + 4, description));
    }

    auto help(fmt::format("usage:\n  {}", fmt::join(usage, " ")));
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
        throw std::runtime_error(fmt::format("missing required argument: {}", argument->names.front()));
      }
    }
  }

  std::string_view program_;
  std::vector<std::unique_ptr<basic_argument>> arguments_;
};

}  // namespace sh
