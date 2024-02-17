
#include "test.hpp"
#include <dynamic_bloom.hpp>

using namespace tnt::test;
using namespace tnt::test::literals;

int main()
{
    "general_test"_test = []
    {
        // const char * is fine here as the value is not stored
        tnt::dynamic_bloom<char const *> bloom(100, 0.01f);

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