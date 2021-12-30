#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <sh/concepts.h>
#include <sh/error.h>
#include <sh/filesystem.h>
#include <sh/fmt.h>
#include <sh/parse.h>
#include <sh/ranges.h>

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

inline auto split(std::string_view str, char delimiter) -> std::vector<std::string_view> {
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

}  // namespace

class name : public std::string_view {};
class desc : public std::string_view {};

class basic_argument {
public:
  explicit basic_argument(const std::vector<std::string_view>& names)
    : names(names) {}

  auto positional() const -> bool {
    return !names.front().starts_with('-');
  }

  auto syntax() const -> std::string {
    std::string syntax(names.front());
    if (positional()) {
      syntax = fmt::format("<{}>", syntax);
    } else if (!boolean()) {
      syntax = fmt::format("{} <{}>", syntax, name);
    }
    if (!required()) {
      syntax = fmt::format("[{}]", syntax);
    }
    return syntax;
  }

  auto description() const -> std::string {
    std::vector<std::string> strings;
    if (!desc.empty()) {
      strings.emplace_back(desc);
    }
    if (default_value_repr) {
      strings.emplace_back(fmt::format("[default: {}]", *default_value_repr));
    }
    return fmt::to_string(fmt::join(strings, " "));
  }

  virtual auto boolean() const -> bool = 0;
  virtual auto required() const -> bool = 0;
  virtual auto satisfied() const -> bool = 0;
  virtual void parse(std::string_view) = 0;

  std::vector<std::string_view> names;
  std::string_view name = "value";
  std::string_view desc;
  std::optional<std::string> default_value_repr;
};

template<typename T>
class argument final : public basic_argument {
public:
  using value_type = value_type_t<T>;

  using basic_argument::basic_argument;

  auto operator<<(T* pointer) -> argument& {
    this->pointer = pointer;
    if (pointer && default_value) {
      *pointer = *default_value;
    }
    return *this;
  }

  auto operator<<(sh::name name) -> argument& {
    this->name = name;
    return *this;
  }

  auto operator<<(sh::desc desc) -> argument& {
    this->desc = desc;
    return *this;
  }

  auto operator<<(const value_type& value) -> argument& {
    if (pointer) {
      *pointer = value;
    }
    default_value = T{value};
    default_value_repr = repr(value);
    return *this;
  }

  auto operator<<(const std::function<void(const value_type&)>& event) -> argument& {
    this->event = event;
    return *this;
  }

  auto boolean() const -> bool final {
    return std::same_as<value_type, bool>;
  }

  auto required() const -> bool final {
    return !(is_specialization_v<T, std::optional> || default_value.has_value());
  }

  auto satisfied() const -> bool final {
    // simpliy/remove?
    return is_specialization_v<T, std::optional> || value.has_value() || default_value.has_value();
  }

  void parse(std::string_view data) final {
    value_type value;
    if (data == std::string_view()) {
      if constexpr (std::same_as<value_type, bool>) {
        value = true;
      } else {
        throw error("expected data for argument: {}", names.front());
      }
    } else {
      data = trim(data);
      if (const auto parsed = sh::parse<value_type>(data)) {
        value = *parsed;
      } else {
        throw error("cannot parse argument data: {}", data);
      }
    }

    if (event) {
      event(value);
    }
    if (pointer) {
      *pointer = value;
    }
    this->value = T{value};
  }

  T* pointer = nullptr;
  std::optional<T> value;
  std::optional<T> default_value;
  std::function<void(const value_type&)> event;
};

class clap {
public:
  explicit clap(std::string_view program)
    : program(program) {}

  template<typename T, std::convertible_to<std::string_view>... Names>
    requires not_empty<Names...>
  auto add(Names&&... names) -> argument<T>& {
    const auto views = {trim(names)...};
    arguments.push_back(std::make_unique<argument<T>>(views));
    return *static_cast<argument<T>*>(arguments.back().get());
  }

  void add_help() {
    add<std::optional<bool>>("-h", "--help") << [this](bool value) {
      if (value) {
        fmt::print("{}\n", help());
        std::exit(0);
      }
    };
  }

  template<typename T>
  auto get(std::string_view name) const -> T {
    if (const auto argument = find(name)) {
      sh::argument<T>* arg = static_cast<sh::argument<T>*>(argument);
      if (arg->value) {
        return *arg->value;
      } else if (arg->default_value) {
        return *arg->default_value;
      } else if constexpr (is_specialization_v<T, std::optional>) {
        return std::nullopt;
      }
      throw error("no argument data: {}", name);
    }
    throw error("unknown argument:", name);
  }

  auto help() const -> std::string {
    std::vector<std::string> usage;
    if (!program.empty()) {
      usage.emplace_back(program);
    }

    std::size_t widest = 0;
    std::vector<std::tuple<std::string, std::string, bool>> lines;
    for (const auto& argument : arguments) {
      const auto names = fmt::to_string(fmt::join(argument->names, ", "));
      lines.emplace_back(names, argument->description(), argument->positional());
      usage.emplace_back(argument->syntax());
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
          throw error("unmatched positional argument: {}", data);
        }
      }
    }
    validate();
  }

  void try_parse(int argc, const char* const* argv) {
    try {
      parse(argc, argv);
    } catch (const error& error) {
      fmt::print("{}\n\n{}\n", error, help());
      std::exit(1);
    }
  }

private:
  auto find(std::string_view name) const -> basic_argument* {
    name = trim(name);
    for (const auto& argument : arguments) {
      if (contains(argument->names, name)) {
        return argument.get();
      }
    }
    return nullptr;
  }

  auto find(std::size_t position) const -> basic_argument* {
    for (const auto& argument : arguments) {
      if (argument->positional() && position-- == 0) {
        return argument.get();
      }
    }
    return nullptr;
  }

  void validate() {
    for (const auto& argument : arguments) {
      if (!argument->satisfied()) {
        throw error("missing required argument: {}", argument->names.front());
      }
    }
  }

  std::string_view program;
  std::vector<std::unique_ptr<basic_argument>> arguments;
};

}  // namespace sh
