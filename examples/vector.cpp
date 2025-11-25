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

// Vector usage example
int main() {
    using namespace oxide;

    Vec<int> v{1, 2, 3};

    // len()
    std::cout << "Length: " << v.len() << "\n";

    // pop()
    if (const auto popped = v.pop()) {
        std::cout << "Popped: " << *popped << "\n";
    }
    std::cout << "New length: " << v.len() << "\n";

    auto empty_pop = v.pop();
    while (empty_pop) { empty_pop = v.pop(); }

    if (const auto none = v.pop(); !none) {
        std::cout << "Empty pop: None\n";
    }

    // get()
    v = {10, 20};

    if (const auto val = v.get(0)) {
        std::cout << "Get[0]: " << val->get() << "\n";  // Outputs: Get[0]: 10 (val is Option<ref<int>>)
        val->get() = 100;  // Mutable access (modifies v[0])
    }

    if (v.get(99)) {
        // Won't reach here
    } else {
        std::cout << "Get[99]: None\n";
    }

    // Get contact reference
    const Vec<int> cv{100, 200};
    if (const auto val = cv.get(0)) {
        std::cout << "Const get[0]: " << val->get() << "\n";
    }

    return 0;
}
