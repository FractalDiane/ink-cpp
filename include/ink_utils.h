#pragma once

#include <string>
#include <vector>

std::string strip_string_edges(const std::string& string, bool left = true, bool right = true, bool include_spaces = false) noexcept;
std::string remove_duplicate_spaces(const std::string& string) noexcept;
std::string join_string_vector(const std::vector<std::string>& vector, std::string&& delimiter) noexcept;
std::size_t randi_range(std::size_t from, std::size_t to) noexcept;
