#pragma once

#include <string>
#include <vector>
#include <random>

std::string strip_string_edges(const std::string& string, bool left = true, bool right = true, bool include_spaces = false) noexcept;
std::string remove_duplicate_spaces(const std::string& string) noexcept;
std::string join_string_vector(const std::vector<std::string>& vector, std::string&& delimiter) noexcept;
std::vector<std::string> split_string(const std::string& string, char delimiter, bool ignore_delim_spaces, bool paren_arguments = false) noexcept;
std::int64_t randi_range(std::int64_t from, std::int64_t to, std::mt19937& generator) noexcept;
