# Oxide — C++ Utility Library, Rust Idiomatic

A header-only C++(23) library for writing C++ code like it's Rust;
with emphasis on simplicity, standards, performance and the Rust idiom.

This library encourages C++ users to take a more functional approach when writing code
and demonstrates how expressive C++ can be.  It's a header-only and namespaced library,
just plug `#include "oxide.hpp"` into your project for the simplest use-case.

While I do expect both Rust and C++ shills to hate it,
this had to be done for the sake of all non-business people languages,
hope you enjoy it!

## Build
(optional, for packaging)

```
mkdir build && cd build
cmake -G Ninja -DCMAKE_INSTALL_PREFIX=../local ..
ninja
ninja install
cd ..
```

## Usage
For comprehensive examples demonstrating these features in action,
refer to the `examples.cpp` file or explorer the fully working examples below.

### Match (non-functional approach)

* The value comes first on the left side of the operator.
* The match expression with lambdas follows in braces.
* This resembles standard C++ operator overloading and is more intuitive.

```cpp
#include <functional>
#include <array>
#include <iostream>
#include <oxide.hpp>

struct Quit {};
struct Move { int x, y; };
struct Write { std::string text; };
struct Read { std::function<void()> callback; };

using Message = oxide::Union<Quit, Move, Write, Read>;

int main() {
    namespace ox = oxide;

    std::array<Message, 4> msgs = {
        Quit{},
        Move{1, 2},
        Write{"Writing #1 .."},
        Read{[] {std::cout << "Reading...\n";}}
    };

    for (auto& msg : msgs) {
        // Operator match syntax
        msg >> ox::match {
            [](const Quit&) { std::cout << "Quit\n"; },
            [](Move& m) { m.x++; m.y++; std::cout << "Move: (" << m.x << ", " << m.y << ")\n"; },
            [](Write& w) { std::cout << "Write: " << w.text.append(".") << "\n"; },
            [](const Read& r) { r.callback(); }
        };
    }

    return 0;
}
```

### Functional Example
```cpp
#include <functional>
#include <array>
#include <iostream>
#include <oxide.hpp>

struct Quit {};
struct Move { int x, y; };
struct Write { std::string text; };
struct Read { std::function<void()> callback; };

// Your message types (can be one of these)
using Message = oxide::Union<Quit, Move, Write, Read>;

// Non-template factory functions
constexpr Message quit() { return Message{Quit{}}; }
constexpr Message move_to(const int x, const int y) { return Message{Move{x, y}}; }
Message write(std::string text) { return Message{Write{std::move(text)}}; }
Message read(std::function<void()> callback) { return Message{Read{std::move(callback)}}; }

oxide::Option<std::pair<int, int>> get_coordinates(const Message& msg);

int main() {
    namespace ox = oxide;
    
    // Alternative way to define messages (type is implicit)
    std::array msgs_functional = {
        quit(),
        move_to(1, 2),
        move_to(2, 3),
        write("Writing #2 ..."),
        read([](){std::cout << "Reading...\n";})
    };

    const ox::Vec<Message> msg_vec(msgs_functional.begin(), msgs_functional.end());
    
    auto is_move_predicate = [](const Message& msg){
        return std::holds_alternative<Move>(msg);
    };

     if (auto found_move = ox::find(msg_vec.iter(), is_move_predicate); found_move.has_value()) {
        std::cout << "Found a Move message!\n";

        if (auto coords = found_move.and_then(get_coordinates)) {
            std::cout << "Move coordinates: (" << coords->first << ", " << coords->second << ")\n";
        }
    } else {
        std::cout << "No Move message found\n";
    }
    
    return 0;
}

oxide::Option<std::pair<int, int>> get_coordinates(const Message& msg) {
    if (const auto* move_msg = std::get_if<Move>(&msg)) {
        return oxide::Some(std::make_pair(move_msg->x, move_msg->y));
    }
    return oxide::None;
}
```

### Type-Implicit Match Example
(with optional settings affecting processing)

```cpp
#include <functional>
#include <array>
#include <iostream>
#include <oxide.hpp>

struct Quit {};
struct Move { int x, y; };
struct Write { std::string text; };
struct Read { std::function<void()> callback; };

// Your message types (can be one of these)
using Message = oxide::Union<Quit, Move, Write, Read>;

// Non-template factory functions
constexpr Message quit() { return Message{Quit{}}; }
constexpr Message move_to(const int x, const int y) { return Message{Move{x, y}}; }
Message write(std::string text) { return Message{Write{std::move(text)}}; }
Message read(std::function<void()> callback) { return Message{Read{std::move(callback)}}; }

int main() {
    namespace ox = oxide;

    std::array msgs_functional = {
        quit(),
        move_to(1, 2),
        move_to(2, 3),
        write("Writing #2 ..."),
        read([](){std::cout << "Reading...\n";})
    };

    // Convert to vector for the find function
    const ox::Vec<Message> msg_vec(msgs_functional.begin(), msgs_functional.end());

    // Optional configuration
    ox::Option<std::string> user_name = ox::Some(std::string("Player1"));
    ox::Option<int> max_moves = ox::None;  // Not configured

    std::cout << "User: " << user_name.value_or("Anonymous") << "\n";
    std::cout << "Max moves: " << max_moves.value_or(100) << "\n";

    auto process_with_context = [&](const Message& msg) {
        msg >> ox::match {
            [&user_name](const Quit&) {
                std::cout << user_name.value_or("Someone") << " wants to quit\n";
            },
            [&max_moves](const Move& m) {
                std::cout << "Processing move: (" << m.x << ", " << m.y << ")";
                if (max_moves) {
                    std::cout << " (limit: " << *max_moves << ")";
                }
                std::cout << "\n";
            },
            [](const Write& w) { std::cout << "Processing write: " << w.text << "\n"; },
            [](const Read& r) { std::cout << "Processing read operation\n"; }
        };
    };

    std::cout << "\nProcessing with context:\n";
    for (const auto& msg : msg_vec.iter()) {
        process_with_context(msg);
    }
    
    return 0;
}
```

