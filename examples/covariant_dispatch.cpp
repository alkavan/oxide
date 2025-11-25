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

#include <iostream>

// Define shape types (no inheritance)
struct Circle {
    double radius = 1.0;

    [[nodiscard]] double area() const {
        return 3.14159 * radius * radius;
    }
};

struct Rectangle {
    double width = 7.0, height = 14.0;

    [[nodiscard]] double perimeter() const {
        return 2 * (width + height);
    }
};

// Create a discriminated union for shapes
using ShapeVariant = oxide::Union<Circle, Rectangle>;

// Clone function using pattern matching on the Union
ShapeVariant clone(const ShapeVariant& shape) {
    return std::visit(oxide::overloaded{
        [](const Circle& c) -> ShapeVariant { return Circle{c.radius}; },
        [](const Rectangle& r) -> ShapeVariant { return Rectangle{r.width, r.height}; }
    }, shape);
}

int main() {
    constexpr ShapeVariant circle = Circle{};
    auto cloned_circle = clone(circle);

    constexpr ShapeVariant rectangle = Rectangle{};
    auto cloned_rectangle = clone(rectangle);

    // Use oxide's match and operator>> to polymorphically compute and print properties (akin to covariant dispatch)
    cloned_circle >> oxide::match{
        [](const Circle& c) { std::cout << "Cloned Circle area: " << c.area() << std::endl; },
        [](const Rectangle& r) { std::cout << "Cloned Rectangle perimeter: " << r.perimeter() << std::endl; }
    };

    cloned_rectangle >> oxide::match{
        [](const Circle& c) { std::cout << "Cloned Circle area: " << c.area() << std::endl; },
        [](const Rectangle& r) { std::cout << "Cloned Rectangle perimeter: " << r.perimeter() << std::endl; }
    };

    return 0;
}
