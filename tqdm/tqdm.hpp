#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <type_traits>

/**
 * @brief IteratorHook class to create an iterator wrapper with hooks.
 *
 * This class provides a mechanism to create an iterator with hooks, allowing additional actions or functions
 * to be triggered at specific points during iteration.
 *
 * @tparam IteratorType The type of the underlying iterator to be wrapped.
 */
template <class IteratorType>
class IteratorHook {
public:
    /**
     * @brief Inner Iterator class to manage the wrapped iterator with hooks.
     */
    class Iterator {
    public:
        typedef typename IteratorType::difference_type difference_type;
        typedef typename IteratorType::value_type value_type;
        typedef typename IteratorType::reference reference;
        typedef typename IteratorType::pointer pointer;
        typedef std::forward_iterator_tag iterator_category;

        Iterator() : it_(IteratorType()), hook_(nullptr) {}
        Iterator(const Iterator& it) : it_(it.it_), hook_(nullptr) {}
        Iterator(const IteratorType& it, std::function<void(IteratorType)>* hook) : it_(it), hook_(hook) {}

        Iterator& operator=(const Iterator& it)
        {
            it_ = it.it_;
            hook_ = it.hook_;
            return *this;
        }
        bool operator==(const Iterator& it) const { return it_ == it.it_; }
        bool operator!=(const Iterator& it) const { return it_ != it.it_; }
        Iterator& operator++()
        {
            if (hook_) (*hook_)(++it_);
            return *this;
        }
        reference operator*() const { return *it_; }
        pointer operator->() const { return it_.operator->(); }

        IteratorType it_;
        std::function<void(IteratorType)>* hook_;
    };

    /**
     * @brief Constructor to create an IteratorHook object with specified begin and end iterators and a hook function.
     *
     * @tparam HookType The type of the hook function.
     * @param begin_it The starting iterator of the range.
     * @param end_it The ending iterator of the range.
     * @param hook The hook function to be associated with the iterators.
     */
    template <class HookType>
    IteratorHook(const IteratorType& begin_it, const IteratorType& end_it, HookType&& hook)
        : begin_it_(begin_it), end_it_(end_it), hook_(std::forward<HookType>(hook)) {}

    Iterator begin()
    {
        hook_(begin_it_);
        return Iterator(begin_it_, &hook_);
    }
    Iterator end() { return Iterator(end_it_, &hook_); }

    IteratorType begin_it_;
    IteratorType end_it_;
    std::function<void(IteratorType)> hook_;
};

/**
 * @brief Function to create an IteratorHook instance for a range of iterators with a specified hook function.
 *
 * This function generates an IteratorHook instance for a given range defined by the beginning and ending iterators.
 * It associates the provided hook function with the iterators, enabling additional actions to be performed
 * during iteration through the range.
 *
 * @tparam IteratorType The type of the underlying iterators for the range.
 * @tparam HookType The type of the hook function.
 * @param begin_it The starting iterator of the range.
 * @param end_it The ending iterator of the range.
 * @param hook The hook function to be associated with the iterators.
 * @return IteratorHook<IteratorType> An IteratorHook instance managing the iteration over the specified range
 *         with the provided hook function.
 */
template <class IteratorType, class HookType>
IteratorHook<IteratorType> makeIteratorRangeHook(const IteratorType& begin_it, const IteratorType& end_it, HookType&& hook)
{
    return {begin_it, end_it, std::forward<HookType>(hook)};
}

/**
 * @brief Tqdm class to display a progress bar in the console with estimated and remaining time.
 *
 * This class represents a progress bar using ASCII characters to display the progress of an operation.
 * It tracks the progress, displays it as a percentage-based progress bar, and also calculates and displays
 * estimated and remaining time for the operation.
 */
