#include <iostream>
#include "../src/OBJLoader.h"

int main() {
    std::cout << "=== OBJ/MTL Texture Loading Test ===" << std::endl;
    
    // Test loading the OBJ with MTL
    OBJLoader::MeshData mesh;
    bool success = OBJLoader::LoadOBJ("res/models/Test.obj", mesh);
    
    if (!success) {
        std::cerr << "FAILED: Could not load Test.obj" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "✓ OBJ file loaded successfully" << std::endl;
    std::cout << "  Vertices: " << mesh.vertices.size() / 5 << " (5 floats per vertex: x,y,z,u,v)" << std::endl;
    std::cout << "  Indices: " << mesh.indices.size() << std::endl;
    std::cout << "  Triangles: " << mesh.indices.size() / 3 << std::endl;
    std::cout << "  Has UVs: " << (mesh.hasTexCoords ? "Yes" : "No") << std::endl;
    
    // Check materials
    if (mesh.materials.empty()) {
        std::cerr << "✗ No materials loaded from MTL file" << std::endl;
        return 1;
    }
    
    std::cout << "✓ Materials loaded: " << mesh.materials.size() << std::endl;
    
    // Check each material
    for (const auto& matPair : mesh.materials) {
        const Material& mat = matPair.second;
        std::cout << "\nMaterial: " << mat.name << std::endl;
        std::cout << "  Diffuse color: (" << mat.diffuse[0] << ", " << mat.diffuse[1] << ", " << mat.diffuse[2] << ")" << std::endl;
        std::cout << "  Texture path: " << mat.diffuseTexturePath << std::endl;
        
        if (mat.diffuseTexturePath.empty()) {
            std::cerr << "  ✗ No diffuse texture path (map_Kd not parsed)" << std::endl;
            return 1;
        } else {
            std::cout << "  ✓ Diffuse texture path found" << std::endl;
        }
        
        if (!mat.diffuseTexture || !mat.diffuseTexture->IsValid()) {
            std::cerr << "  ✗ Texture not loaded or invalid" << std::endl;
            return 1;
        } else {
            std::cout << "  ✓ Texture loaded successfully!" << std::endl;
            std::cout << "    Size: " << mat.diffuseTexture->GetWidth() << "x" << mat.diffuseTexture->GetHeight() << std::endl;
        }
    }
    
    std::cout << "\n=== All Tests Passed! ===" << std::endl;
    return 0;
}
