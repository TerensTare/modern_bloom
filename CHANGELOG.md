# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## 2024-02-17

### Added

- Support for specifying the approximate number of elements and the desired false positive probability in the constructor of `tnt::dynamic_bloom`. The parameter is optional and defaults to 1% false positives rate.


### Changed

- `tnt::dynamic_bloom` now steps multiple times through the hash functions to improve the distribution of the bits.
- Deprecated `tnt::dynamic_bloom::clear_and_resize` method. Use the constructor instead, as follows:
- Deprecated `contains` method on both filters. Use `matches` instead, as it is more aptly named. Credits to @Luke from the `#include` discord server for the suggestion. Samples/tests have been updated accordingly.

```cpp
tnt::dynamic_bloom<my_type> my_filter(1'000, 0.01);

// use the bloom filter

// now clear and resize the bloom filter
my_filter = tnt::dynamic_bloom<my_type>(2'000, 0.03);
```