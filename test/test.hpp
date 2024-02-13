
#pragma once

#include <cstdio>
#include <cinttypes>
#include <source_location>
#include <string_view>

namespace tnt::test
{
    struct counter final
    {
        inline static std::uint32_t ensures_passed{};
        inline static std::uint32_t ensures_failed{};

        inline static std::uint32_t tests_passed{};
        inline static std::uint32_t tests_failed{};

        inline static bool failed_ensures_here{};
    };

    struct ensure_type final
    {
        ensure_type(ensure_type const &) = delete;
        ensure_type &operator=(ensure_type const &) = delete;

        inline ensure_type(ensure_type &&other) noexcept
        {
            std::swap(sl, other.sl);
            std::swap(condition, other.condition);
            std::swap(message, other.message);

            other.message = nullptr;
        }

        inline ensure_type &operator=(ensure_type &&other) noexcept
        {
            std::swap(sl, other.sl);
            std::swap(condition, other.condition);
            std::swap(message, other.message);

            other.message = nullptr;

            return *this;
        }

        inline ~ensure_type() noexcept
        {
            if (message)
            {
                auto result = condition ? "[v] PASSED" : "[x] FAILED";
                condition ? ++counter::ensures_passed : ++counter::ensures_failed;

                auto file_name = std::string_view{sl.file_name()};

                if (auto cursor = file_name.find_last_of('/'); cursor != std::string::npos)
                    file_name = file_name.substr(cursor + 1);
                else if (cursor = file_name.find_last_of('\\'); cursor != std::string::npos)
                    file_name = file_name.substr(cursor + 1);

                std::printf(
                    "%s:%" PRIuLEAST32 " %s %s\n",
                    file_name.data(),
                    sl.line(),
                    result,
                    condition ? "" : message //
                );
            }
        }

        friend ensure_type operator<<(ensure_type et, char const *msg) noexcept
        {
            et.message = msg;
            return et;
        }

    private:
        inline ensure_type(std::source_location sl, bool condition) noexcept
            : sl{sl}, condition{condition} {}

        std::source_location sl;
        bool condition = false;
        char const *message = "";

        friend ensure_type ensure(bool condition, std::source_location sl) noexcept;
    };

    inline ensure_type ensure(
        bool condition,
        std::source_location sl = std::source_location::current()) noexcept
    {
        return {sl, condition};
    }

    struct test_suite final
    {
        inline explicit test_suite(char const *name) noexcept
            : name{name}
        {
            counter::failed_ensures_here = false;

            std::printf("Running '%s'...\n", name);
        }

        inline ~test_suite() noexcept
        {
            if (counter::failed_ensures_here)
                ++counter::tests_failed;
            else
                ++counter::tests_passed;
        }

        inline void operator=(void (*pfn)()) noexcept
        {
            pfn();
        }

        char const *name;
    };

    namespace literals
    {
        inline test_suite operator""_test(char const *name, std::size_t) noexcept
        {
            return test_suite{name};
        }
    }

    struct general_reporter_type final
    {
        inline ~general_reporter_type() noexcept
        {
            std::printf(
                "\nEnsures: %" PRIu32 " passed, %" PRIu32 " failed\nTests:   %" PRIu32 " passed, %" PRIu32 " failed\n",
                counter::ensures_passed,
                counter::ensures_failed,
                counter::tests_passed,
                counter::tests_failed);
        }
    };

    inline static general_reporter_type general_reporter{};
}