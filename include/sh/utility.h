#pragma once

namespace sh {

template <typename... Ts>
constexpr void unused(const Ts&...) {}

}  // namespace sh
