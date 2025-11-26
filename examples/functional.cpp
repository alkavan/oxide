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

    // =============================================================================
    // 1. Result<T> Example
    // =============================================================================

    // Example division function with error
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

    // =============================================================================
    // 2. Value-Owning Option<T> (like std::optional<T>)
    // =============================================================================

    // Default ctor / None
    Option<int> opt_none;
    std::cout << "\nDefault/None has_value: " << opt_none.has_value() << "\n";

    // Free function None<T>
    const Option<int> opt_none2 = None<int>();
    std::cout << "None<int>() has_value: " << opt_none2.has_value() << "\n";

    // explicit(none_t)
    const Option<int> opt_none3(_none);
    std::cout << "Option(_none) has_value: " << opt_none3.has_value() << "\n";

    // explicit bool conversion
    if (!opt_none) {
        std::cout << "operator bool() works for None\n";
    }

    // unwrap_or (const& and && overloads)
    int default_val = 999;
    std::cout << "unwrap_or on None (&): " << opt_none.unwrap_or(default_val) << "\n";
    std::cout << "unwrap_or on None (&&): " << std::move(opt_none).unwrap_or(888) << "\n";

    // Some constructors
    Option<int> opt_some(42);  // template explicit(U&&)
    std::cout << "Option(42) value: " << opt_some.value() << "\n";

    Option<int> opt_some2 = Some(84);
    std::cout << "Some(84) value: " << opt_some2.value() << "\n";

    // operator bool
    if (opt_some) {
        std::cout << "operator bool() true for Some\n";
    }

    // value() overloads: &, const&, &&, const&&
    opt_some.value() = 100;  // &
    std::cout << "value() & modified: " << opt_some.value() << "\n";  // still 100

    const Option<int>& const_opt = opt_some;
    const int& const_ref = const_opt.value();  // const&
    std::cout << "value() const&: " << const_ref << "\n";

    int rvalue = std::move(opt_some2).value();  // &&
    std::cout << "value() &&: " << rvalue << "\n";

    // unwrap aliases value()
    std::cout << "unwrap(): " << opt_none2.unwrap_or(0) << "\n";

    // expect()
    std::cout << "expect(): " << opt_some.expect("boom on None") << "\n";

    // operators * and ->
    struct TestStruct {
        int x = 0;
        [[nodiscard]] int y() const { return x * 2; }
    };
    Option<TestStruct> opt_struct = Some(TestStruct{21});
    std::cout << "operator*()->x: " << opt_struct->x << "\n";
    std::cout << "operator*().y(): " << opt_struct->y() << "\n";
    std::cout << "operator*().x: " << (*opt_struct).x << "\n";

    // reset()
    opt_struct.reset();
    std::cout << "after reset(): " << opt_struct.has_value() << "\n";

    // Assignments: copy, move, value(U&&), none_t
    Option<int> opt_copy(opt_some);  // copy ctor
    std::cout << "copy ctor value: " << opt_copy.value() << "\n";

    Option<int> opt_move_ctor(std::move(opt_some));  // move ctor
    std::cout << "move ctor has_value: " << opt_move_ctor.has_value() << ", source: " << opt_some.has_value() << "\n";

    opt_copy = opt_move_ctor;  // copy assign
    std::cout << "copy assign: " << opt_copy.value() << "\n";

    Option<int> opt_assign_move = std::move(opt_move_ctor);  // move assign
    std::cout << "move assign has: " << opt_assign_move.has_value() << ", source: " << opt_move_ctor.has_value() << "\n";

    opt_assign_move = 200;  // template U&& assign
    std::cout << "U&& assign (int): " << opt_assign_move.value() << "\n";

    opt_assign_move = 300L;  // different integral type
    std::cout << "U&& assign (long): " << opt_assign_move.value() << "\n";

    opt_assign_move = _none;  // none_t assign
    std::cout << "none_t assign has: " << opt_assign_move.has_value() << "\n";

    // and_then (&&, const&, monadic chaining)
    auto chain = Some(10)
        .and_then([](const int v) -> Option<int> {  // &&
            return Some(v * 3);
        })
        .and_then([](const int v) -> Option<int> {
            return v > 25 ? Some(v + 5) : None<int>();
        });
    std::cout << "and_then chain: " << (chain.has_value() ?
        std::to_string(chain.value()) : "None") << "\n";

    const auto& const_chain = chain;
    auto const_and_then = const_chain.and_then([](const int& v) -> Option<int> {  // const&
        return Some(v * 2);
    });
    std::cout << "const& and_then: " << (const_and_then.has_value() ?
        std::to_string(const_and_then.value()) : "None") << "\n";

    // =============================================================================
    // 3. Option<T&> (non-const reference)
    // =============================================================================
    int ref_target = 50;
    Option<int&> ref_none;
    std::cout << "\nOption<int&> None has: " << ref_none.has_value() << "\n";

    Option<int&> opt_ref = Some(ref_target);
    std::cout << "Option<int&> value: " << opt_ref.value() << "\n";

    opt_ref.value() = 60;  // modifies ref_target
    std::cout << "ref_target after modify: " << ref_target << "\n";

    // uses value, doesn't assign default
    std::cout << "unwrap_or_ref: " << opt_ref.unwrap_or(ref_target = 70) << "\n";

    int def_ref_target = 999;
    std::cout << "None ref unwrap_or: " << ref_none.unwrap_or(def_ref_target) << "\n";

    // and_then for ref
    auto ref_then = opt_ref.and_then([](int& v) -> Option<int> {  // No capture needed
        v *= 2;
        return Some(int{v});
    });
    std::cout << "ref and_then: " << ref_then.value() << ", ref_target: " << ref_target << "\n";

    // Copyable trivially
    Option<int&> ref_copy = opt_ref;
    std::cout << "ref copy value: " << ref_copy.value() << "\n";

    // =============================================================================
    // 4. Option<const T&> (const reference)
    // =============================================================================
    constexpr int const_target = 80;
    Option<const int&> cref_none;
    std::cout << "\nOption<const int&> None has: " << cref_none.has_value() << "\n";

    Option<const int&> opt_cref = Some(const_target);
    std::cout << "Option<const int&> value: " << opt_cref.value() << "\n";

    std::cout << "cref unwrap_or: " << opt_cref.unwrap_or(90) << "\n";

    std::cout << "None cref unwrap_or: " << cref_none.unwrap_or(const_target) << "\n";

    // and_then const&
    auto cref_then = opt_cref.and_then([](const int& v) -> Option<int> {
        return Some(v + 10);
    });
    std::cout << "cref and_then: " << cref_then.value() << "\n";

    // Bind to non-const too
    int mutable_target = 100;
    Option<const int&> opt_mutable_cref(mutable_target);
    std::cout << "const& from non-const: " << opt_mutable_cref.value() << "\n";

    return 0;
}
