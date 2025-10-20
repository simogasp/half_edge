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

[[nodiscard]]
std::array<index, 3> parse_face(const std::string& line)
{
    std::istringstream iss(line);
    std::array<index, 3> face{};

    for(auto& vertex_idx : face)
    {
        long long tmp;
        iss >> tmp;
        if(iss.fail())
        {
            throw std::invalid_argument("failed to parse the faces: " + line);
        }
        if(tmp < 0)
        {
            throw std::invalid_argument("face indices must be non-negative: " + line);
        }
        vertex_idx = static_cast<index>(tmp);
    }
    // if there are still characters in the stream, it means that the input is not good
    if(!iss.eof())
    {
            throw std::invalid_argument("invalid input string: " + line);
    }
    return face;
}

[[nodiscard]]
std::vector<index> read_faces(std::istream& off_file, std::size_t n_faces)
{
    std::vector<index> faces;
    faces.reserve(3 * n_faces);

    std::string line;
    std::size_t processed_faces = 0;

    while(processed_faces < n_faces && std::getline(off_file, line))
    {
        if(is_line_to_skip(line))
        {
            continue;
        }

        const auto face = parse_face(line);
        std::ranges::copy(face, std::back_inserter(faces));
        ++processed_faces;
    }

    return faces;
}

void read_OFFfile(const std::string& name, std::vector<vertex>& m_vertices, std::vector<index>& faces)
{
    // Read the OFF file
    std::ifstream off_file(name);

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

    // Read vertices
    m_vertices = read_vertices(off_file, n_vertices);
    // Read faces
    faces = read_faces(off_file, n_faces);

    off_file.close();
}
}