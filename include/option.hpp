#pragma once

#include <type_traits>  // For std::decay_t, std::enable_if_t, etc.
#include <concepts>     // For std::invocable, std::same_as, etc.
#include <utility>      // For std::forward, std::move
#include <memory>       // For std::construct_at, std::destroy_at

#include "output.hpp"

namespace oxide {
    // Tag type for None (like std::nullopt_t)
    struct none_t {
        explicit none_t() = default;
    };
    inline constexpr none_t _none{};

    // ---------------------------------------------------------------------------
    // Primary template for Option<T> (value-owning, like std::optional)
    // ---------------------------------------------------------------------------
    template <typename T>
    class Option {
    private:
        alignas(T) unsigned char m_storage[sizeof(T)]{};
        bool m_has_value = false;

        T* ptr() { return reinterpret_cast<T*>(m_storage); }
        const T* ptr() const { return reinterpret_cast<const T*>(m_storage); }

    public:
        using value_type = T;

        // Constructors
        constexpr Option() noexcept : m_has_value(false) {}
        constexpr explicit Option(none_t) noexcept : Option() {}
        constexpr Option(const Option& other) {
            if (other.m_has_value) {
                std::construct_at(ptr(), *other.ptr());
                m_has_value = true;
            }
        }
        constexpr Option(Option&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            if (other.m_has_value) {
                std::construct_at(ptr(), std::move(*other.ptr()));
                m_has_value = true;
                other.reset();
            }
        }
        template <typename U = T, std::enable_if_t<std::is_constructible_v<T, U&&>, int> = 0>
        constexpr explicit Option(U&& value) : m_has_value(true) {
            std::construct_at(ptr(), std::forward<U>(value));
        }

        // Destructor
        ~Option() { reset(); }

        // Assignment
        Option& operator=(const Option& other) {
            if (this != &other) {
                reset();
                if (other.m_has_value) {
                    std::construct_at(ptr(), *other.ptr());
                    m_has_value = true;
                }
            }
            return *this;
        }

        Option& operator=(Option&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            if (this != &other) {
                reset();
                if (other.m_has_value) {
                    std::construct_at(ptr(), std::move(*other.ptr()));
                    m_has_value = true;
                    other.reset();
                }
            }
            return *this;
        }

        template <typename U = T, std::enable_if_t<std::is_constructible_v<T, U&&>, int> = 0>
        constexpr Option& operator=(U&& value) {
            reset();
            std::construct_at(ptr(), std::forward<U>(value));
            m_has_value = true;
            return *this;
        }

        Option& operator=(none_t) noexcept {
            reset();
            return *this;
        }

        // Observers
        [[nodiscard]] constexpr bool has_value() const noexcept { return m_has_value; }
        constexpr explicit operator bool() const noexcept { return has_value(); }

        constexpr T& value() & {
            if (!m_has_value) panic("called Option::value() on None");
            return *ptr();
        }
        constexpr const T& value() const& {
            if (!m_has_value) panic("called Option::value() on None");
            return *ptr();
        }
        constexpr T&& value() && {
            if (!m_has_value) panic("called Option::value() on None");
            return std::move(*ptr());
        }
        constexpr const T&& value() const&& {
            if (!m_has_value) panic("called Option::value() on None");
            return std::move(*ptr());
        }

        constexpr T& unwrap() & { return value(); }
        constexpr const T& unwrap() const& { return value(); }
        constexpr T&& unwrap() && { return std::move(value()); }
        constexpr const T&& unwrap() const&& { return std::move(value()); }

        constexpr T* operator->() noexcept { return ptr(); }
        constexpr const T* operator->() const noexcept { return ptr(); }
        constexpr T& operator*() & noexcept { return *ptr(); }
        constexpr const T& operator*() const& noexcept { return *ptr(); }
        constexpr T&& operator*() && noexcept { return std::move(*ptr()); }
        constexpr const T&& operator*() const&& noexcept { return std::move(*ptr()); }

        // Modifiers
        void reset() noexcept {
            if (m_has_value) {
                std::destroy_at(ptr());
                m_has_value = false;
            }
        }

        // Rust-like helpers (add more as needed)
        T& expect(const char* msg) & { if (!m_has_value) panic(msg); return *ptr(); }
        const T& expect(const char* msg) const& { if (!m_has_value) panic(msg); return *ptr(); }
        template <typename U>
        constexpr T unwrap_or(U&& default_value) const& { return m_has_value ? *ptr() : std::forward<U>(default_value); }
        template <typename U>
        constexpr T unwrap_or(U&& default_value) && { return m_has_value ? std::move(*ptr()) : std::forward<U>(default_value); }

        // and_then, like std::optional::and_then (C++23)
        template <typename F>
        requires std::invocable<F&&, T&&> &&
                 requires { typename std::invoke_result_t<F&&, T&&>::value_type; } &&
                 std::same_as<std::remove_cvref_t<std::invoke_result_t<F&&, T&&>>,
                              Option<typename std::invoke_result_t<F&&, T&&>::value_type>>
        constexpr auto and_then(F&& f) && {
            using Rt = std::invoke_result_t<F&&, T&&>;
            using U = Rt::value_type;
            if (!has_value()) {
                return Option<U>(_none);
            }
            return std::forward<F>(f)(std::move(value()));
        }