class Tqdm {
    using TimeType = std::chrono::system_clock::time_point;

public:
    /**
     * @brief Constructor to initialize the Tqdm object with size, title, and width of the progress bar.
     *
     * @param size The total size of the operation or iterations.
     * @param title The title for the progress bar (defaults to an empty string).
     * @param mininterval The minimum interval between updates in milliseconds (defaults to 100).
     * @param width The width of the progress bar (defaults to 10).
     */
    Tqdm(size_t size, const std::string& title = "", long mininterval = 100, int width = 10) : size_{size}, title_{title}, mininterval_{mininterval}, width_{width}
    {
        reset();
    }

    /**
     * @brief Resets the progress bar state to initial values and sets the start time.
     */
    void reset()
    {
        step_ = 0;
        first_print_ = true;
        start_time_ = std::chrono::system_clock::now();
    }

    /**
     * @brief Increments the progress step, updates the current time, and returns the current step value.
     *
     * @return size_t The current step value after incrementing.
     */
    size_t step()
    {
        if (step_ < size_) {
            step_++;
            current_time_ = std::chrono::system_clock::now();
        }
        return step_;
    }

    /**
     * @brief Checks if the progress has reached the end.
     *
     * @return bool Returns true if the progress has reached the end; otherwise, false.
     */
    bool isEnd() const
    {
        return step_ >= size_;
    }

    /**
     * @brief Overloading the << operator to display the progress bar, estimated time, and remaining time.
     *
     * @param os The output stream where the progress bar will be displayed.
     * @param tqdm The Tqdm object representing the progress bar.
     * @return std::ostream& The output stream containing the progress bar and time information.
     */
    friend std::ostream& operator<<(std::ostream& os, Tqdm& tqdm)
    {
        auto current_print_time = std::chrono::system_clock::now();
        if (!tqdm.first_print_ && !tqdm.isEnd() && std::chrono::duration_cast<std::chrono::milliseconds>(current_print_time - tqdm.last_print_time_).count() < tqdm.mininterval_) {
            return os;
        }
        tqdm.last_print_time_ = current_print_time;
        // std::string output = tqdm.first_print_ ? "\0337\0338" : "\0338";
        std::string output = "\r";
        tqdm.first_print_ = false;
        double processed = double(tqdm.step_) / tqdm.size_;
        output += tqdm.title_ + " [";
        for (int i = 0; i < tqdm.width_; i++) {
            if (double(i) / tqdm.width_ <= processed) {
                output += "=";
            } else {
                output += " ";
            }
        }
        output += "]";
        output += " " + std::to_string(int(processed * 100)) + "%";
        output += " " + std::to_string(tqdm.step_) + "/" + std::to_string(tqdm.size_);
        // calculate remaining time
        static auto to_time_string = [](long milliseconds) {
            // format: HH:MM:SS
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') << milliseconds / 3600000 << ":"
               << std::setw(2) << std::setfill('0') << (milliseconds % 3600000) / 60000 << ":"
               << std::setw(2) << std::setfill('0') << (milliseconds % 60000) / 1000;
            return ss.str();
        };
        auto cost = std::chrono::duration_cast<std::chrono::milliseconds>(tqdm.current_time_ - tqdm.start_time_).count();
        auto estimated_time = tqdm.step_ ? long(int64_t(cost) * tqdm.size_ / tqdm.step_) : 0l;
        auto remaining_time = tqdm.step_ ? long(int64_t(cost) * (tqdm.size_ - tqdm.step_) / tqdm.step_) : 0l;
        auto passed_time = tqdm.step_ ? long(std::chrono::duration_cast<std::chrono::milliseconds>(tqdm.current_time_ - tqdm.start_time_).count()) : 0l;
        output += " [" + to_time_string(estimated_time) + "<" + to_time_string(passed_time) + "]";
        return os << output;
    }

private:
    size_t size_;
    std::string title_;
    long mininterval_;
    int width_;
    size_t step_;
    bool first_print_;
    TimeType start_time_;
    TimeType current_time_;
    TimeType last_print_time_;
};

