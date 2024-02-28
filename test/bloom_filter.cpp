
#include "test.hpp"
#include <bloom_filter.hpp>

using namespace tnt::test;
using namespace tnt::test::literals;

int main()
{
    "filter_on_heap"_test = []
    {
        // const char * is fine here as the value is not stored
        tnt::bloom_filter<char const *> bloom{100, 0.01f};

        bloom.insert("Hello");

        ensure(bloom.matches("Hello")) << "- Bloom filter should contain 'Hello'";
        ensure(!bloom.matches("World")) << "- Bloom filter should not contain 'World'";

        bloom.clear();

        ensure(!bloom.matches("Hello")) << "- Bloom filter should not contain 'Hello'";

        bloom.insert("World");

        ensure(bloom.matches("World")) << "- Bloom filter should contain 'World'";
    };

    "filter_on_stack"_test = []
    {
        auto constexpr bits = tnt::bloom_filter_bits(100, 0.01f);
        unsigned char buffer[bits];
        std::pmr::monotonic_buffer_resource res{buffer, sizeof(buffer)};

        // const char * is fine here as the value is not stored
        tnt::pmr::bloom_filter<char const *> bloom{100, 0.01f, {}, &res};

        bloom.insert("Hello");

        ensure(bloom.matches("Hello")) << "- Bloom filter should contain 'Hello'";
        ensure(!bloom.matches("World")) << "- Bloom filter should not contain 'World'";

        bloom.clear();

        ensure(!bloom.matches("Hello")) << "- Bloom filter should not contain 'Hello'";

        bloom.insert("World");

        ensure(bloom.matches("World")) << "- Bloom filter should contain 'World'";
    };

    return 0;
}