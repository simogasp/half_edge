#include "model_io.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace half_edge {

[[nodiscard]]
bool has_valid_off_header(std::istream& off_file)
{
    std::string line;
    while(std::getline(off_file, line))
    {
        if(is_line_to_skip(line))
        {
            continue;
        }

        if(line.starts_with(OFF_HEADER)) // Check if the format is OFF
        {
            return true;
        }
        return false;

    }
    return false;
}

[[nodiscard]]
std::pair<std::size_t, std::size_t> parse_num_vertex_face(std::istream& off_file)
{
    for(std::string line; std::getline(off_file, line);)
    {
        if(is_line_to_skip(line))
        {
            continue;
        }
        long long n_vertices{0};
        long long n_faces{0};
        auto istr = std::istringstream(line);
        istr >> n_vertices >> n_faces;
        if(istr.fail())
        {
            throw std::invalid_argument("failed to parse the number of vertices and faces");
        }
        if(n_vertices <= 0 || n_faces <= 0)
        {
            throw std::invalid_argument("number of vertices and faces must be greater than 0");
        }
        return {static_cast<std::size_t>(n_vertices), static_cast<std::size_t>(n_faces)};
    }
    throw std::invalid_argument("cannot extract the number of vertices and faces");
}

[[nodiscard]] std::vector<vertex> read_vertices(std::istream& off_file, std::size_t num_vert)
{
    index idx{0};
    std::string line;
    std::vector<vertex> vertices;
    vertices.reserve(num_vert);
    while(idx < num_vert && std::getline(off_file, line))
    {
        if(is_line_to_skip(line))
        {
            continue;
        }

        double a1{.0};
        double a2{.0};
        double a3{.0};
        auto istr = std::istringstream(line);
        istr >> a1 >> a2 >> a3;
        if(istr.fail())
        {
            throw std::invalid_argument("failed to parse the vertices");
        }
        vertices.emplace_back(a1, a2);
        idx++;
    }
    return vertices;
}

void read_OFFfile(const std::string& name, std::vector<vertex>& m_vertices, std::vector<index>& faces)
{
    // Read the OFF file
    std::string line;
    std::ifstream off_file(name);
    std::string tmp;

    if(!off_file.is_open())
    {
        std::cerr << "unable to open file " << name << std::endl;
        throw std::invalid_argument("unable to open file");
    }

    // Check that the first line is an OFF file
    if(!has_valid_off_header(off_file))
    {
        std::cerr << "The file is not an OFF file" << std::endl;
        throw std::invalid_argument("The file is not an OFF file");
    }
    // Read the number of vertices and faces
    const auto& [n_vertices, n_faces] = parse_num_vertex_face(off_file);


    faces.reserve(3 * n_faces);
    // Read vertices
    m_vertices = read_vertices(off_file, n_vertices);
    // Read faces
    std::size_t idx = 0;
    while(idx < n_faces && std::getline(off_file, line))
    {
        std::istringstream(line) >> tmp;
        if(is_line_to_skip(tmp))
        {
            continue;
        }
        std::size_t length;
        std::size_t t1;
        std::size_t t2;
        std::size_t t3;
        std::istringstream(line) >> length >> t1 >> t2 >> t3;
        faces.push_back(t1);
        faces.push_back(t2);
        faces.push_back(t3);
        idx++;
    }

    off_file.close();
}
}