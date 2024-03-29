#pragma once

#include <memory>
#include <vector>
#include <sh/concepts.h>
#include <sh/error.h>
#include <sh/filesystem.h>
#include <sh/parse.h>
#include <sh/ranges.h>

namespace sh {

namespace {

template<formattable T>
auto repr(const T& value) -> std::string {
  if constexpr (std::convertible_to<T, std::string_view>) {
    return fmt::format(R"("{}")", std::string_view{value});
  } else if constexpr (std::same_as<T, std::filesystem::path>) {
    return fmt::format(R"("{}")", value);
  } else {
    return fmt::to_string(value);
  }
}

inline auto split(std::string_view view, char delimiter) -> std::vector<std::string_view> {
  const auto pos = view.find_first_of(delimiter);
  if (pos == std::string_view::npos) {
    return {view};
  } else {
    return {view.substr(0, pos), view.substr(pos + 1)};
  }
}

inline auto trim(std::string_view view) -> std::string_view {
  const auto pos = view.find_first_not_of(' ');
  if (std::string_view::npos == pos) {
    return view;
  }
  const auto end = view.find_last_not_of(' ');
  return view.substr(pos, end - pos + 1);
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

class clap;

class basic_argument {
public:
  friend class clap;

  explicit basic_argument(const std::vector<std::string_view>& names)
    : names_(names) {}

protected:
  virtual auto boolean() const -> bool = 0;
  virtual auto optional() const -> bool = 0;
  virtual void validate() const = 0;
  virtual void parse(std::optional<std::string_view>) = 0;

  std::vector<std::string_view> names_;
  std::string_view what_ = "value";
  std::string_view desc_;
  std::string default_repr_;

private:
  auto positional() const -> bool {
    for (const auto& name : names_) {
      if (name.starts_with('-')) {
        return false;
      }
    }
    return true;
  }

  auto names() const -> std::string {
    return fmt::to_string(fmt::join(names_, ", "));
  }

  auto usage() const -> std::string {
    std::string usage(names_.front());
    if (positional()) {
      usage = fmt::format("<{}>", usage);
    } else if (!boolean()) {
      usage = fmt::format("{} <{}>", usage, what_);
    }
    if (optional()) {
      usage = fmt::format("[{}]", usage);
    }
    return usage;
  }

  auto description() const -> std::string {
    std::vector<std::string> desc;
    if (!desc_.empty()) {
      desc.emplace_back(desc_);
    }
    if (!default_repr_.empty()) {
      desc.emplace_back(fmt::format("[default: {}]", default_repr_));
    }
    return fmt::to_string(fmt::join(desc, " "));
  }
};

class what : public std::string_view {};
class desc : public std::string_view {};

template<argument_type T>
class argument final : public basic_argument {
public:
  friend class clap;

  using value_type = value_type_t<T>;
  using event = std::function<void(const value_type&)>;

  explicit argument(const std::vector<std::string_view>& names)
    : basic_argument(names) {
    *this << [this](const value_type& value) {
      if (pointer_) *pointer_ = value;
    };
    *this << [this](const value_type& value) {
      value_ = value;
    };
  }

  auto operator<<(T* pointer) -> argument& {
    pointer_ = pointer;
    sync();
    return *this;
  }

  auto operator<<(what what) -> argument& {
    what_ = what;
    return *this;
  }

  auto operator<<(desc desc) -> argument& {
    desc_ = desc;
    return *this;
  }

  auto operator<<(const value_type& value) -> argument& {
    default_ = T{value};
    default_repr_ = repr(value);
    sync();
    return *this;
  }

  auto operator<<(const event& event) -> argument& {
    events_.emplace_back(event);
    return *this;
  }

protected:
  auto boolean() const -> bool final {
    return std::same_as<value_type, bool>;
  }

  auto optional() const -> bool final {
    return specialization<T, std::optional> || default_;
  }

  void validate() const final {
    if (!(optional() || value_)) {
      throw error("missing required argument: {}", names_.front());
    }
  }

  void parse(std::optional<std::string_view> data) final {
    auto broadcast = [this](const value_type& value) {
      for (const auto& event : events_) {
        event(value);
      }
    };

    if (data) {
      const auto& view = trim(*data);
      if (const auto value = sh::parse<value_type>(view)) {
        broadcast(*value);
      } else {
        throw error("invalid argument data: {}", view);
      }
    } else {
      if constexpr (std::same_as<value_type, bool>) {
        broadcast(true);
      } else {
        throw error("missing argument data: {}", names_.front());
      }
    }
  }

private:
  T value() const {
    if (value_) {
      return *value_;
    } else if (default_) {
      return *default_;
    } else if constexpr (specialization<T, std::optional>) {
      return std::nullopt;
    }
    throw error("empty argument: {}", names_.front());
  }

  void sync() {
    if (pointer_ && default_) {
      *pointer_ = *default_;
    }
  }

  T* pointer_ = nullptr;
  std::optional<T> value_;
  std::optional<T> default_;
  std::vector<event> events_;
};

class clap {
public:
  explicit clap(std::string_view program)
    : program_(program) {}

  template<argument_type T, std::convertible_to<std::string_view>... Names>
    requires not_empty<Names...>
  auto add(Names&&... names) -> argument<T>& {
    const auto views = {trim(names)...};
    arguments_.emplace_back(std::make_unique<argument<T>>(views));
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
    auto pos_index = 0;
    auto pos_force = false;
    while (index < argc) {
      const auto& data = trim(argv[index++]);
      if (data == "--" && !pos_force) {
        pos_force = true;
        continue;
      }

      const auto kvp = split(data, '=');
      const auto argument = find(kvp.front());
      if (argument && !argument->positional() && !pos_force) {
        if (kvp.size() == 2) {
          argument->parse(kvp.back());
        } else if (index < argc && !argument->boolean()) {
          argument->parse(argv[index++]);
        } else {
          argument->parse(std::nullopt);
        }
      } else {
        if (const auto argument = find(pos_index++)) {
          argument->parse(data);
        } else {
          unmatched.emplace_back(data);
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
    if (const auto argument = find(name)) {
      return static_cast<sh::argument<T>*>(argument)->value();
    }
    throw error("unknown argument:", name);
  }

  auto help() const -> std::string {
    std::size_t padding = 0;
    for (const auto& argument : arguments_) {
      padding = std::max(padding, argument->names().size());
    }

    std::vector<std::string> usage;
    if (!program_.empty()) {
      usage.emplace_back(program_);
    }

    struct group {
      std::string_view caption;
      std::string content;
    };

    group groups[2] = {{"keyword arguments"}, {"positional arguments"}};
    for (const auto& argument : arguments_) {
      auto& group = groups[argument->positional()];
      group.content.append(fmt::format("\n  {:<{}}{}", argument->names(), padding + 4, argument->description()));
      usage.emplace_back(argument->usage());
    }

    auto help = fmt::format("usage:\n  {}", fmt::join(usage, " "));
    for (const auto& [caption, content] : groups) {
      if (!content.empty()) {
        help.append(fmt::format("\n\n{}:", caption));
        help.append(content);
      }
    }
    return help;
  }

  std::vector<std::string> unmatched;

private:
  auto find(std::string_view name) const -> basic_argument* {
    name = trim(name);
    for (const auto& argument : arguments_) {
      if (contains(argument->names_, name)) {
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
