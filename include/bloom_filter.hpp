
#pragma once

#include <cmath>
#include <memory>
#include <memory_resource>

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

    template <typename Alloc>
    struct deduce_allocator final
    {
        using type = std::allocator_traits<Alloc>::template rebind_alloc<std::uint64_t>;
    };
}

/// @endcond

namespace tnt
{
    /// @brief Utility function that calculates the number of bits to be used in the bloom filter. Useful for pre-allocating the storage if desired.
    /// @param n The number of elements to be inserted into the filter.
    /// @param eps The desired false positive rate.
    /// @return The number of bits to be used in the bloom filter.
    constexpr std::size_t bloom_filter_bits(std::size_t n, float eps) noexcept
    {
        // this only works for eps > 1.0, so it needs a - sign afterwards
        auto const ln_eps = [](float x, int n = 10)
        {
            if (x == 1.0f)
                return 0.0f;

            auto const xi = x - 1.0f;

            auto res = 0.0f;
            auto term = xi;

            for (int i = 0; i < n; ++i)
            {
                res += term / (i + 1);
                term *= -xi;
            }

            return res;
        }(eps);

        auto constexpr ln_2 = 0.6931471805599453f;
        auto constexpr ln_2p2 = ln_2 * ln_2;

        return static_cast<std::size_t>(n * -ln_eps / ln_2p2);
    }

    /// @brief A bloom filter is a space-efficient probabilistic data structure that is used to test whether an element might be a member of a set.
    /// @tparam T The type of the elements to be inserted into the bloom filter.
    /// @tparam Hash The hash function to be used for hashing the elements. Defaults to std::hash.
    /// @tparam Alloc The allocator to be used for memory management. Defaults to std::allocator.
    template <
        typename T,
        typename Hash = std::hash<T>,
        typename Alloc = std::allocator<std::uint64_t>>
    class bloom_filter final
        : private Hash,
          private utils::deduce_allocator<Alloc>::type
    {
        static_assert(
            utils::is_hashable_with<T, Hash>::value,
            "Hash must be a callable object that returns a size_t!");

        using allocator_type = typename utils::deduce_allocator<Alloc>::type;

    public:
        /// @brief Construct a new instance of the bloom filter, given the number of elements and the desired false positive rate.
        /// @param n The number of elements to be inserted into the filter.
        /// @param eps The desired false positive rate. Defaults to 0.01 (1% false positives).
        /// @param hash The hash function to be used for hashing the elements.
        /// @param alloc The allocator to be used for memory management.
        CONST_ALLOC bloom_filter(
            std::size_t n,
            float eps = 0.01f,
            Hash const &hash = Hash{},
            allocator_type const &alloc = allocator_type{})
            : Hash(hash),
              allocator_type(alloc)
        {
            auto const nlog_eps = -std::log(eps);
            auto const log_2 = 0.6931471805599453f;

            m = static_cast<std::size_t>(n * nlog_eps / (log_2 * log_2));
            k = static_cast<std::size_t>(nlog_eps / log_2);

            auto const len = (m >> 6) + ((m & 63) != 0);
            bits = allocator_type::allocate(len);
            std::fill_n(bits, len, 0);
        }

        /// @brief The copy constructor.
        CONST_ALLOC bloom_filter(bloom_filter const &rhs) noexcept
            : m{rhs.m}, k{rhs.k}
        {
            auto const len = (m >> 6) + ((m & 63) != 0);
            bits = allocator_type::allocate(len);
            std::copy_n(rhs.bits, len, bits);
        }

        /// @brief The copy assignment operator.
        CONST_ALLOC bloom_filter &operator=(bloom_filter const &rhs) noexcept
        {
            bloom_filter tmp{rhs};
            swap(*this, tmp);

            return *this;
        }

        /// @brief The move constructor.
        CONST_SWAP bloom_filter(bloom_filter &&rhs) noexcept
        {
            swap(*this, rhs);
        }

        /// @brief The move assignment operator.
        CONST_SWAP bloom_filter &operator=(bloom_filter &&rhs) noexcept
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief The destructor.
        CONST_ALLOC ~bloom_filter() noexcept
        {
            auto const len = (m >> 6) + ((m & 63) != 0);
            allocator_type::deallocate(bits, len);
        }

        /// @brief Add the value into the filter.
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

        /// @brief Remove all elements from the filter.
        constexpr void clear() noexcept
        {
            auto const len = (m >> 6) + ((m & 63) != 0);
            std::fill_n(bits, len, 0);
        }

        /// @brief Swap the contents of two filters together.
        friend CONST_SWAP void swap(bloom_filter &lhs, bloom_filter &rhs) noexcept
        {
            std::swap(lhs.m, rhs.m);
            std::swap(lhs.k, rhs.k);
            std::swap(lhs.bits, rhs.bits);
            std::swap(static_cast<allocator_type &>(lhs), static_cast<allocator_type &>(rhs));
        }

    private:
        std::size_t m : sizeof(std::size_t) * 8 - 8;
        std::size_t k : 8;
        std::uint64_t *bits;
    };

    namespace pmr
    {
        /// @brief Specialization of bloom_filter using a polymorphic allocator.
        /// @tparam T The type of the elements to be inserted into the bloom filter.
        /// @tparam Hash The hash function to be used for hashing the elements. Defaults to std::hash.
        template <typename T, typename Hash = std::hash<T>>
        using bloom_filter = tnt::bloom_filter<
            T, Hash,
            std::pmr::polymorphic_allocator<std::uint64_t>>;
    }
}