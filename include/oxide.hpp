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

#ifndef OXIDE_HPP
#define OXIDE_HPP

#include <variant>
#include <string>
#include <utility>
#include <expected>
#include <optional>
#include <vector>
#include <ranges>
#include <functional>
#include <stdexcept>
#include <span>
#include <iterator>

#define OXIDE_VERSION_MAJOR 1
#define OXIDE_VERSION_MINOR 1
#define OXIDE_VERSION_PATCH 0

namespace oxide {
    // Optional type
    template<typename T>
    using Option = std::optional<T>;

    // Some-value type
    template<typename T>
    constexpr Option<std::decay_t<T>> Some(T&& value) {
        return std::make_optional(std::forward<T>(value));
    }

    // No-value type
    constexpr std::nullopt_t None = std::nullopt;

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

    // Vector type
    template<typename T>
    struct Vec : protected std::vector<T> {
        using std::vector<T>::vector;  // Inherit all constructors

        /**
         * @brief Returns a const pointer to the vector's buffer.
         *
         * @return A const pointer to the first element, or nullptr if empty.
         */
        [[nodiscard]] const T* as_ptr() const noexcept {
            return this->data();
        }

        /**
         * @brief Returns a mutable pointer to the vector's buffer.
         *
         * @return A mutable pointer to the first element, or nullptr if empty.
         */
        [[nodiscard]] T* as_mut_ptr() noexcept {
            return this->data();
        }

        /**
         * @brief Removes all elements from the vector.
         */
        void clear() noexcept {
            std::vector<T>::clear();
        }

        /**
         * @brief Shortens the vector to the specified length, removing elements from the end.
         *        If len >= current length, does nothing.
         *
         * @param len The new length of the vector.
         */
        void truncate(size_t len) noexcept {
            if (len < this->size()) {
                this->erase(this->begin() + len, this->end());
            }
        }

        /**
         * @brief Returns a const slice of the vector's elements.
         *
         * @return A std::span<const T> over the vector's elements.
         */
        [[nodiscard]] std::span<const T> as_slice() const noexcept {
            return std::span<const T>(this->data(), this->size());
        }

        /**
         * @brief Returns a mutable slice of the vector's elements.
         *
         * @return A std::span<T> over the vector's elements.
         */
        [[nodiscard]] std::span<T> as_mut_slice() noexcept {
            return std::span<T>(this->data(), this->size());
        }

        /**
         * @brief Accesses the element at the specified index.
         *
         * @param index The index of the element to access (must be within bounds).
         * @return A reference to the element at the given index.
         * @throws std::out_of_range if the index is out of bounds.
         */
        [[nodiscard]] auto operator[](const size_t index) -> T& {
            auto opt = get(index);
            if (!opt) {
                throw std::out_of_range("index out of bounds");
            }
            return opt.value().get();
        }

        /**
         * @brief Accesses the element at the specified index (const version).
         *
         * @param index The index of the element to access (must be within bounds).
         * @return A const reference to the element at the given index.
         * @throws std::out_of_range if the index is out of bounds.
         */
        [[nodiscard]] auto operator[](const size_t index) const -> const T& {
            auto opt = get(index);
            if (!opt) {
                throw std::out_of_range("index out of bounds");
            }
            return opt.value().get();
        }

        /**
         * @brief Returns the number of elements in the vector.
         *
         * @return The size of the vector as a size_t.
         */
        [[nodiscard]] constexpr auto len() const noexcept -> size_t {
            return this->size();
        }

        /**
         * @brief Removes and returns the last element from the vector if it exists.
         *
         * @return An Option containing the moved last element if the vector is not empty,
         *         otherwise None.
         */
        [[nodiscard]] auto pop() -> Option<T> {
            if (this->empty()) {
                return None;
            }
            T value = std::move(this->back());
            this->pop_back();
            return Some(std::move(value));
        }

