#pragma once

#include <string>
#include <memory>
#include "Texture.h"

struct Material {
    std::string name;
    
    // Colors (default to white)
    float ambient[3] = {1.0f, 1.0f, 1.0f};
    float diffuse[3] = {1.0f, 1.0f, 1.0f};
    float specular[3] = {1.0f, 1.0f, 1.0f};
    
    float specularExponent = 32.0f;
    float opacity = 1.0f;
    
    // Texture maps
    std::string diffuseTexturePath;
    std::shared_ptr<Texture> diffuseTexture;
    
    Material() = default;
    Material(const std::string& n) : name(n) {}
};
