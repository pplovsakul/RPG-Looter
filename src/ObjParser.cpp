#include "ObjParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

bool ObjParser::ParseMTL(const std::string& filepath, std::map<std::string, Material>& materials) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << filepath << std::endl;
        return false;
    }

    Material* currentMaterial = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        line = Trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        ParseMTLLine(line, &currentMaterial, materials);
    }

    file.close();
    return true;
}

void ObjParser::ParseMTLLine(const std::string& line, Material** currentMaterial, 
                             std::map<std::string, Material>& materials) {
    std::istringstream iss(line);
    std::string keyword;
    iss >> keyword;

    if (keyword == "newmtl") {
        // Start a new material
        std::string name;
        iss >> name;
        materials[name] = Material(name);
        *currentMaterial = &materials[name];
    }
    else if (*currentMaterial != nullptr) {
        // Parse material properties
        if (keyword == "Ka") {
            // Ambient color
            iss >> (*currentMaterial)->ambient[0] >> (*currentMaterial)->ambient[1] >> (*currentMaterial)->ambient[2];
        }
        else if (keyword == "Kd") {
            // Diffuse color
            iss >> (*currentMaterial)->diffuse[0] >> (*currentMaterial)->diffuse[1] >> (*currentMaterial)->diffuse[2];
        }
        else if (keyword == "Ks") {
            // Specular color
            iss >> (*currentMaterial)->specular[0] >> (*currentMaterial)->specular[1] >> (*currentMaterial)->specular[2];
        }
        else if (keyword == "Ns") {
            // Shininess
            iss >> (*currentMaterial)->shininess;
        }
        else if (keyword == "d") {
            // Opacity
            iss >> (*currentMaterial)->opacity;
        }
        else if (keyword == "Tr") {
            // Transparency (inverse of opacity)
            float transparency;
            iss >> transparency;
            (*currentMaterial)->opacity = 1.0f - transparency;
        }
        else if (keyword == "Ni") {
            // Index of refraction
            iss >> (*currentMaterial)->indexOfRefraction;
        }
        else if (keyword == "illum") {
            // Illumination model
            iss >> (*currentMaterial)->illuminationModel;
        }
        else if (keyword == "map_Ka") {
            // Ambient texture map
            iss >> (*currentMaterial)->mapAmbient;
        }
        else if (keyword == "map_Kd") {
            // Diffuse texture map
            iss >> (*currentMaterial)->mapDiffuse;
        }
        else if (keyword == "map_Ks") {
            // Specular texture map
            iss >> (*currentMaterial)->mapSpecular;
        }
        else if (keyword == "map_Bump" || keyword == "bump") {
            // Bump map
            iss >> (*currentMaterial)->mapBump;
        }
        else if (keyword == "disp") {
            // Displacement map
            iss >> (*currentMaterial)->mapDisplacement;
        }
        else if (keyword == "map_d") {
            // Alpha map
            iss >> (*currentMaterial)->mapAlpha;
        }
    }
}

