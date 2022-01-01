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

template<typename T>
concept argument_type = parsable<value_type_t<T>> && formattable<value_type_t<T>>;

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

  auto help() const -> std::string {
    std::string string(names.front());
    if (positional()) {
      string = fmt::format("<{}>", string);
    } else if (!boolean()) {
      string = fmt::format("{} <{}>", string, name);
    }
    if (!required()) {
      string = fmt::format("[{}]", string);
    }
    return string;
  }

  auto description() const -> std::string {
    std::vector<std::string> strings;
    if (!desc.empty()) {
      strings.emplace_back(desc);
    }
    if (default_value_repr) {
      strings.emplace_back(fmt::format("(default: {})", *default_value_repr));
    }
    return fmt::to_string(fmt::join(strings, " "));
  }

  virtual auto boolean() const -> bool = 0;
  virtual auto required() const -> bool = 0;
  virtual void validate() const = 0;
  virtual void parse(std::string_view) = 0;

  std::vector<std::string_view> names;
  std::string_view name = "value";
  std::string_view desc;
  std::optional<std::string> default_value_repr;
};

template<argument_type T>
class argument final : public basic_argument {
public:
  using value_type = value_type_t<T>;

  explicit argument(const std::vector<std::string_view>& names)
    : basic_argument(names) {
    events.emplace_back([this](const value_type& value) {
      if (pointer) *pointer = value;
    });
    events.emplace_back([this](const value_type& value) {
      this->value = value;
    });
  }

  auto operator<<(T* pointer) -> argument& {
    this->pointer = pointer;
    this->sync();
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
    this->default_value = T{value};
    this->default_value_repr = repr(value);
    this->sync();
    return *this;
  }

  auto operator<<(const std::function<void(const value_type&)>& event) -> argument& {
    events.emplace_back(event);
    return *this;
  }

  auto boolean() const -> bool final {
    return std::same_as<value_type, bool>;
  }

  auto required() const -> bool final {
    return !(is_specialization_v<T, std::optional> || default_value);
  }

  void validate() const final {
    if (required() && !value) {
      throw error("missing required argument: {}", names.front());
    }
  }

  void parse(std::string_view data) final {
    auto broadcast = [this](const value_type& value) {
      for (const auto& event : events) {
        event(value);
      }
    };

    if (data == std::string_view()) {
      if constexpr (std::same_as<value_type, bool>) {
        broadcast(true);
      } else {
        throw error("expected data for argument: {}", names.front());
      }
    } else {
      data = trim(data);
      if (const auto value = sh::parse<value_type>(data)) {
        broadcast(*value);
      } else {
        throw error("cannot parse argument data: {}", data);
      }
    }
  }

  T* pointer = nullptr;
  std::optional<T> value;
  std::optional<T> default_value;
  std::vector<std::function<void(const value_type&)>> events;

private:
  void sync() {
    if (pointer && default_value) {
      *pointer = *default_value;
    }
  }
};

class clap {
public:
  explicit clap(std::string_view program)
    : program_(program) {}

  template<argument_type T, std::convertible_to<std::string_view>... Names>
    requires not_empty<Names...>
  auto add(Names&&... names) -> argument<T>& {
    const auto views = {trim(names)...};
    arguments_.push_back(std::make_unique<argument<T>>(views));
    return *static_cast<argument<T>*>(arguments_.back().get());
  }

  void add_help() {
    add<std::optional<bool>>("-h", "--help") << desc("print help") << [this](bool value) {
      if (value) {
        fmt::print("{}\n", help());
        std::exit(0);
      }
    };
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

      const auto kvp = split(data, '=');
      const auto argument = find(kvp.front());
      if (argument && !argument->positional() && !positional_force) {
        if (kvp.size() == 2) {
          argument->parse(kvp.back());
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

    for (const auto& argument : arguments_) {
      argument->validate();
    }
  }

  void try_parse(int argc, const char* const* argv) {
    try {
      parse(argc, argv);
    } catch (const error& error) {
      fmt::print("{}\n\n{}\n", error, help());
      std::exit(1);
    }
  }

  template<argument_type T>
  auto get(std::string_view name) const -> T {
    if (const auto argument = static_cast<sh::argument<T>*>(find(name))) {
      if (argument->value) {
        return *argument->value;
      } else if (argument->default_value) {
        return *argument->default_value;
      } else if constexpr (is_specialization_v<T, std::optional>) {
        return std::nullopt;
      }
      throw error("no argument data: {}", name);
    }
    throw error("unknown argument:", name);
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
      usage.emplace_back(argument->help());
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

  std::string_view program_;
  std::vector<std::unique_ptr<basic_argument>> arguments_;
};

}  // namespace sh
