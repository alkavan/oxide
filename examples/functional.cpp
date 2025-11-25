/*
 *  Copyright (C) 2025 Igal Alkon and ALKONTEK
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
#include <oxide.hpp>

#include <functional>
#include <iostream>

// Functional usage example
int main() {
    using namespace oxide;

    auto divide = [](const int a, const int b) -> Result<int> {
        if (b == 0) return std::unexpected("Division by zero");
        return a / b;
    };

    Result<int> ok_res = divide(84, 2);
    Result<int> err_res = divide(84, 0);

    // Built-in methods: has_value() like is_ok, value() like unwrap (throws if error), error(), value_or(default)
    if (ok_res.has_value()) {
        std::cout << "Ok: " << ok_res.value() << "\n";
    } else {
        std::cout << "Err: " << ok_res.error() << "\n";
    }

    // Monadic chaining (Rust-like map/and_then) - capture divide by reference
    const auto chained = ok_res.and_then([&divide](const int val) -> Result<int> {
        return divide(val, 3);  // Now divide is accessible via capture
    }).transform([](const int val) { return val * 2; });  // Maps success value

    if (chained.has_value()) {
        std::cout << "Chained Ok: " << chained.value() << "\n";
    }

    // For err_res, similar handling or or_else for recovery
    const auto recovered = err_res.or_else([](const std::string&) -> Result<int> {
        return 0;  // Recover from error
    });
    std::cout << "Recovered: " << recovered.value_or(-1) << "\n";

    return 0;
}
