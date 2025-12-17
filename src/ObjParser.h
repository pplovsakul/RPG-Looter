#pragma once

#include <string>
#include <vector>
#include <map>
#include "Material.h"
#include "Mesh.h"

/**
 * @brief Parser for Wavefront OBJ and MTL files
 * 
 * This class provides functionality to parse .obj and .mtl files
 * and populate Mesh objects with geometry and material data.
 * No external libraries like tinyobjloader or assimp are used.
 */
class ObjParser {
public:
    /**
     * @brief Parse an MTL file and return materials
     * @param filepath Path to the .mtl file
     * @param materials Output map of material name to Material object
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseMTL(const std::string& filepath, std::map<std::string, Material>& materials);

    /**
     * @brief Parse an OBJ file and populate a Mesh
     * @param filepath Path to the .obj file
     * @param mesh Output mesh to populate with geometry and materials
     * @param loadMaterials If true, automatically load referenced MTL files
     * @return true if parsing succeeded, false otherwise
     */
    static bool ParseOBJ(const std::string& filepath, Mesh& mesh, bool loadMaterials = true);

private:
    // Helper structures for OBJ parsing
    struct ObjVertex {
        int positionIndex = -1;
        int texCoordIndex = -1;
        int normalIndex = -1;
    };

    /**
     * @brief Parse a single line from an MTL file
     * @param line The line to parse
     * @param currentMaterial Pointer to the current material being constructed
     * @param materials Map of all materials
     */
    static void ParseMTLLine(const std::string& line, Material** currentMaterial, 
                            std::map<std::string, Material>& materials);

    /**
     * @brief Parse a vertex face definition (e.g., "v/vt/vn")
     * @param token The token to parse
     * @return ObjVertex structure with parsed indices
     */
    static ObjVertex ParseVertexToken(const std::string& token);

    /**
     * @brief Get the directory path from a file path
     * @param filepath Full file path
     * @return Directory path (without filename)
     */
    static std::string GetDirectory(const std::string& filepath);

    /**
     * @brief Trim whitespace from both ends of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string Trim(const std::string& str);

    /**
     * @brief Split a string by whitespace
     * @param str String to split
     * @return Vector of tokens
     */
    static std::vector<std::string> Split(const std::string& str);
};
