#pragma once

#include <string>
#include <array>

/**
 * @brief Material structure representing properties from MTL files
 * 
 * This structure stores material properties commonly found in Wavefront MTL files,
 * including ambient, diffuse, and specular colors, texture maps, and other rendering properties.
 */
struct Material {
    // Material name (from "newmtl" directive)
    std::string name;
    
    // Ambient color (Ka)
    std::array<float, 3> ambient = {0.2f, 0.2f, 0.2f};
    
    // Diffuse color (Kd)
    std::array<float, 3> diffuse = {0.8f, 0.8f, 0.8f};
    
    // Specular color (Ks)
    std::array<float, 3> specular = {1.0f, 1.0f, 1.0f};
    
    // Specular exponent / shininess (Ns)
    float shininess = 32.0f;
    
    // Transparency (d or Tr)
    // d: 1.0 = fully opaque, 0.0 = fully transparent
    float opacity = 1.0f;
    
    // Optical density / Index of refraction (Ni)
    float indexOfRefraction = 1.0f;
    
    // Illumination model (illum)
    // 0 = Color on and Ambient off
    // 1 = Color on and Ambient on
    // 2 = Highlight on
    // 3 = Reflection on and Ray trace on
    // etc.
    int illuminationModel = 2;
    
    // Texture map file paths
    std::string mapAmbient;      // map_Ka - ambient texture map
    std::string mapDiffuse;      // map_Kd - diffuse texture map (most common)
    std::string mapSpecular;     // map_Ks - specular texture map
    std::string mapBump;         // map_Bump or bump - bump map
    std::string mapDisplacement; // disp - displacement map
    std::string mapAlpha;        // map_d - alpha/opacity map
    
    /**
     * @brief Default constructor creating a basic white material
     */
    Material() = default;
    
    /**
     * @brief Constructor with material name
     * @param materialName The name of the material
     */
    explicit Material(const std::string& materialName) : name(materialName) {}
};
