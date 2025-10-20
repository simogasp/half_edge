#pragma once

#include "Triangulation.hpp"

#include <algorithm>
#include <string_view>

namespace half_edge {
constexpr auto OFF_HEADER{"OFF"};
constexpr char COMMENT_CHAR = '#';

constexpr bool is_space_char(char c) noexcept
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

[[nodiscard]]
constexpr std::string_view trim_leading_whitespace(std::string_view str) noexcept
{
    const auto first_non_space = std::ranges::find_if(str, [](char c) { return c != ' ' && c != '\t'; });
    const auto distance = static_cast<size_t>(std::ranges::distance(str.begin(), first_non_space));
    str.remove_prefix(distance);
    return str;
}

/**
 * Checks if the string contains only whitespace characters or is empty.
 * @param[in] s The string view to check
 * @return true if the string contains only whitespace characters or is empty,
 *         false otherwise
 */
constexpr bool contains_only_whitespaces(std::string_view s) noexcept { return std::ranges::all_of(s, is_space_char); }

/**
 * Determines if a given string starts with a comment indicator ('#').
 * @param[in] s The string view to check.
 * @return true if the string starts with the comment character ('#'),
 *         false otherwise.
 */
constexpr bool is_comment_line(std::string_view s) noexcept
{
    const auto res = s.find_first_not_of(" \t");
    return res != std::string::npos && s[res] == COMMENT_CHAR;
}

/**
 * Determines if a given line should be skipped based on its contents.
 * A line is considered skippable if it is either a comment line (starts with '#')
 * or contains only whitespace characters.
 *
 * @param[in] s The string view representing the line to check.
 * @return true if the line should be skipped, false otherwise.
 */
constexpr bool is_line_to_skip(std::string_view s) noexcept
{
    return is_comment_line(s) || contains_only_whitespaces(s);
}

[[nodiscard]]
bool has_valid_off_header(std::istream& off_file);

[[nodiscard]]
std::pair<std::size_t, std::size_t> parse_num_vertex_face(std::istream& off_file);

[[nodiscard]] std::vector<vertex> read_vertices(std::istream& off_file, std::size_t num_vert);

[[nodiscard]]
std::array<index, 3> parse_face(const std::string& line);

[[nodiscard]]
std::vector<index> read_faces(std::istream& off_file, std::size_t n_faces);

void read_OFFfile(const std::string& name, std::vector<vertex>& m_vertices, std::vector<index>& faces);

#if defined(HE_BUILD_TESTS)
#include "helpers_test.hpp"
#endif
}