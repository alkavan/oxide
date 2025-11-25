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