        template <typename F>
        requires std::invocable<F&&, const T&> &&
                 requires { typename std::invoke_result_t<F&&, const T&>::value_type; } &&
                 std::same_as<std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>,
                              Option<typename std::invoke_result_t<F&&, const T&>::value_type>>
        constexpr auto and_then(F&& f) const& {
            using Rt = std::invoke_result_t<F&&, const T&>;
            using U = typename Rt::value_type;
            if (!has_value()) {
                return Option<U>(_none);
            }
            return std::forward<F>(f)(value());
        }
    };

    // ---------------------------------------------------------------------------
    // Partial specialization for Option<T&> (non-const reference)
    // ---------------------------------------------------------------------------
    template <typename T>
    class Option<T&> {
    private:
        T* m_ptr = nullptr;

    public:
        using value_type = T&;

        constexpr Option() noexcept = default;
        constexpr explicit Option(none_t) noexcept : Option() {}
        constexpr explicit Option(T& ref) noexcept : m_ptr(&ref) {}
        constexpr Option(const Option&) noexcept = default;
        constexpr Option& operator=(const Option&) noexcept = default;

        [[nodiscard]] constexpr bool has_value() const noexcept { return m_ptr != nullptr; }
        constexpr explicit operator bool() const noexcept { return has_value(); }

        T& value() const {
            if (!m_ptr) panic("called Option::value() on None");
            return *m_ptr;
        }
        T& unwrap() const { return value(); }

        T* operator->() const noexcept { return m_ptr; }
        T& operator*() const noexcept { return *m_ptr; }

        T& expect(const char* msg) const { if (!m_ptr) panic(msg); return *m_ptr; }
        T& unwrap_or(T& default_value) const noexcept { return m_ptr ? *m_ptr : default_value; }

        // and_then, like std::optional::and_then (C++23)
        template <typename F>
        requires std::invocable<F&&, T&> &&
                 requires { typename std::invoke_result_t<F&&, T&>::value_type; } &&
                 std::same_as<std::remove_cvref_t<std::invoke_result_t<F&&, T&>>,
                              Option<typename std::invoke_result_t<F&&, T&>::value_type>>
        constexpr auto and_then(F&& f) const& {
            using Rt = std::invoke_result_t<F&&, T&>;
            using U = typename Rt::value_type;
            if (!has_value()) {
                return Option<U>(_none);
            }
            return std::forward<F>(f)(value());
        }
    };

    // ---------------------------------------------------------------------------
    // Partial specialization for Option<const T&> (const reference)
    // ---------------------------------------------------------------------------
    template <typename T>
    class Option<const T&> {
    private:
        const T* m_ptr = nullptr;

    public:
        using value_type = const T&;

        constexpr Option() noexcept = default;
        constexpr explicit Option(none_t) noexcept : Option() {}
        constexpr explicit Option(const T& ref) noexcept : m_ptr(&ref) {}
        constexpr Option(const Option&) noexcept = default;
        constexpr Option& operator=(const Option&) noexcept = default;

        [[nodiscard]] constexpr bool has_value() const noexcept { return m_ptr != nullptr; }
        constexpr explicit operator bool() const noexcept { return has_value(); }

        const T& value() const {
            if (!m_ptr) panic("called Option::value() on None");
            return *m_ptr;
        }
        const T& unwrap() const { return value(); }

        const T* operator->() const noexcept { return m_ptr; }
        const T& operator*() const noexcept { return *m_ptr; }

        const T& expect(const char* msg) const { if (!m_ptr) panic(msg); return *m_ptr; }
        const T& unwrap_or(const T& default_value) const noexcept { return m_ptr ? *m_ptr : default_value; }

        // and_then, like std::optional::and_then (C++23)
        template <typename F>
        requires std::invocable<F&&, const T&> &&
                 requires { typename std::invoke_result_t<F&&, const T&>::value_type; } &&
                 std::same_as<std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>,
                              Option<typename std::invoke_result_t<F&&, const T&>::value_type>>
        constexpr auto and_then(F&& f) const& {
            using Rt = std::invoke_result_t<F&&, const T&>;
            using U = typename Rt::value_type;
            if (!has_value()) {
                return Option<U>(_none);
            }
            return std::forward<F>(f)(value());
        }
    };

    // ---------------------------------------------------------------------------
    // Helper free functions for Some / None
    // ---------------------------------------------------------------------------
    template <typename T>
    constexpr Option<T> Some(T&& value) noexcept
        requires (!std::is_lvalue_reference_v<T>)
    {
        return Option<T>(std::forward<T>(value));
    }

    template <typename T>
    constexpr Option<T&> Some(T& value) noexcept {
        return Option<T&>(value);
    }

    template <typename T>
    constexpr Option<const T&> Some(const T& value) noexcept {
        return Option<const T&>(value);
    }

    template <typename T>
    requires (!std::is_reference_v<T>)
    constexpr Option<T> None() noexcept {
        return Option<T>(_none);
    }

}  // namespace oxide