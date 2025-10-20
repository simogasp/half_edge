#include "model_io.hpp"


#include <catch2/catch_all.hpp>

#include <sstream>

TEST_CASE("has_valid_off_header validation", "[off_header]")
{
    struct OffHeaderTestCase
    {
        std::string name;
        std::string input;
        bool expected_result;
    };
    const std::vector<OffHeaderTestCase> test_cases = {// Valid cases
                                                       {"Simple OFF header", "OFF", true},
                                                       {"OFF header with trailing whitespace", "OFF  \t  \n", true},
                                                       {"OFF header with extended format", "OFF BINARY", true},
                                                       {"OFF header after comments",
                                                        "# Comment line 1\n"
                                                        "  # Comment line 2  \n"
                                                        "OFF\n",
                                                        true},
                                                       {"OFF header after empty lines", "\n   \nOFF", true},

                                                       // Invalid cases
                                                       {"Empty file", "", false},
                                                       {"Only whitespace", "   \n\t   ", false},
                                                       {"Only comments",
                                                        "# Comment line 1\n"
                                                        "# Comment line 2\n",
                                                        false},
                                                       {"Wrong case", "off", false},
                                                       {"Partial match", "OF", false},
                                                       {"OFF in middle of line", "Something OFF", false},
                                                       {"OFF after non-comment line",
                                                        "# Comment\n"
                                                        "Some content\n"
                                                        "OFF",
                                                        false}};

    for(const auto& [name, input, expected_result] : test_cases)
    {
        auto test_input = std::istringstream(input);
        SECTION(name) { REQUIRE(half_edge::has_valid_off_header(test_input) == expected_result); }
    }

    // Stream behavior tests still need separate sections
    SECTION("Stream manipulation")
    {
        std::istringstream input("OFF\n"
                                 "Additional content");
        REQUIRE(half_edge::has_valid_off_header(input));

        std::string next_line;
        REQUIRE(std::getline(input, next_line));
        REQUIRE(next_line == "Additional content");

        // Test multiple validations on same stream
        input.clear();
        input.seekg(0);
        REQUIRE(half_edge::has_valid_off_header(input));
    }


}

TEST_CASE("parse_num_vertex_face parsing", "[off_parser]")
{
    struct parse_num_test_cases
    {
        std::string name;
        std::string input;
        std::pair<std::size_t, std::size_t> expected;
        bool should_throw{false};
    };
    using namespace half_edge;

    const std::vector<parse_num_test_cases> test_cases = {
        // Valid cases
        {
            "Simple numbers",
            "8 12",
            {8, 12}
        },
        {
            "Numbers with extra whitespace",
            "   10    15   ",
            {10, 15}
        },
        {
            "Numbers after comments",
            "# This is a comment\n"
            "  # Another comment  \n"
            "4 6",
            {4, 6}
        },
        {
            "Numbers after empty lines",
            "\n\n   \n"
            "100 200",
            {100, 200}
        },
        {
            "Numbers with trailing data",
            "5 7 0",  // Some OFF files include number of edges
            {5, 7}
        },

        // Invalid cases
        {
            "Empty file",
            "",
            {0, 0},
            true
        },
        {
            "Only comments",
            "# Comment 1\n"
            "# Comment 2",
            {0, 0},
            true
        },
        {
            "Only whitespace",
            "    \n\t    ",
            {0, 0},
            true
        },
        {
            "Single number",
            "42",
            {0, 0},
            true
        },
        {
            "Invalid characters",
            "abc def",
            {0, 0},
            true
        },
        {
            "Negative numbers",
            "-5 -10",
            {0, 0},  // Will actually read as 0 due to unsigned conversion
            true
        }
    };

    for (const auto& test_case : test_cases) {
        SECTION(test_case.name) {
            std::istringstream input(test_case.input);

            if (test_case.should_throw) {
                REQUIRE_THROWS_AS(parse_num_vertex_face(input), std::invalid_argument);
            } else {
                const auto& [vert, faces] = parse_num_vertex_face(input);
                REQUIRE(vert == test_case.expected.first);
                REQUIRE(faces == test_case.expected.second);
            }
        }
    }

    // Stream position tests
    SECTION("Stream position after parsing") {
        std::istringstream input(
            "8 12\n"
            "Additional content"
        );
        const auto& [vert, faces] = parse_num_vertex_face(input);
        REQUIRE(vert == 8);
        REQUIRE(faces == 12);

        std::string next_line;
        REQUIRE(std::getline(input, next_line));
        REQUIRE(next_line == "Additional content");
    }

    // Multiple calls test
    SECTION("Multiple parse attempts on same stream") {
        std::istringstream input(
            "# Comment\n"
            "8 12\n"
            "Additional content"
        );
        const auto& [vert, faces] = parse_num_vertex_face(input);
        REQUIRE(vert == 8);
        REQUIRE(faces == 12);

        // Second parse should fail as we've consumed the numbers
        REQUIRE_THROWS_AS(parse_num_vertex_face(input), std::invalid_argument);
    }
}