        /**
         * @brief Retrieves a reference to the element at the specified index if it exists.
         *
         * @param index The index of the element to retrieve.
         * @return An Option containing a reference to the element at the given index
         *         if the index is within bounds (less than the size of the vector),
         *         otherwise None.
         */
        [[nodiscard]] auto get(size_t index) -> Option<std::reference_wrapper<T>> {
            if (index < this->size()) {
                return Some(std::ref(this->data()[index]));
            }
            return None;
        }

        /**
         * @brief Retrieves a const reference to the element at the specified index if it exists.
         *
         * @param index The index of the element to retrieve.
         * @return An Option containing a const reference to the element at the given index
         *         if the index is within bounds (less than the size of the vector),
         *         otherwise None.
         */
        [[nodiscard]] auto get(size_t index) const -> Option<std::reference_wrapper<const T>> {
            if (index < this->size()) {
                return Some(std::cref(this->data()[index]));
            }
            return None;
        }

        /**
         * @brief Appends an element to the back of the vector.
         *
         * @param value The value to append (moved into the vector).
         */
        void push(T value) {
            this->push_back(std::move(value));
        }

        /**
         * @brief Inserts an element at the specified position in the vector.
         *        Throws if the index is greater than the current length.
         *
         * @param index The position at which to insert the element (0 <= index <= len()).
         * @param value The value to insert (moved into the vector).
         * @throws std::out_of_range if index > len().
         */
        void insert(size_t index, T value) {
            if (index > this->size()) {
                throw std::out_of_range("insert index out of bounds");
            }
            std::vector<T>::insert(this->begin() + index, std::move(value));
        }

        /**
         * @brief Removes the element at the specified position and returns it.
         *        It shifts subsequent elements down and throws if the index is out of bounds.
         *
         * @param index The index of the element to remove (0 <= index < len()).
         * @return The removed element (moved out).
         * @throws std::out_of_range if index >= len().
         */
        [[nodiscard]] auto remove(size_t index) -> T {
            if (index >= this->size()) {
                throw std::out_of_range("remove index out of bounds");
            }
            T value = std::move((*this)[index]);
            this->erase(this->begin() + index);
            return value;
        }

        /**
         * @brief Checks if the vector contains no elements.
         *
         * @return true if the vector is empty, false otherwise.
         */
        [[nodiscard]] bool is_empty() const noexcept {
            return this->empty();
        }

        /**
         * @brief Returns the total number of elements the vector can hold without reallocating.
         *
         * @return The capacity of the vector as a size_t.
         */
        [[nodiscard]] size_t capacity() const noexcept {
            return static_cast<const std::vector<T>&>(*this).capacity();
        }

        /**
         * @brief Reserves capacity for at least `additional` more elements.
         *        May reallocate if current capacity is insufficient.
         *
         * @param additional The number of additional elements to reserve space for.
         */
        void reserve(size_t additional) {
            static_cast<std::vector<T>&>(*this).reserve(this->size() + additional);
        }

        /**
         * @brief Requests the vector to shrink its capacity to fit its size.
         *        This is a non-binding request; the capacity may not change.
         */
        void shrink_to_fit() noexcept {
            static_cast<std::vector<T>&>(*this).shrink_to_fit();
        }

        /**
         * @brief Returns a const range over the elements (for immutable iteration).
         *
         * @return A subrange representing the const view of the vector.
         */
        [[nodiscard]] auto iter() const noexcept -> std::ranges::subrange<typename std::vector<T>::const_iterator> {
            return std::ranges::subrange(this->cbegin(), this->cend());
        }

        /**
         * @brief Returns a mutable range over the elements (for mutable iteration).
         *
         * @return A subrange representing the mutable view of the vector.
         */
        [[nodiscard]] auto iter_mut() noexcept -> std::ranges::subrange<typename std::vector<T>::iterator> {
            return std::ranges::subrange(this->begin(), this->end());
        }

        /**
         * @brief An iterator that allows draining elements from a container,
         *        consuming the container as it iterates.
         */
        class DrainIterator;

        /**
         * @brief A sentinel structure used to indicate the end of a drain operation.
         *
         * DrainSentinel serves as a marker or terminator in contexts where draining
         * resources or data streams is required, ensuring safe and controlled termination.
         */
        struct DrainSentinel;

