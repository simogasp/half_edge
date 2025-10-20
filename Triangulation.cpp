#include "Triangulation.hpp"
#include "model_io.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace half_edge {

// Read the mesh from a file in OFF format
std::vector<index> Triangulation::read_OFFfile(const std::string& name)
{
    // Read the OFF file
    std::vector<index> faces;
    std::string line;
    std::ifstream off_file(name);
    std::string tmp;
    if(!off_file.is_open())
    {
        std::cerr << "unable to open file " << name << std::endl;
        throw std::runtime_error("unable to open file");
    }

    // Check first line is a OFF file
    while(std::getline(off_file, line))
    {
        // add check boundary vertices flag
        std::istringstream(line) >> tmp;
        if(tmp[0] != '#' && !contains_only_whitespaces(line))
        {
            if(tmp.starts_with("OFF")) // Check if the format is OFF
                break;
            else
            {
                std::cout << "The file is not an OFF file" << std::endl;
                exit(0);
            }
        }
    }
    // Read the number of vertices and faces
    while(std::getline(off_file, line))
    {
        // add check boundary vertices flag
        std::istringstream(line) >> tmp;
        if(tmp[0] != '#' && !contains_only_whitespaces(line))
        {
            std::istringstream(line) >> this->n_vertices >> this->n_faces;
            this->m_vertices.reserve(this->n_vertices);
            faces.reserve(3 * this->n_faces);
            break;
        }
    }
    // Read vertices
    index idx{0};
    while(idx < n_vertices && std::getline(off_file, line))
    {
        std::istringstream(line) >> tmp;
        if(tmp[0] != '#' && !contains_only_whitespaces(line))
        {
            double a1;
            double a2;
            double a3;
            std::istringstream(line) >> a1 >> a2 >> a3;
            vertex ve;
            ve.x = a1;
            ve.y = a2;
            this->m_vertices.push_back(ve);
            idx++;
        }
    }
    // Read faces
    idx = 0;
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
    return faces;
}

Triangulation::Triangulation(const std::string& OFF_file)
{
    std::cout << "Reading OFF file " << OFF_file << std::endl;
    std::vector<index> faces = read_OFFfile(OFF_file);
    construct_interior_halfEdges_from_faces(faces);
    construct_exterior_halfEdges();
}

// Generate interior halfedges using a vector with the faces of the triangulation
// if an interior half-edge is border, it is mark as border-edge
// mark border-edges
void Triangulation::construct_interior_halfEdges_from_faces(const std::vector<index>& faces)
{
    auto hash_for_pair = [](const _edge& p) { return std::hash<index>{}(p.first) ^ std::hash<index>{}(p.second); };

    // set of edges to calculate the boundary and twin edges
    std::unordered_map<_edge, index, decltype(hash_for_pair)> map_edges(3 * this->n_faces, hash_for_pair);

    for(std::size_t i = 0; i < n_faces; i++)
    {
        for(std::size_t j = 0; j < 3; j++)
        {
            half_edge he{};
            const auto v_origin = faces.at(3 * i + j);
            const auto v_target = faces.at(3 * i + (j + 1) % 3);
            he.origin = v_origin;
            he.next = i * 3 + (j + 1) % 3;
            he.prev = i * 3 + (j + 2) % 3;
            he.is_border = false;
            he.twin = NOT_A_TWIN;
            m_vertices.at(v_origin).incident_halfedge = i * 3 + j;
            map_edges[{v_origin, v_target}] = i * 3 + j;
            m_half_edges.push_back(he);
        }
    }

    // Calculate twin halfedge and boundary halfedges from set_edges
    for(std::size_t i = 0; i < m_half_edges.size(); ++i)
    {
        // if halfedge has already a  twin skip
        if(m_half_edges.at(i).twin != NOT_A_TWIN)
        {
            continue;
        }
        const auto tgt = origin(next(i));
        const auto org = origin(i);
        const _edge twin = {tgt, org};
        // if twin is found
        if(const auto it = map_edges.find(twin); it != map_edges.end())
        {
            const auto index_twin = it->second;
            m_half_edges.at(i).twin = index_twin;
            m_half_edges.at(index_twin).twin = i;
        }
        else
        {
            // if twin is not found the halfedge is on the boundary
            m_half_edges.at(i).is_border = true;
            m_vertices.at(org).is_border = true;
            m_vertices.at(tgt).is_border = true;
        }
    }
}

// Generate exterior half edges
// This takes  n + k time where n is the number of vertices and k is the number of border edges
void Triangulation::construct_exterior_halfEdges()
{
    // search interior edges labed as border, generates exterior edges
    // with the origin and target inverted and add at the of HalfEdges vector
    // std::cout<<"Size vector: "<<HalfEdges.size()<<std::endl;
    this->n_half_edges = m_half_edges.size();

    std::size_t border_count = 0;
    for (std::size_t i = 0; i < this->n_half_edges; ++i)
    {
        if (m_half_edges.at(i).is_border) border_count++;
    }
    m_half_edges.reserve(m_half_edges.size() + border_count);

    // @FIXME there might be a problem with pushing back and indices as the vector grows and be reallocated
    for(std::size_t i = 0; i < this->n_half_edges; ++i)
    {
        if(m_half_edges.at(i).is_border)
        {
            half_edge he_aux{};
            he_aux.twin = i;
            he_aux.origin = origin(next(i));
            he_aux.is_border = true;
            m_half_edges.at(i).is_border = false;

            m_half_edges.push_back(he_aux);
            m_half_edges.at(i).twin = m_half_edges.size() - 1;
        }
    }
    // traverse the exterior edges and search their next prev halfedge
    index nxtCCW, prvCCW;
    for(std::size_t i = n_half_edges; i < m_half_edges.size(); ++i)
    {
        if(m_half_edges.at(i).is_border)
        {
            nxtCCW = CCW_edge_to_vertex(m_half_edges.at(i).twin);
            while(!m_half_edges.at(nxtCCW).is_border)
                nxtCCW = this->CCW_edge_to_vertex(nxtCCW);
            m_half_edges.at(i).next = nxtCCW;

            prvCCW = this->next(twin(i));
            while(!m_half_edges.at(m_half_edges.at(prvCCW).twin).is_border)
                prvCCW = this->CW_edge_to_vertex(prvCCW);
            m_half_edges.at(i).prev = m_half_edges.at(prvCCW).twin;
        }
    }
    this->n_half_edges = m_half_edges.size();
}

// Given an edge with vertex origin v, return the next counterclockwise edge of v with v as origin
// Input: e is the edge
// Output: the next counterclockwise edge of v
index Triangulation::CCW_edge_to_vertex(index e) const
{
    const auto prv = m_half_edges.at(e).prev;
    return m_half_edges.at(prv).twin;
}

// Given an edge with vertex origin v, return the prev clockwise edge of v with v as origin
// Input: e is the edge
// Output: the prev clockwise edge of v
index Triangulation::CW_edge_to_vertex(index e) const
{
    const auto twn = m_half_edges.at(e).twin;
    return m_half_edges.at(twn).next;
}




}