### Results, Errors and Chaining
```cpp
#include <iostream>
#include <oxide.hpp>

int main() {
    using oxide::Result;

    auto divide = [](const int a, const int b) -> Result<int> {
        if (b == 0) return std::unexpected("Division by zero");
        return a / b;
    };

    Result<int> ok_res = divide(84, 2);
    Result<int> err_res = divide(84, 0);

    if (ok_res.has_value()) {
        std::cout << "Ok: " << ok_res.value() << "\n";
    } else {
        std::cout << "Err: " << ok_res.error() << "\n";
    }

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
```

### Database Example (Vec)
```cpp
#include <functional>
#include <iostream>
#include <oxide.hpp>

struct Insert { std::string key; int value; };
struct Update { std::string key; int new_value; };
struct Delete { std::string key; };
struct Select { std::string key; std::function<void(int)> callback; };
struct Noop {};

// Your message types (can be one of these)
using Operation = oxide::Union<Insert, Update, Delete, Select, Noop>;

int main() {
    oxide::Vec<std::pair<std::string, int>> db;

    // Demonstrate push()
    for (int i = 0; i < 100; ++i) {
        db.push({"user" + std::to_string(i), i * 10});
    }

    // Demonstrate len()
    std::cout << "Initial size: " << db.len() << "\n";

    // Demonstrate capacity()
    std::cout << "Capacity: " << db.capacity() << "\n";

    // Demonstrate get() and mutable access
    if (const auto record = db.get(0)) {
        std::cout << "First record: " << record->get().first << " -> " << record->get().second << "\n";
        record->get().second = 999;  // Modify
    }

    // Demonstrate iter() const
    std::cout << "First 5 records: ";
    size_t count = 0;
    for (const auto& [key, val] : db.iter()) {
        if (count >= 5) break;
        std::cout << key << ":" << val << " ";
        ++count;
    }
    std::cout << "\n";

    // Demonstrate reserve() and shrink_to_fit()
    db.reserve(10000);
    std::cout << "After reserve(10000), capacity: " << db.capacity() << "\n";
    db.shrink_to_fit();
    std::cout << "After shrink_to_fit, capacity: " << db.capacity() << "\n";

    // Message examples with pattern matching for database operations
    const Operation op1 = Insert{"user100", 1000};
    const Operation op2 = Update{"user50", 500};
    const Operation op3 = Delete{"user25"};
    const Operation op4 = Select{
        "user75",
        [](const int value){
        std::cout << "Queried value: " << value << "\n";
    }};
    const Operation op5 = Noop{};

    // Demonstrate handler using oxide::match{}
    auto db_handler = [&](const Operation& msg) {
        msg >> oxide::match{
            [&](const Insert& ins) {
                db.push({ins.key, ins.value});
                std::cout << "Inserted: " << ins.key << " -> " << ins.value << "\n";
            },
            [&](const Update& upd) {
                bool found = false;
                for (auto& [key, val] : db.iter_mut()) {
                    if (key == upd.key) {
                        val = upd.new_value;
                        std::cout << "Updated: " << upd.key << "=" << upd.new_value << "\n";
                        found = true;
                        break;
                    }
                }
                if (!found) std::cout << "Update failed: key not found\n";
            },
            [&](const Delete& del) {
                bool found = false;
                for (size_t i = 0; i < db.len(); ++i) {
                    if (const auto rec = db.get(i); rec && rec->get().first == del.key) {
                        auto [key, val] = db.remove(i);
                        std::cout << "Deleted: " << key << "=" << val << "\n";
                        found = true;
                        break;
                    }
                }
                if (!found) std::cout << "Delete failed: key not found\n";
            },
            [&](const Select& sel) {
                bool found = false;
                for (const auto& [key, val] : db.iter()) {
                    if (key == sel.key) {
                        sel.callback(val);
                        found = true;
                        break;
                    }
                }
                if (!found) std::cout << "Select failed: key not found\n";
            },
            [&](const Noop&) {
                std::cout << "No operation performed\n";
            }
        };
    };

    // Process operations
    db_handler(op1);
    db_handler(op2);
    db_handler(op3);
    db_handler(op4);
    db_handler(op5);

    // Final state
    std::cout << "Final database size: " << db.len() << "\n";
    std::cout << "Is empty: " << (db.is_empty() ? "true" : "false") << "\n";

    return 0;
}
```


### Covariant Dispatch Example
```cpp
#include <iostream>
#include <oxide.hpp>

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
```
