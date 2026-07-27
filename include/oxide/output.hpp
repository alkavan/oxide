#ifndef OXIDE_OUTPUT_HPP
#define OXIDE_OUTPUT_HPP
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

#include <cstdlib>      // std::abort
#include <iostream>     // std::cerr
#include <sstream>      // std::ostringstream
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <utility>      // std::forward, std::index_sequence

namespace oxide {
    namespace detail {
        // Convert one argument through iostreams (no std::format).
        template <typename T>
        inline std::string arg_to_string(const T& value) {
            std::ostringstream out;
            out << value;
            return out.str();
        }

        // Runtime "{}" formatter. Intentionally NOT std::format:
        // clangd + libstdc++ choke on format_string/make_format_args consteval paths.
        template <typename... Args>
        inline std::string format_msg(std::string_view fmt, const Args&... args) {
            constexpr std::size_t n = sizeof...(Args);
            std::string values[n > 0 ? n : 1];
            if constexpr (n > 0) {
                std::size_t i = 0;
                ((values[i++] = arg_to_string(args)), ...);
            }

            std::string out;
            out.reserve(fmt.size() + 32);

            std::size_t arg_index = 0;
            for (std::size_t i = 0; i < fmt.size(); ++i) {
                const char ch = fmt[i];
                if (ch == '{' && i + 1 < fmt.size()) {
                    if (fmt[i + 1] == '{') {  // escaped "{{"
                        out.push_back('{');
                        ++i;
                        continue;
                    }
                    if (fmt[i + 1] == '}') {  // replacement "{}"
                        if (arg_index < n) {
                            out += values[arg_index++];
                        }
                        ++i;
                        continue;
                    }
                }
                if (ch == '}' && i + 1 < fmt.size() && fmt[i + 1] == '}') {  // escaped "}}"
                    out.push_back('}');
                    ++i;
                    continue;
                }
                out.push_back(ch);
            }
            return out;
        }
    }  // namespace detail

    [[noreturn]] inline void panic(std::string_view msg) {
        std::cerr << "panic: " << msg << '\n';
        std::abort();
    }

    template <typename... Args>
    [[noreturn]] inline void panic_fmt(std::string_view fmt, const Args&... args) {
        panic(detail::format_msg(fmt, args...));
    }

    [[noreturn]] inline void unreachable(
        std::string_view msg = "entered unreachable code"
    ) {
        std::cerr << "unreachable: " << msg << '\n';
        std::abort();
    }

    [[noreturn]] inline void todo(std::string_view msg = "not yet implemented") {
        std::cerr << "todo: " << msg << '\n';
        std::abort();
    }

    [[noreturn]] inline void unimplemented(
        std::string_view msg = "not implemented"
    ) {
        std::cerr << "unimplemented: " << msg << '\n';
        std::abort();
    }
}  // namespace oxide

#endif  // OXIDE_OUTPUT_HPP