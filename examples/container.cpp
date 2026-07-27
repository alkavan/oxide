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
#include <oxide.hpp>
#include <oxide/container.hpp>

#include <iostream>
#include <string>

// MultiVec (heterogeneous container) usage example
int main() {
    using namespace oxide;

    MultiVec<int, std::string, double> bag;

    bag.push(1);
    bag.push(2);
    bag.push(3);
    bag.push(std::string{"oxide"});
    bag.push(3.14);

    // len<T>() / total_len()
    std::cout << "int length: " << bag.len<int>() << "\n";
    std::cout << "string length: " << bag.len<std::string>() << "\n";
    std::cout << "double length: " << bag.len<double>() << "\n";
    std::cout << "Total length: " << bag.total_len() << "\n";

    // pop<T>()
    if (const auto popped = bag.pop<int>()) {
        std::cout << "Popped int: " << *popped << "\n";
    }
    std::cout << "New int length: " << bag.len<int>() << "\n";

    auto empty_pop = bag.pop<int>();
    while (empty_pop) {
        empty_pop = bag.pop<int>();
    }

    if (const auto none = bag.pop<int>(); !none) {
        std::cout << "Empty int pop: None\n";
    }

    // get<T>()
    bag.push(10);
    bag.push(20);

    if (const auto val = bag.get<int>(0)) {
        std::cout << "Get<int>[0]: " << val->get() << "\n";
        val->get() = 100;  // Mutable access (modifies bag's int bucket)
    }

    if (bag.get<int>(99)) {
        // Won't reach here
    } else {
        std::cout << "Get<int>[99]: None\n";
    }

    // Const get<T>()
    const MultiVec<int, std::string> const_bag = [] {
        MultiVec<int, std::string> tmp;
        tmp.push(100);
        tmp.push(200);
        tmp.push(std::string{"const"});
        return tmp;
    }();

    if (const auto val = const_bag.get<int>(0)) {
        std::cout << "Const get<int>[0]: " << val->get() << "\n";
    }

    if (const auto val = const_bag.get<std::string>(0)) {
        std::cout << "Const get<string>[0]: " << val->get() << "\n";
    }

    // visit all elements across type buckets
    bag.visit(match{
        [](const int& i) { std::cout << "visit int: " << i << "\n"; },
        [](const std::string& s) { std::cout << "visit string: " << s << "\n"; },
        [](const double& d) { std::cout << "visit double: " << d << "\n"; },
    });

    return 0;
}