TEST_CASE("read_vertices function", "[model_io]") {

    struct VertexTestCase {
        std::string name;
        std::string input;
        std::size_t num_vertices;
        std::vector<half_edge::vertex> expected;
        bool should_throw;
    };

    const std::vector<VertexTestCase> test_cases = {
        {
            "Valid vertices",
            "1.0 2.0 3.0\n4.0 5.0 6.0\n7.0 8.0 9.0",
            3u,
            {{1.0, 2.0}, {4.0, 5.0}, {7.0, 8.0}},
            false
        },
        {
            "With comments and empty lines",
            "# Comment\n\n1.0 2.0 3.0\n\n# Another comment\n4.0 5.0 6.0",
            2u,
            {{1.0, 2.0}, {4.0, 5.0}},
            false
        },
        {
            "Invalid format - missing coordinate",
            "1.0 2.0\n4.0 5.0 6.0",
            2u,
            {},
            true
        },
        {
            "Invalid format - non-numeric input",
            "1.0 2.0 3.0\nabc def ghi",
            2u,
            {},
            true
        },
        {
            "Zero vertices requested",
            "1.0 2.0 3.0",
            0u,
            {},
            false
        },
        {
            "More input than requested vertices",
            "1.0 2.0 3.0\n4.0 5.0 6.0\n7.0 8.0 9.0",
            2u,
            {{1.0, 2.0}, {4.0, 5.0}},
            false
        },
        {
            "Fewer vertices than requested",
            "1.0 2.0 3.0\n4.0 5.0 6.0",
            3u,
            {{1.0, 2.0}, {4.0, 5.0}},
            false
        },
        {
            "Scientific notation",
            "1.0e1 2.0e-1 3.0\n-4.0e2 5.0e+1 6.0",
            2u,
            {{10.0, 0.2}, {-400.0, 50.0}},
            false
        }
    };

    for (const auto& [name, input_str, num_vertices, expected, should_throw] : test_cases) {
        SECTION(name) {
            std::istringstream input(input_str);
            if (should_throw) {
                REQUIRE_THROWS_AS(half_edge::read_vertices(input, num_vertices),
                                std::invalid_argument);
            } else {
                auto result = half_edge::read_vertices(input, num_vertices);
                REQUIRE(result.size() == expected.size());
                for (size_t i = 0; i < result.size(); ++i) {
                    REQUIRE(result[i].x == Catch::Approx(expected[i].x));
                    REQUIRE(result[i].y == Catch::Approx(expected[i].y));
                }
            }
        }
    }
}


TEST_CASE("parse_face function tests", "[parse_face]")
{
    using face_t = std::array<half_edge::index, 3>;
    struct ParseFaceTestCase {
        std::string input;
        std::optional<face_t> expected_result;

        ParseFaceTestCase(std::string in, const std::optional<face_t>& exp)
            : input(std::move(in)), expected_result(exp) {}
    };

    const std::vector<ParseFaceTestCase> test_cases{
        // Valid cases
            {"0 1 2", face_t{0, 1, 2}},
            {"10 20 30", face_t{10, 20, 30}},
            {"999 888 777", face_t{999, 888, 777}},
            {"1     2     3", face_t{1, 2, 3}},  // Extra spaces

            // Invalid cases
            {"", std::nullopt},  // Empty string
            {"3", std::nullopt}, // Incomplete data
            {"3 1 2 3", std::nullopt}, // Extra number, something fishy
            {"a b c", std::nullopt}, // Non-numeric input
            {"-1 2 3", std::nullopt}, // Negative index
            {"1.5 2 3", std::nullopt}, // Floating point numbers
            {"invalid", std::nullopt}, // Invalid format
            {"3 1 abc", std::nullopt}, // Mixed valid/invalid data
        };

    for (const auto& test_case : test_cases)
    {
        SECTION("Input: '" + test_case.input + "'")
        {
            if (test_case.expected_result)
            {
                // Test cases where we expect successful parsing
                const auto result = half_edge::parse_face(test_case.input);
                REQUIRE(result == *test_case.expected_result);
            }
            else
            {
                // Test cases where we expect exceptions
                REQUIRE_THROWS_AS(half_edge::parse_face(test_case.input), std::invalid_argument);
            }
        }
    }
}


