#ifndef OXIDE_ASSERT_HPP
#define OXIDE_ASSERT_HPP
/*
 *  Copyright (C) 2025-2026 Igal Alkon and ALKONTEK
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#include "output.hpp"

#include <source_location>
#include <string_view>

namespace oxide {
    namespace detail {
        [[noreturn]] inline void fail_check(
            const std::string_view kind,
            const std::string_view expr,
            const std::string_view msg,
            const std::source_location& loc
        ) {
            if (msg.empty()) {
                panic_fmt(
                    "{} failed at {}:{} in {}: {}",
                    kind,
                    loc.file_name(),
                    loc.line(),
                    loc.function_name(),
                    expr
                );
            }

            panic_fmt(
                "{} failed at {}:{} in {}: {} — {}",
                kind,
                loc.file_name(),
                loc.line(),
                loc.function_name(),
                expr,
                msg
            );
        }

        template <typename... Args>
        [[noreturn]] void fail_check_fmt(
            const std::string_view kind,
            const std::string_view expr,
            const std::source_location& loc,
            std::string_view fmt,
            const Args&... args
        ) {
            fail_check(kind, expr, format_msg(fmt, args...), loc);
        }

        inline void assert_that_impl(
            const bool condition,
            const std::string_view expr,
            const std::string_view msg,
            const std::source_location& loc
        ) {
            if (!condition) [[unlikely]] {
                fail_check("assertion", expr, msg, loc);
            }
        }

        inline void check_impl(
            const bool condition,
            const std::string_view expr,
            const std::string_view msg,
            const std::source_location& loc
        ) {
            if (!condition) [[unlikely]] {
                fail_check("check", expr, msg, loc);
            }
        }

        template <typename... Args>
        void assert_that_fmt_impl(
            const bool condition,
            std::string_view expr,
            const std::source_location& loc,
            std::string_view fmt,
            const Args&... args
        ) {
            if (!condition) [[unlikely]] {
                fail_check_fmt("assertion", expr, loc, fmt, args...);
            }
        }

        template <typename... Args>
        void check_fmt_impl(
            const bool condition,
            std::string_view expr,
            const std::source_location& loc,
            std::string_view fmt,
            const Args&... args
        ) {
            if (!condition) [[unlikely]] {
                fail_check_fmt("check", expr, loc, fmt, args...);
            }
        }
    }  // namespace detail

    inline void assert_that(
        const bool condition,
        const std::string_view msg = {},
        const std::source_location& loc = std::source_location::current()
    ) {
        detail::assert_that_impl(condition, "condition", msg, loc);
    }

    inline void check(
        const bool condition,
        const std::string_view msg = {},
        const std::source_location& loc = std::source_location::current()
    ) {
        detail::check_impl(condition, "condition", msg, loc);
    }
}  // namespace oxide

#endif  // OXIDE_ASSERT_HPP