bool ObjParser::ParseOBJ(const std::string& filepath, Mesh& mesh, bool loadMaterials) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        return false;
    }

    // Temporary storage for OBJ data
    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 2>> texCoords;
    std::vector<std::array<float, 3>> normals;
    
    std::string currentMaterial;
    std::string line;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        line = Trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "v") {
            // Vertex position
            std::array<float, 3> pos;
            iss >> pos[0] >> pos[1] >> pos[2];
            positions.push_back(pos);
        }
        else if (keyword == "vt") {
            // Texture coordinate
            std::array<float, 2> tex;
            iss >> tex[0] >> tex[1];
            texCoords.push_back(tex);
        }
        else if (keyword == "vn") {
            // Vertex normal
            std::array<float, 3> norm;
            iss >> norm[0] >> norm[1] >> norm[2];
            normals.push_back(norm);
        }
        else if (keyword == "f") {
            // Face definition
            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token) {
                tokens.push_back(token);
            }

            // Handle triangulation for faces with more than 3 vertices
            // Convert polygon to triangle fan
            for (size_t i = 1; i + 1 < tokens.size(); ++i) {
                ObjVertex v0 = ParseVertexToken(tokens[0]);
                ObjVertex v1 = ParseVertexToken(tokens[i]);
                ObjVertex v2 = ParseVertexToken(tokens[i + 1]);

                // Create vertices for each face vertex
                Vertex vertex0, vertex1, vertex2;

                // Set positions
                if (v0.positionIndex >= 0 && v0.positionIndex < static_cast<int>(positions.size())) {
                    vertex0.position = positions[v0.positionIndex];
                }
                if (v1.positionIndex >= 0 && v1.positionIndex < static_cast<int>(positions.size())) {
                    vertex1.position = positions[v1.positionIndex];
                }
                if (v2.positionIndex >= 0 && v2.positionIndex < static_cast<int>(positions.size())) {
                    vertex2.position = positions[v2.positionIndex];
                }

                // Set texture coordinates
                if (v0.texCoordIndex >= 0 && v0.texCoordIndex < static_cast<int>(texCoords.size())) {
                    vertex0.texCoords = texCoords[v0.texCoordIndex];
                }
                if (v1.texCoordIndex >= 0 && v1.texCoordIndex < static_cast<int>(texCoords.size())) {
                    vertex1.texCoords = texCoords[v1.texCoordIndex];
                }
                if (v2.texCoordIndex >= 0 && v2.texCoordIndex < static_cast<int>(texCoords.size())) {
                    vertex2.texCoords = texCoords[v2.texCoordIndex];
                }

                // Set normals
                if (v0.normalIndex >= 0 && v0.normalIndex < static_cast<int>(normals.size())) {
                    vertex0.normal = normals[v0.normalIndex];
                }
                if (v1.normalIndex >= 0 && v1.normalIndex < static_cast<int>(normals.size())) {
                    vertex1.normal = normals[v1.normalIndex];
                }
                if (v2.normalIndex >= 0 && v2.normalIndex < static_cast<int>(normals.size())) {
                    vertex2.normal = normals[v2.normalIndex];
                }

                // Add vertices to mesh and get their indices
                unsigned int idx0 = mesh.AddVertex(vertex0);
                unsigned int idx1 = mesh.AddVertex(vertex1);
                unsigned int idx2 = mesh.AddVertex(vertex2);

                // Create face
                Face face;
                face.indices = {idx0, idx1, idx2};
                face.materialName = currentMaterial;
                mesh.AddFace(face);
            }
        }
        else if (keyword == "usemtl") {
            // Use material
            iss >> currentMaterial;
        }
        else if (keyword == "mtllib" && loadMaterials) {
            // Material library
            std::string mtlFile;
            iss >> mtlFile;
            
            // Construct path to MTL file (in same directory as OBJ)
            std::string directory = GetDirectory(filepath);
            std::string mtlPath = directory.empty() ? mtlFile : directory + "/" + mtlFile;
            
            // Parse MTL file and add materials to mesh
            std::map<std::string, Material> materials;
            if (ParseMTL(mtlPath, materials)) {
                for (const auto& pair : materials) {
                    mesh.AddMaterial(pair.second);
                }
            }
        }
        else if (keyword == "o" || keyword == "g") {
            // Object name or group name
            std::string name;
            iss >> name;
            if (mesh.GetName().empty()) {
                mesh.SetName(name);
            }
        }
    }

    file.close();
    return true;
}

ObjParser::ObjVertex ObjParser::ParseVertexToken(const std::string& token) {
    ObjVertex vertex;
    
    size_t firstSlash = token.find('/');
    size_t secondSlash = token.find('/', firstSlash + 1);
    
    // Parse position index (always present)
    if (firstSlash == std::string::npos) {
        // Format: v
        vertex.positionIndex = std::stoi(token) - 1; // OBJ indices are 1-based
    }
    else {
        vertex.positionIndex = std::stoi(token.substr(0, firstSlash)) - 1;
        
        // Parse texture coordinate index
        if (secondSlash == std::string::npos) {
            // Format: v/vt
            if (firstSlash + 1 < token.length()) {
                vertex.texCoordIndex = std::stoi(token.substr(firstSlash + 1)) - 1;
            }
        }
        else {
            // Format: v/vt/vn or v//vn
            if (secondSlash > firstSlash + 1) {
                vertex.texCoordIndex = std::stoi(token.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
            }
            
            // Parse normal index
            if (secondSlash + 1 < token.length()) {
                vertex.normalIndex = std::stoi(token.substr(secondSlash + 1)) - 1;
            }
        }
    }
    
    return vertex;
}

std::string ObjParser::GetDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(0, lastSlash);
    }
    return "";
}

std::string ObjParser::Trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    
    return str.substr(start, end - start);
}

std::vector<std::string> ObjParser::Split(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}
