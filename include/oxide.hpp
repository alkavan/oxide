#ifndef OXIDE_HPP
#define OXIDE_HPP
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

#include <variant>
#include <string>
#include <utility>
#include <expected>
#include <vector>
#include <ranges>
#include <type_traits>

#define OXIDE_VERSION_MAJOR 1
#define OXIDE_VERSION_MINOR 2
#define OXIDE_VERSION_PATCH 0

#include "oxide/option.hpp"

namespace oxide {
    // Union type
    template <typename... Variants>
    using Union = std::variant<Variants...>;

    // Pattern matching template
    template <class... Handlers>
    struct match : Handlers... {
        using Handlers::operator()...;
    };

    // Overload >> for visitation
    template <typename Variant, typename Matcher>
    void operator>>(Variant&& v, Matcher&& m) {
        std::visit(std::forward<Matcher>(m), std::forward<Variant>(v));
    }

    // Overloaded helper for macro-based matching
    template <class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

    // Result type
    template <typename T, typename E = std::string>
    using Result = std::expected<T, E>;

    /**
     * @brief Finds the first element in the given range that satisfies the predicate.
     *
     * This function iterates over the range and applies the predicate to each element.
     * If an element is found that satisfies the predicate, it returns an Option containing
     * the moved element. If no such element is found, it returns None.
     *
     * @tparam R The type of the range to search in.
     * @tparam P The type of the predicate function (callable that takes an element and returns bool).
     * @param range The range to search in (forwarded).
     * @param pred The predicate to apply to each element.
     * @return An Option containing the first matching element (moved), or None if not found.
     */
    template <typename R, typename P>
    [[nodiscard]] auto find(R&& range, P&& pred) -> Option<std::remove_cv_t<std::remove_reference_t<decltype(*std::ranges::begin(range))>>> {
        using ValueType = std::remove_cv_t<std::remove_reference_t<decltype(*std::ranges::begin(range))>>;
        for (auto&& item : std::forward<R>(range)) {
            if (pred(item)) {
                return Some(ValueType(std::move(item)));
            }
        }
        return {};
    }
}

#endif // OXIDE_HPP