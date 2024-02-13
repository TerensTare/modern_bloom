
#pragma once

#include "internal/utils.hpp"

#ifdef __cpp_lib_constexpr_algorithms
#define CONST_SWAP constexpr
#else
#define CONST_SWAP inline
#endif

namespace tnt
{
    /// @brief A bloom filter with maximum size determined at compile time. Cannot be resized.
    /// @tparam T The type of the elements represented on the bloom filter.
    /// @tparam N The maximum number of elements stored.
    /// @tparam Hash The type of the hash function object used. Defaults to `std::hash`.
    template <typename T, std::size_t N, typename Hash = std::hash<T>>
    struct static_bloom final : private Hash
    {
        static_assert(
            utils::is_hashable_with<T, Hash>::value,
            "Hash must be a callable object that returns a size_t!");

        /// @brief The default constructor.
        static_bloom() = default;

        static_bloom(static_bloom const &) = default;
        static_bloom &operator=(static_bloom const &) = default;

        /// @brief The move constructor. 
        CONST_SWAP static_bloom(static_bloom &&rhs) noexcept
        {
            swap(*this, rhs);
        }

        /// @brief The move assignment operator. 
        CONST_SWAP static_bloom &operator=(static_bloom &&rhs) noexcept
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Construct a new instance of the bloom filter, with a specific instance of the hash object.
        /// @param hash The hash object to use.
        explicit constexpr static_bloom(Hash &&hash) : Hash(std::move(hash)) {}

        /// @brief Add the value into the filter.
        /// @param value The value to insert into the filter.
        constexpr void insert(T const &value) noexcept
        {
            auto const h = bucket(value);
            bits[h >> 6] |= 1ULL << (h & 63);
        }

        /// @brief Check whether the filter *might* contain the given element. Also supports transparent hash objects.
        /// @param value The value to check.
        template <typename U>
        constexpr bool contains(U &&value) const noexcept
        {
            static_assert(
                (std::is_same_v<std::decay_t<U>, T> || utils::is_transparent<Hash>::value) &&
                    utils::is_hashable_with<U, Hash>::value,
                "This type is not hashable with the given hash function!");

            auto const h = bucket(static_cast<U &&>(value));
            return (bits[h >> 6] & (1ULL << (h & 63))) != 0;
        }

        /// @brief Remove all elements from the filter.
        constexpr void clear() noexcept
        {
            for (auto &bit : bits)
                bit = 0;
        }

        /// @brief Swap the contents of two filters together. 
        friend CONST_SWAP void swap(static_bloom &lhs, static_bloom &rhs) noexcept
        {
            std::swap(lhs.bits, rhs.bits);
        }

    private:
        template <typename U>
        constexpr std::size_t bucket(U &&value) const noexcept
        {
            if constexpr ((N & (N - 1)) == 0)
                return static_cast<Hash const &>(*this)(static_cast<U &&>(value)) & (N - 1);
            else
                return static_cast<Hash const &>(*this)(static_cast<U &&>(value)) % N;
        }

        std::uint64_t bits[N / 64 + N % 64]{};
    };
}

#undef CONST_SWAP