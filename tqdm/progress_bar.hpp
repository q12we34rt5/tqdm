#pragma once

#include <iostream>
#include <string>
#include <vector>

/**
 * @brief A class representing a customizable progress bar.
 *
 * The ProgressBar class creates and manages a visual progress bar with customizable width and display patterns.
 * It allows users to set the width, display patterns, and percentage of completion.
 */
class ProgressBar {
public:
    /**
     * @brief Constructor for the ProgressBar class.
     *
     * @param width The width of the progress bar.
     * @param patterns Vector of strings representing the display patterns.
     *                 Default patterns include spaces and various Unicode block elements.
     */
    ProgressBar(int width, std::vector<std::string> patterns = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"})
        : width_(width), patterns_(std::move(patterns)) {}

    void setWidth(int width) { width_ = width; }
    int getWidth() const { return width_; }
    void setPatterns(std::vector<std::string> patterns) { patterns_ = std::move(patterns); }
    const std::vector<std::string>& getPatterns() const { return patterns_; }
    void setPercentage(float percentage) { percentage_ = percentage; }
    float getPercentage() const { return percentage_; }

    std::string toString() const
    {
        if (width_ <= 0) return {};
        std::string bar;
        int pattern_num = patterns_.size() - 1;
        int num = (percentage_ - 1e-5) * width_ * pattern_num;
        for (int i = 0; i < num / pattern_num; ++i) {
            bar += patterns_.back();
        }
        bar += patterns_[num % pattern_num + 1];
        for (int i = 0; i < width_ - num / pattern_num - 1; ++i) {
            bar += patterns_.front();
        }
        return bar;
    }

    friend std::ostream& operator<<(std::ostream& os, const ProgressBar& bar) { return os << bar.toString(); }

private:
    int width_;
    std::vector<std::string> patterns_;
    float percentage_;
};
