// Example: Loading and using a mesh with the OBJ/MTL parser
//
// This example demonstrates how to load a 3D model from OBJ/MTL files
// and prepare it for rendering with OpenGL.

#include "Mesh.h"
#include "ObjParser.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include <iostream>

void ExampleMeshLoading() {
    // 1. Create a mesh and load from OBJ file
    Mesh mesh;
    bool success = ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);
    
    if (!success) {
        std::cerr << "Failed to load mesh!" << std::endl;
        return;
    }
    
    std::cout << "Successfully loaded mesh:" << std::endl;
    std::cout << "  - Vertices: " << mesh.GetVertexCount() << std::endl;
    std::cout << "  - Faces: " << mesh.GetFaceCount() << std::endl;
    std::cout << "  - Materials: " << mesh.GetMaterialCount() << std::endl;
    
    // 2. Get interleaved vertex data for OpenGL
    // Format: [px, py, pz, nx, ny, nz, u, v, ...]
    std::vector<float> vertexData = mesh.GetInterleavedVertexData();
    std::vector<unsigned int> indexData = mesh.GetIndexData();
    
    // 3. Create OpenGL buffers
    VertexBuffer vb(vertexData.data(), vertexData.size() * sizeof(float));
    IndexBuffer ib(indexData.data(), indexData.size() * sizeof(unsigned int));
    
    // 4. Setup vertex array with proper layout
    VertexArray va;
    VertexBufferLayout layout;
    layout.AddFloat(3);  // Position (x, y, z)
    layout.AddFloat(3);  // Normal (nx, ny, nz)
    layout.AddFloat(2);  // Texture coordinates (u, v)
    va.AddBuffer(vb, layout);
    
    // 5. Access material information
    const std::vector<Face>& faces = mesh.GetFaces();
    for (size_t i = 0; i < faces.size(); ++i) {
        const Face& face = faces[i];
        
        // Get material for this face
        if (!face.materialName.empty()) {
            const Material* mat = mesh.GetMaterial(face.materialName);
            if (mat) {
                std::cout << "Face " << i << " uses material: " << mat->name << std::endl;
                std::cout << "  Diffuse color: ("
                          << mat->diffuse[0] << ", "
                          << mat->diffuse[1] << ", "
                          << mat->diffuse[2] << ")" << std::endl;
                
                // Check for diffuse texture
                if (!mat->mapDiffuse.empty()) {
                    std::cout << "  Diffuse texture: " << mat->mapDiffuse << std::endl;
                    
                    // Here you would load the texture using stb_image:
                    // int width, height, channels;
                    // unsigned char* data = stbi_load(mat->mapDiffuse.c_str(), 
                    //                                 &width, &height, &channels, 0);
                    // if (data) {
                    //     // Create OpenGL texture
                    //     // glTexImage2D(...);
                    //     stbi_image_free(data);
                    // }
                }
            }
        }
    }
    
    // 6. Now you can render the mesh using the vertex array and index buffer
    // renderer.Draw(va, ib, shader);
}

void ExampleMaterialAccess() {
    // Load a mesh with materials
    Mesh mesh;
    ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);
    
    // Iterate through all materials
    const std::map<std::string, Material>& materials = mesh.GetMaterials();
    
    for (const auto& [name, material] : materials) {
        std::cout << "\nMaterial: " << name << std::endl;
        std::cout << "  Ambient:  (" << material.ambient[0] << ", " 
                  << material.ambient[1] << ", " << material.ambient[2] << ")" << std::endl;
        std::cout << "  Diffuse:  (" << material.diffuse[0] << ", " 
                  << material.diffuse[1] << ", " << material.diffuse[2] << ")" << std::endl;
        std::cout << "  Specular: (" << material.specular[0] << ", " 
                  << material.specular[1] << ", " << material.specular[2] << ")" << std::endl;
        std::cout << "  Shininess: " << material.shininess << std::endl;
        std::cout << "  Opacity: " << material.opacity << std::endl;
        
        // List all texture maps
        if (!material.mapDiffuse.empty())
            std::cout << "  Diffuse map: " << material.mapDiffuse << std::endl;
        if (!material.mapSpecular.empty())
            std::cout << "  Specular map: " << material.mapSpecular << std::endl;
        if (!material.mapBump.empty())
            std::cout << "  Bump map: " << material.mapBump << std::endl;
    }
}

void ExampleCreateMeshProgrammatically() {
    // Create a simple triangle mesh programmatically
    Mesh mesh("Triangle");
    
    // Define vertices
    Vertex v0, v1, v2;
    
    v0.position = {-0.5f, -0.5f, 0.0f};
    v0.normal = {0.0f, 0.0f, 1.0f};
    v0.texCoords = {0.0f, 0.0f};
    v0.color = {1.0f, 0.0f, 0.0f};  // Red
    
    v1.position = {0.5f, -0.5f, 0.0f};
    v1.normal = {0.0f, 0.0f, 1.0f};
    v1.texCoords = {1.0f, 0.0f};
    v1.color = {0.0f, 1.0f, 0.0f};  // Green
    
    v2.position = {0.0f, 0.5f, 0.0f};
    v2.normal = {0.0f, 0.0f, 1.0f};
    v2.texCoords = {0.5f, 1.0f};
    v2.color = {0.0f, 0.0f, 1.0f};  // Blue
    
    // Add vertices to mesh
    unsigned int idx0 = mesh.AddVertex(v0);
    unsigned int idx1 = mesh.AddVertex(v1);
    unsigned int idx2 = mesh.AddVertex(v2);
    
    // Create a material
    Material material("TriangleMaterial");
    material.diffuse = {1.0f, 1.0f, 1.0f};
    material.shininess = 64.0f;
    mesh.AddMaterial(material);
    
    // Add face
    Face face;
    face.indices = {idx0, idx1, idx2};
    face.materialName = "TriangleMaterial";
    mesh.AddFace(face);
    
    std::cout << "Created triangle mesh with " << mesh.GetVertexCount() 
              << " vertices and " << mesh.GetFaceCount() << " faces" << std::endl;
    
    // Get data for rendering
    std::vector<float> vertexData = mesh.GetInterleavedVertexData();
    std::vector<unsigned int> indices = mesh.GetIndexData();
    
    // Use with OpenGL...
}

// This file is for documentation purposes only
// Uncomment and integrate into main.cpp to use these examples
