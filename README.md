
# Bloom filters for modern C++

This repo contains a simple implementation of a bloom filter in C++17.

## Introduction

A bloom filter is a probabilistic data structure that is used to test whether an element is a member of a set. False positives are possible, but false negatives are not. Elements can be added to the set, but not removed.

## Usage

```cpp
#include <dynamic_bloom.hpp>

struct my_article final
{
    std::string title;
    std::string author;
    std::string content;
};

struct article_hash final
{
    inline std::size_t operator()(my_article const& article) const noexcept
    {
        return std::hash<std::string>{}(article.title);
    }
};

int main()
{
    // Assume that the average user can read 1000 articles on average.
    tnt::dynamic_bloom<my_article, article_hash> read_articles(1'000);

    // Our user reads an article.
    my_article article1{"The C++ Programming Language", "Bjarne Stroustrup", "The C++ Programming Language is a computer programming book first published in October 1985."};

    read_articles.insert(article1);

    std::vector<my_article> suggestions;

    for (auto const &article : <list-of-all-articles>)
    {
        if (!read_articles.contains(article))
        {
            // Add non-read articles to the suggestions list.
            suggestions.push_back(article);
        }
    }
}
```


## Integration

This is a header-only library. You can use it by simply copying the files in the `include` directory to your project. Then include one of the header files in your source code as following.

```cpp
// for fixed-size bloom filter
#include <static_bloom.hpp> // tnt::static_bloom

// for dynamically-sized bloom filter
#include <dynamic_bloom.hpp> // tnt::dynamic_bloom
```


## Requirements

The library is written in C++17, so you need a compiler that supports at least C++17. However, the tests are written in C++20, so you need a compiler that supports at least C++20 to run the tests. Furthermore, you need CMake 3.14 or newer to build the tests.


## Tests

To run the tests, you need to have CMake 3.14 or newer installed on your system. Then, you can run the following commands.

```bash
mkdir build
cd build
cmake ..
cmake --build . --target run_tests
```

## License

Code is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.