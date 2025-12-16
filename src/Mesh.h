#pragma once

#include <vector>
#include <string>
#include <array>
#include <map>
#include "Material.h"

/**
 * @brief Vertex structure containing all possible vertex attributes
 */
struct Vertex {
    std::array<float, 3> position = {0.0f, 0.0f, 0.0f};    // x, y, z coordinates
    std::array<float, 3> normal = {0.0f, 1.0f, 0.0f};      // normal vector (nx, ny, nz)
    std::array<float, 2> texCoords = {0.0f, 0.0f};         // texture coordinates (u, v)
    std::array<float, 3> color = {1.0f, 1.0f, 1.0f};       // optional vertex color (r, g, b)
};

/**
 * @brief Face structure representing a triangle with material information
 */
struct Face {
    std::array<unsigned int, 3> indices;  // Indices of the three vertices forming this triangle
    std::string materialName;              // Name of the material used for this face
};

/**
 * @brief Mesh class for storing and managing 3D geometry data
 * 
 * This class manages vertices, faces (triangles), and materials.
 * It supports vertex attributes like position, normal, texture coordinates, and optional colors.
 * Each face can reference a different material, allowing for multi-material meshes.
 */
class Mesh {
private:
    std::vector<Vertex> m_Vertices;
    std::vector<Face> m_Faces;
    std::map<std::string, Material> m_Materials;
    std::string m_Name;

public:
    /**
     * @brief Default constructor
     */
    Mesh() = default;

    /**
     * @brief Constructor with mesh name
     * @param name The name of the mesh
     */
    explicit Mesh(const std::string& name) : m_Name(name) {}

    /**
     * @brief Add a vertex to the mesh
     * @param vertex The vertex to add
     * @return Index of the added vertex
     */
    unsigned int AddVertex(const Vertex& vertex);

    /**
     * @brief Add a face to the mesh
     * @param face The face to add
     */
    void AddFace(const Face& face);

    /**
     * @brief Add a material to the mesh's material library
     * @param material The material to add
     */
    void AddMaterial(const Material& material);

    /**
     * @brief Get a material by name
     * @param name The name of the material
     * @return Pointer to the material if found, nullptr otherwise
     */
    const Material* GetMaterial(const std::string& name) const;

    /**
     * @brief Get all vertices
     * @return Reference to the vector of vertices
     */
    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }

    /**
     * @brief Get all faces
     * @return Reference to the vector of faces
     */
    const std::vector<Face>& GetFaces() const { return m_Faces; }

    /**
     * @brief Get all materials
     * @return Reference to the map of materials
     */
    const std::map<std::string, Material>& GetMaterials() const { return m_Materials; }

    /**
     * @brief Get mesh name
     * @return The name of the mesh
     */
    const std::string& GetName() const { return m_Name; }

    /**
     * @brief Set mesh name
     * @param name The new name for the mesh
     */
    void SetName(const std::string& name) { m_Name = name; }

    /**
     * @brief Clear all mesh data
     */
    void Clear();

    /**
     * @brief Get vertex count
     * @return Number of vertices in the mesh
     */
    size_t GetVertexCount() const { return m_Vertices.size(); }

    /**
     * @brief Get face count
     * @return Number of faces (triangles) in the mesh
     */
    size_t GetFaceCount() const { return m_Faces.size(); }

    /**
     * @brief Get material count
     * @return Number of materials in the mesh
     */
    size_t GetMaterialCount() const { return m_Materials.size(); }

    /**
     * @brief Get flattened vertex data for OpenGL (position, normal, texcoord interleaved)
     * @return Vector of floats in format: [px, py, pz, nx, ny, nz, u, v, ...]
     */
    std::vector<float> GetInterleavedVertexData() const;

    /**
     * @brief Get index data for indexed rendering
     * @return Vector of indices for triangle rendering
     */
    std::vector<unsigned int> GetIndexData() const;
};