/**
 * @brief Creates an IteratorHook instance with a progress indicator using Tqdm for a range of iterators.
 *
 * This function generates an IteratorHook instance to iterate over a range of iterators and display a
 * progress bar using Tqdm, indicating the progress of operations on the specified range.
 *
 * @tparam IteratorType The type of iterators used for the range.
 * @param begin_it The starting iterator of the range.
 * @param end_it The ending iterator of the range.
 * @param title The title for the progress bar (defaults to an empty string).
 * @param os The output stream where the progress bar will be displayed (defaults to std::cerr).
 * @param mininterval The minimum interval between updates in milliseconds (defaults to 100).
 * @param width The width of the progress bar (defaults to 10).
 * @return IteratorHook<IteratorType> An IteratorHook instance managing the iteration over the iterator range
 *         with a progress indicator.
 */
template <class IteratorType>
IteratorHook<IteratorType> tqdm(const IteratorType& begin_it, const IteratorType& end_it, const std::string& title = "", std::ostream& os = std::cerr, long mininterval = 100, int width = 10)
{
    size_t size = end_it - begin_it;
    Tqdm tqdm(size, title, mininterval, width);
    return makeIteratorRangeHook(begin_it, end_it, [begin_it, end_it, tqdm, &os](IteratorType it) mutable {
        if (it == end_it) {
            os << tqdm << std::endl;
            return;
        }
        os << tqdm;
        tqdm.step();
    });
}

/**
 * @brief Creates an IteratorHook instance with a progress indicator using Tqdm for a range defined by a starting iterator and size.
 *
 * This function generates an IteratorHook instance to iterate over a range defined by a starting iterator and a specified size,
 * displaying a progress bar using Tqdm, indicating the progress of operations within that range.
 *
 * @tparam IteratorType The type of iterators used for the range.
 * @param begin_it The starting iterator of the range.
 * @param size The size of the range to iterate over.
 * @param title The title for the progress bar (defaults to an empty string).
 * @param os The output stream where the progress bar will be displayed (defaults to std::cerr).
 * @param mininterval The minimum interval between updates in milliseconds (defaults to 100).
 * @param width The width of the progress bar (defaults to 10).
 * @return IteratorHook<IteratorType> An IteratorHook instance managing the iteration over the defined range
 *         with a progress indicator using Tqdm.
 */
template <class IteratorType>
IteratorHook<IteratorType> tqdm(const IteratorType& begin_it, size_t size, const std::string& title = "", std::ostream& os = std::cerr, long mininterval = 100, int width = 10)
{
    auto end_it = begin_it;
    std::advance(end_it, size);
    Tqdm tqdm(size, title, mininterval, width);
    return makeIteratorRangeHook(begin_it, end_it, [begin_it, end_it, tqdm, &os](IteratorType it) mutable {
        if (it == end_it) {
            os << tqdm << std::endl;
            return;
        }
        os << tqdm;
        tqdm.step();
    });
}

/**
 * @brief A function emulating the behavior of Python's tqdm module to display progress for containers.
 *
 * This function generates an IteratorHook instance to iterate over the elements of a container and
 * display a progress bar using Tqdm. It allows tracking progress of operations on the container.
 *
 * @tparam ContainerType The type of the container (vector, list, etc.) being iterated.
 * @param container The container over which to iterate.
 * @param title The title for the progress bar (defaults to an empty string).
 * @param os The output stream where the progress bar will be displayed (defaults to std::cerr).
 * @param mininterval The minimum interval between updates in milliseconds (defaults to 100).
 * @param width The width of the progress bar (defaults to 10).
 * @return IteratorHook<typename ContainerType::iterator> An IteratorHook instance managing the iteration
 *         over the container elements with a progress indicator.
 */
template <class ContainerType>
auto tqdm(ContainerType&& container, const std::string& title = "", std::ostream& os = std::cerr, long mininterval = 100, int width = 10)
{
    auto begin_it = container.begin(), end_it = container.end();
    size_t size = container.size();
    Tqdm tqdm(size, title, mininterval, width);
    return makeIteratorRangeHook(begin_it, end_it, [container = std::forward<ContainerType>(container), begin_it, end_it, tqdm, &os](decltype(begin_it) it) mutable {
        if (it == end_it) {
            os << tqdm << std::endl;
            return;
        }
        os << tqdm;
        tqdm.step();
    });
}