        friend bool operator==(const DrainIterator& it, const DrainSentinel& sent) noexcept {
            return it.vec_ == sent.vec_ && it.index_ == sent.end_;
        }

        friend bool operator!=(const DrainIterator& it, const DrainSentinel& sent) noexcept {
            return !(it == sent);
        }

        friend std::ptrdiff_t operator-(const DrainSentinel& sent, const DrainIterator& it) noexcept {
            return static_cast<std::ptrdiff_t>(sent.end_ - it.index_);
        }

        friend std::ptrdiff_t operator-(const DrainIterator& it, const DrainSentinel& sent) noexcept {
            return -(sent - it);
        }

        /**
         * @brief An iterator that drains elements from a container by transferring ownership.
         */
        class DrainIterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&&;

            DrainIterator() = delete;
            DrainIterator(Vec<T>* vec, size_t start, size_t end)
                : vec_(vec), start_(start), index_(start), end_(end) {
                if (start > end || end > vec_->size()) {
                    throw std::out_of_range("drain range out of bounds");
                }
            }

            DrainIterator(const DrainIterator&) = delete;
            DrainIterator& operator=(const DrainIterator&) = delete;

            DrainIterator(DrainIterator&& other) noexcept
                : vec_(other.vec_), start_(other.start_), index_(other.index_), end_(other.end_) {
                other.vec_ = nullptr;
            }

            DrainIterator& operator=(DrainIterator&& other) noexcept {
                if (this != &other) {
                    this->~DrainIterator();  // Erase if this was active
                    vec_ = other.vec_;
                    start_ = other.start_;
                    index_ = other.index_;
                    end_ = other.end_;
                    other.vec_ = nullptr;
                }
                return *this;
            }

            T&& operator*() {
                return std::move((*vec_)[index_]);
            }

            DrainIterator& operator++() {
                ++index_;
                return *this;
            }

            DrainIterator operator++(int) {
                DrainIterator tmp = *this;
                ++index_;
                return tmp;
            }

            friend bool operator==(const DrainIterator& a, const DrainIterator& b) noexcept {
                return a.vec_ == b.vec_ && a.index_ == b.index_;
            }

            ~DrainIterator() {
                if (vec_) {
                    vec_->erase(vec_->begin() + start_, vec_->begin() + end_);
                    vec_ = nullptr;
                }
            }

        private:
            Vec<T>* vec_;
            size_t start_;
            size_t index_;
            size_t end_;
        };

        struct DrainSentinel {
            Vec<T>* vec_ = nullptr;
            size_t end_ = 0;

            // TODO: do not make this explicit
            operator DrainIterator() const noexcept {
                return DrainIterator(vec_, end_, end_);
            }
        };

        /**
         * @brief Drains elements from the specified range,
         *        returning a subrange view for iteration and removal.
         *
         * @param range The range specifying the elements to drain.
         * @return A subrange with DrainIterator and DrainSentinel representing the drained elements.
         */
        [[nodiscard]] auto drain(std::ranges::range auto range) -> std::ranges::subrange<DrainIterator, DrainSentinel> {
            auto [start, end] = [&]() -> std::pair<size_t, size_t> {
                if constexpr (std::is_same_v<decltype(range), std::ranges::subrange<typename std::vector<T>::iterator>>) {
                    return {std::distance(this->begin(), range.begin()), std::distance(this->begin(), range.end())};
                } else {
                    return {range.begin(), range.end()};
                }
            }();
            if (start > end || end > this->size()) {
                throw std::out_of_range("drain range out of bounds");
            }
            return std::ranges::subrange<DrainIterator, DrainSentinel>(DrainIterator(this, start, end), DrainSentinel{this, end});
        }
    };
    
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
    [[nodiscard]] auto find(R&& range, P&& pred) -> Option<std::remove_reference_t<decltype(*std::ranges::begin(range))>> {
        for (auto&& item : std::forward<R>(range)) {
            if (pred(item)) {
                return Some(std::move(item));
            }
        }
        return None;
    }
}

#endif // OXIDE_HPP
