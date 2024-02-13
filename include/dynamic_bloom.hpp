
#pragma once

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
        /// @param n The maximum number of elements the filter is expected to represent.
        /// @param hash An instance of the hash function to use internally.
        CONST_ALLOC dynamic_bloom(std::size_t n, Hash &&hash = Hash{})
            : bits(new std::uint64_t[(n >> 6) + (n & 63)]{}), n{n}, Hash(std::move(hash)) {}

        dynamic_bloom(dynamic_bloom const &) = default;
        dynamic_bloom &operator=(dynamic_bloom const &) = default;

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
            auto const h = static_cast<Hash const &>(*this)(value) % n;
            bits[h >> 6] |= 1ULL << (h & 63);
        }

        /// @brief Check whether the given value *might* be present in the bloom filter. Also supports transparent hash objects.
        /// @param value The value to check.
        template <typename U>
        constexpr bool contains(U &&value) const noexcept
        {
            using raw_u = std::decay_t<U>;

            static_assert(
                (std::is_same_v<raw_u, T> || utils::is_transparent<Hash>::value) &&
                    utils::is_hashable_with<U, Hash>::value,
                "This type is not hashable with the given hash function!");

            auto const h = static_cast<Hash const &>(*this)(static_cast<U &&>(value)) % n;
            return (bits[h >> 6] & (1ULL << (h & 63))) != 0;
        }

        /// @brief Remove all possible values stored by the filter.
        constexpr void clear() noexcept
        {
            auto const len = (n >> 6) + (n & 63);
            for (std::size_t i{}; i < len; ++i)
                bits[i] = 0;
        }

        /// @brief Remove all possible values stored by the filter and then resize to store at most `n` elements.
        /// @param n The new maximum number of elements.
        CONST_ALLOC void clear_and_resize(std::size_t n) noexcept
        {
            auto const new_len = (n >> 6) + (n & 63);
            auto const old_len = (this->n >> 6) + (this->n & 63);

            // don't reallocate unless you have to.
            if (new_len <= old_len)
                clear();
            else
            {
                delete[] bits;
                bits = new std::uint64_t[new_len]{};
            }

            this->n = n;
        }

        /// @brief Swap two bloom filters' data.
        friend CONST_SWAP void swap(dynamic_bloom &lhs, dynamic_bloom &rhs) noexcept
        {
            std::swap(lhs.bits, rhs.bits);
            std::swap(lhs.n, rhs.n);
        }

    private:
        std::uint64_t *bits = nullptr;
        std::size_t n;
    };
}

#undef CONST_ALLOC