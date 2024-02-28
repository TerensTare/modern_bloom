
#pragma once

#include <cmath>

#if __has_include(<version>)
#include <version>
#endif

#include "internal/utils.hpp"

#if defined(__cpp_constexpr_dynamic_alloc) && (__cpp_constexpr_dynamic_alloc >= 201907L)
#define CONST_ALLOC constexpr
#else
#define CONST_ALLOC inline
#endif

#if defined(__cpp_lib_constexpr_algorithms) && (__cpp_lib_constexpr_algorithms >= 202306L)
#define CONST_SWAP constexpr
#else
#define CONST_SWAP inline
#endif

/// @cond NO_DOXYGEN

namespace tnt::utils
{
    struct size_traits final
    {
        // split in two halves, then multiply by 8 to get the number of bits.
        inline static constexpr std::size_t low_mask = std::size_t(-1) >> (sizeof(std::size_t) / 2 * 8);
        inline static constexpr std::size_t high_mask = std::size_t(-1) & ~low_mask;
        inline static constexpr std::size_t high_shift = sizeof(std::size_t) / 2 * 8;
    };
}

/// @endcond

namespace tnt
{
    /// @brief A bloom filter with maximum size determined at runtime. Can be resized, but stored elements are then erased.
    /// @tparam T The type of the elements represented on the bloom filter.
    /// @tparam Hash The type of the hash function object used. Defaults to `std::hash`.
    template <typename T, typename Hash = std::hash<T>>
    struct dynamic_bloom final : private Hash
    {
        static_assert(
            utils::is_hashable_with<T, Hash>::value,
            "Hash must be a callable object that returns a size_t!");

        /// @brief Construct a new bloom filter from a maximum number of elements and (optionally), a hash function instance.
        /// @param n The expected number of elements the filter is expected to represent.
        /// @param eps The desired false positive rate, between 0 and 1. Defaults to 0.01 (1 %).
        /// @param hash An instance of the hash function to use internally.
        CONST_ALLOC dynamic_bloom(std::size_t n, float eps = 0.01f, Hash const &hash = Hash{})
            : Hash(hash)
        {
            auto const nlog_eps = -std::log(eps);
            auto const log_2 = 0.6931471805599453f;

            m = static_cast<std::size_t>(n * nlog_eps / (log_2 * log_2));
            k = static_cast<std::size_t>(nlog_eps / log_2);

            auto const len = (m >> 6) + ((m & 63) != 0);
            bits = new std::uint64_t[len]{};
        }

        /// @brief Copy constructor.
        CONST_ALLOC dynamic_bloom(dynamic_bloom const &rhs) noexcept
            : m{rhs.m}, k{rhs.k}
        {
            auto const len = (m >> 6) + ((m & 63) != 0);
            bits = new std::uint64_t[len]{};

            for (std::size_t i{}; i < len; ++i)
                bits[i] = rhs.bits[i];
        }

        /// @brief Copy assignment operator.
        CONST_ALLOC dynamic_bloom &operator=(dynamic_bloom const &rhs) noexcept
        {
            dynamic_bloom tmp{rhs};
            swap(*this, tmp);
            return *this;
        }

        /// @brief Move constructor.
        CONST_SWAP dynamic_bloom(dynamic_bloom &&rhs) noexcept
        {
            swap(*this, rhs);
        }

        /// @brief Move assignment operator.
        CONST_SWAP dynamic_bloom &operator=(dynamic_bloom &&rhs) noexcept
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Destructor.
        CONST_ALLOC ~dynamic_bloom() noexcept { delete[] bits; }

        /// @brief Insert the value into the bloom filter.
        /// @param value The value to insert.
        constexpr void insert(T const &value) noexcept
        {
            auto const hash = static_cast<Hash const &>(*this)(value);
            auto const step = hash & utils::size_traits::low_mask;

            auto h = (hash & utils::size_traits::high_mask) >> utils::size_traits::high_shift;

            // strategy based on
            // https://github.com/Claudenw/BloomFilters/wiki/Bloom-Filters----An-overview
            for (std::size_t i{}; i < k; ++i)
            {
                h += i * step;

                auto const index = h % m;
                bits[index >> 6] |= std::size_t{1} << (index & 63);
            }
        }

        /// @brief Check whether the given value *might* be present in the bloom filter.
        /// @param value The value to check.
        /// @note This function is deprecated in favor of `matches`.
        template <typename U>
        [[deprecated("Use `matches` instead")]] constexpr bool contains(U &&value) const noexcept
        {
            return matches(static_cast<U &&>(value));
        }

        /// @brief Check whether the given value *might* be present in the bloom filter. While this function can return false positives, it will never return false negatives.
        /// Ie. `matches(x)` might be wrong, but `!matches(x)` is always correct.
        /// @param value The value to check.
        template <typename U>
        constexpr bool matches(U &&value) const noexcept
        {
            using raw_u = std::decay_t<U>;

            static_assert(
                (std::is_same_v<raw_u, T> || utils::is_transparent<Hash>::value) &&
                    utils::is_hashable_with<U, Hash>::value,
                "This type is not hashable with the given hash function!");

            auto const hash = static_cast<Hash const &>(*this)(value);
            auto const step = hash & utils::size_traits::low_mask;

            auto h = (hash & utils::size_traits::high_mask) >> utils::size_traits::high_shift;

            bool found{false};

            for (std::size_t i{}; i < k; ++i)
            {
                h += i * step;

                auto const index = h % m;
                found |= (bits[index >> 6] & (std::size_t{1} << (index & 63))) != 0;
            }

            return found;
        }

        /// @brief Remove all possible values stored by the filter.
        constexpr void clear() noexcept
        {
            auto const len = (m >> 6) + ((m & 63) != 0);
            for (std::size_t i{}; i < len; ++i)
                bits[i] = 0;
        }

        /// @brief Clear the filter and resize it to a new size.
        /// @param n The new size of the filter.
        /// @param eps The desired false positive rate, between 0 and 1. Defaults to 0.01 (1 %).
        [[deprecated("Use the constructor instead")]] void clear_and_resize(std::size_t n, float eps = 0.01f)
        {
            dynamic_bloom tmp{n, eps, static_cast<Hash const &>(*this)};
            swap(*this, tmp);
        }

        /// @brief Swap two bloom filters' data.
        friend CONST_SWAP void swap(dynamic_bloom &lhs, dynamic_bloom &rhs) noexcept
        {
            std::swap(lhs.bits, rhs.bits);
            std::swap(lhs.m, rhs.m);
            std::swap(lhs.k, rhs.k);
        }

    private:
        std::uint64_t *bits = nullptr;
        std::size_t m : sizeof(std::size_t) * 8 - 8;
        std::size_t k : 8;
    };
}

#undef CONST_ALLOC