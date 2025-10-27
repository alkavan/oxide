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
#include <array>
#include <iostream>

struct Quit {};  // Unit variant
struct Move { int x, y; };  // Struct variant
struct Write { std::string text; };  // Tuple-like (but using struct for named field)
struct Read { std::function<void()> callback; };  // Variant holding a lambda or function

// Your message types (can be one of these)
using Message = oxide::Union<Quit, Move, Write, Read>;

// Non-template factory functions
constexpr Message quit() { return Message{Quit{}}; }
constexpr Message move_to(const int x, const int y) { return Message{Move{x, y}}; }
Message write(std::string text) { return Message{Write{std::move(text)}}; }
Message read(std::function<void()> callback) { return Message{Read{std::move(callback)}}; }

// Example helper functions
oxide::Option<std::pair<int, int>> get_coordinates(const Message& msg);

// Usage example
int main() {
    namespace ox = oxide;

    std::array<Message, 4> msgs = {
        Quit{},
        Move{1, 2},
        Write{"Writing #1 .."},
        Read{[](){std::cout << "Reading...\n";}}
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

    // Optional configuration
    ox::Option<std::string> user_name = ox::Some(std::string("Player1"));
    ox::Option<int> max_moves = ox::None;  // Not configured

    std::cout << "User: " << user_name.value_or("Anonymous") << "\n";
    std::cout << "Max moves: " << max_moves.value_or(100) << "\n";

    // Alternative way to define messages (type is implicit)
    std::array msgs_functional = {
        quit(),
        move_to(1, 2),
        move_to(2, 3),
        write("Writing #2 ..."),
        read([](){std::cout << "Reading...\n";})
    };

    // Convert to vector for the find function
    ox::Vec<Message> msg_vec(msgs_functional.begin(), msgs_functional.end());

    // Is 'Move' message predicate
    auto is_move_predicate = [](const Message& msg){
        return std::holds_alternative<Move>(msg);
    };

    // Using Option to find a specific message type
    if (auto found_move = ox::find(msg_vec.iter(), is_move_predicate); found_move.has_value()) {
        std::cout << "Found a Move message!\n";

        // Chain operations with Option
        if (auto coords = found_move.and_then(get_coordinates)) {
            std::cout << "Move coordinates: (" << coords->first << ", " << coords->second << ")\n";
        }
    } else {
        std::cout << "No Move message found\n";
    }

    // Example with optional settings affecting processing
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

    for (const auto& msg : msg_vec.iter()) {
        process_with_context(msg);
    }

    using ox::Result;

    // Rust-like example with std::expected (built-in monadic ops: and_then, transform, etc.)
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

    // Vector
    oxide::Vec<int> v{1, 2, 3};

    // len()
    std::cout << "Length: " << v.len() << "\n";

    // pop()
    if (auto popped = v.pop()) {
        std::cout << "Popped: " << *popped << "\n";
    }
    std::cout << "New length: " << v.len() << "\n";

    auto empty_pop = v.pop();
    while (empty_pop) { empty_pop = v.pop(); }

    if (auto none = v.pop(); !none) {
        std::cout << "Empty pop: None\n";
    }

    // get()
    v = {10, 20};

    if (auto val = v.get(0)) {
        std::cout << "Get[0]: " << val->get() << "\n";  // Outputs: Get[0]: 10 (val is Option<ref<int>>)
        val->get() = 100;  // Mutable access (modifies v[0])
    }

    if (v.get(99)) {
        // Won't reach here
    } else {
        std::cout << "Get[99]: None\n";
    }

    // Get contact reference
    const oxide::Vec<int> cv{100, 200};
    if (auto val = cv.get(0)) {
        std::cout << "Const get[0]: " << val->get() << "\n";
    }

    return 0;
}

/**
 * Extracts the coordinates from a `Move` message contained within a `Message` variant.
 *
 * @param msg The `Message` variant to analyze, which may contain a `Move` type.
 * @return An `oxide::Option` containing a `std::pair<int, int>` with the `x` and `y`
 *         coordinates if `msg` contains a `Move` variant; otherwise, returns `oxide::None`.
 */
oxide::Option<std::pair<int, int>> get_coordinates(const Message& msg) {
    if (const auto* move_msg = std::get_if<Move>(&msg)) {
        return oxide::Some(std::make_pair(move_msg->x, move_msg->y));
    }
    return oxide::None;
}
