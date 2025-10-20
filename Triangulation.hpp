#pragma once

#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace half_edge {
using index = std::size_t;
using _edge = std::pair<index, index>;

constexpr auto NOT_A_TWIN = std::numeric_limits<std::size_t>::max();



struct vertex
{
    double x{};
    double y{};
    /// whether the vertex is on the boundary
    bool is_border{false};
    /// halfedge incident to the vertex, vertex is the origin of the halfedge
    index incident_halfedge{};

    vertex() = default;

    vertex(double x_coord, double y_coord)
        : x(x_coord), y(y_coord) {}
};

struct half_edge
{
    /// tail of edge
    index origin{};
    /// opposite halfedge
    index twin{NOT_A_TWIN};
    /// next halfedge of the same face
    index next{};
    /// previous halfedge of the same face
    index prev{};
    /// whether the halfedge is on the boundary
    bool is_border{false};

    // Default constructor
    half_edge() = default;

    // Copy constructor
    half_edge(const half_edge&) = default;

    // Copy assignment operator
    half_edge& operator=(const half_edge&) = default;

    // If you're using C++11 or later, you might also want to add:
    // Move constructor
    half_edge(half_edge&&) noexcept = default;

    // Move assignment operator
    half_edge& operator=(half_edge&&) noexcept = default;

};

class Triangulation
{
  private:
    /// number of halfedges
    std::size_t n_half_edges{0};
    /// number of faces
    std::size_t n_faces{0};
    /// number of vertices
    std::size_t n_vertices{0};
    /// number of border edges
    std::size_t n_border_edges{0};
    /// time to generate the triangulation
    double t_triangulation_generation{0};

    /// AoS of vertices
    std::vector<vertex> m_vertices{};
    /// AoS of half-edges
    std::vector<half_edge> m_half_edges{};

  public:
    explicit Triangulation(const std::string& OFF_file);

    std::vector<index> read_OFFfile(const std::string& name);

    void construct_interior_halfEdges_from_faces(const std::vector<index>& faces);

    void construct_exterior_halfEdges();

    [[nodiscard]] auto faces_size() const { return n_faces; }
    [[nodiscard]] auto halfEdges_size() const { return n_half_edges; };
    [[nodiscard]] auto vertices_size() const { return n_vertices; };

    // Calculates the tail vertex of the edge e
    // Input: e is the edge
    // Output: the tail vertex v of the edge e
    [[nodiscard]] auto origin(index e) const { return m_half_edges.at(e).origin; }

    // Calculates the head vertex of the edge e
    // Input: e is the edge
    // Output: the head vertex v of the edge e
    [[nodiscard]] auto target(index e) const { return this->origin(m_half_edges.at(e).twin); }

    // Return the twin edge of the edge e
    // Input: e is the edge
    // Output: the twin edge of e
    [[nodiscard]] auto twin(index e) const { return m_half_edges.at(e).twin; }

    // Calculates the next edge of the face incident to edge e
    // Input: e is the edge
    // Output: the next edge of the face incident to e
    [[nodiscard]] auto next(index e) const { return m_half_edges.at(e).next; }

    // Return the twin edge of the edge e
    // Input: e is the edge
    // Output: the twin edge of e
    [[nodiscard]] auto prev(index e) const { return m_half_edges.at(e).prev; }

    // return a edge associate to the node v
    // Input: v is the node
    // Output: the edge associate to the node v
    [[nodiscard]] auto edge_of_vertex(index v) const { return m_vertices.at(v).incident_halfedge; }

    [[nodiscard]] index CCW_edge_to_vertex(index e) const;

    [[nodiscard]] index CW_edge_to_vertex(index e) const;

    // Input: edge e
    // Output: true if is the face of e is border face
    //         false otherwise
    [[nodiscard]] bool is_border_face(index e) { return m_half_edges.at(e).is_border; }

    bool is_border_vertex(index v);

    int degree(index v);

    int incident_halfedge(index f);

    // return the x coordinate of the vertex v
    [[nodiscard]] auto get_PointX(index v) const { return m_vertices.at(v).x; }
    // return the y coordinate of the vertex v
    [[nodiscard]] auto get_PointY(index v) const { return m_vertices.at(v).y; }
};


}
