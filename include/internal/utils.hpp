
#pragma once

#include <type_traits>

/// @cond NO_DOXYGEN

namespace tnt::utils
{
    template <typename T, typename = void>
    struct is_transparent final
    {
        inline static constexpr bool value = false;
    };

    template <typename T>
    struct is_transparent<T, std::void_t<typename T::is_transparent>> final
    {
        inline static constexpr bool value = true;
    };

    template <typename T, typename Hash, typename = void>
    struct is_hashable_with
    {
        inline static constexpr bool value = false;
    };

    template <typename T, typename Hash>
    struct is_hashable_with<T, Hash, std::void_t<decltype(std::declval<Hash &>()(std::declval<T &>()))>>
    {
        static_assert(
            std::is_same_v<
                std::invoke_result_t<Hash, T>,
                std::size_t>,
            "Hash function must return a size_t!");

        inline static constexpr bool value = true;
    };

    constexpr std::size_t next_power_of_two(std::size_t n, std::size_t i = sizeof(std::size_t)) noexcept
    {
        return (n & (1 << i))
                   ? ((n & (n - 1)) ? 1 << (i + 1) : n)
                   : next_power_of_two(n, i - 1);
    }
}

/// @endcond