
#include "test.hpp"
#include <static_bloom.hpp>

using namespace tnt::test;
using namespace tnt::test::literals;

int main()
{
    "general_test"_test = []
    {
        // const char * is fine here as the value is not stored
        tnt::static_bloom<char const *, 100> bloom;

        bloom.insert("Hello");

        ensure(bloom.contains("Hello")) << "- Bloom filter should contain 'Hello'";
        ensure(!bloom.contains("World")) << "- Bloom filter should not contain 'World'";

        bloom.clear();

        ensure(!bloom.contains("Hello")) << "- Bloom filter should not contain 'Hello'";

        bloom.insert("World");

        ensure(bloom.contains("World")) << "- Bloom filter should contain 'World'";
    };

    return 0;
}