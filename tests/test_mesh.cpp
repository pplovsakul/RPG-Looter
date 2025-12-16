#include <iostream>
#include <cassert>
#include <cmath>
#include "../src/Mesh.h"
#include "../src/Material.h"
#include "../src/ObjParser.h"

// Helper function to compare floats
bool FloatEqual(float a, float b, float epsilon = 0.0001f) {
    return std::abs(a - b) < epsilon;
}

// Test Material creation
void TestMaterialCreation() {
    std::cout << "Testing Material creation..." << std::endl;
    
    Material mat("TestMaterial");
    assert(mat.name == "TestMaterial");
    assert(FloatEqual(mat.diffuse[0], 0.8f));
    assert(FloatEqual(mat.diffuse[1], 0.8f));
    assert(FloatEqual(mat.diffuse[2], 0.8f));
    assert(FloatEqual(mat.opacity, 1.0f));
    
    std::cout << "  ✓ Material creation test passed" << std::endl;
}

// Test Mesh basic operations
void TestMeshBasics() {
    std::cout << "Testing Mesh basic operations..." << std::endl;
    
    Mesh mesh("TestMesh");
    assert(mesh.GetName() == "TestMesh");
    assert(mesh.GetVertexCount() == 0);
    assert(mesh.GetFaceCount() == 0);
    
    // Add a vertex
    Vertex v1;
    v1.position = {1.0f, 2.0f, 3.0f};
    v1.normal = {0.0f, 1.0f, 0.0f};
    v1.texCoords = {0.5f, 0.5f};
    
    unsigned int idx = mesh.AddVertex(v1);
    assert(idx == 0);
    assert(mesh.GetVertexCount() == 1);
    
    // Add a material
    Material mat("TestMat");
    mat.diffuse = {1.0f, 0.0f, 0.0f};
    mesh.AddMaterial(mat);
    assert(mesh.GetMaterialCount() == 1);
    
    const Material* retrievedMat = mesh.GetMaterial("TestMat");
    assert(retrievedMat != nullptr);
    assert(FloatEqual(retrievedMat->diffuse[0], 1.0f));
    
    // Add a face
    Face face;
    face.indices = {0, 1, 2};
    face.materialName = "TestMat";
    mesh.AddFace(face);
    assert(mesh.GetFaceCount() == 1);
    
    std::cout << "  ✓ Mesh basic operations test passed" << std::endl;
}

// Test MTL parsing
void TestMTLParsing() {
    std::cout << "Testing MTL file parsing..." << std::endl;
    
    std::map<std::string, Material> materials;
    bool result = ObjParser::ParseMTL("res/models/cube.mtl", materials);
    
    if (!result) {
        std::cerr << "  ✗ Failed to parse cube.mtl" << std::endl;
        return;
    }
    
    assert(materials.size() == 3);
    assert(materials.find("RedMaterial") != materials.end());
    assert(materials.find("BlueMaterial") != materials.end());
    assert(materials.find("TexturedMaterial") != materials.end());
    
    // Check RedMaterial properties
    const Material& redMat = materials["RedMaterial"];
    assert(FloatEqual(redMat.diffuse[0], 0.8f));
    assert(FloatEqual(redMat.diffuse[1], 0.0f));
    assert(FloatEqual(redMat.diffuse[2], 0.0f));
    assert(FloatEqual(redMat.shininess, 32.0f));
    
    // Check TexturedMaterial properties
    const Material& texMat = materials["TexturedMaterial"];
    assert(texMat.mapDiffuse == "texture.png");
    
    std::cout << "  ✓ MTL parsing test passed" << std::endl;
}

// Test OBJ parsing
void TestOBJParsing() {
    std::cout << "Testing OBJ file parsing..." << std::endl;
    
    Mesh mesh;
    bool result = ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);
    
    if (!result) {
        std::cerr << "  ✗ Failed to parse cube.obj" << std::endl;
        return;
    }
    
    // Cube has 8 unique vertices, but after parsing with separate normals/texcoords,
    // we will have many more vertices (36 for 12 triangles)
    assert(mesh.GetVertexCount() > 0);
    assert(mesh.GetFaceCount() == 12); // 6 faces * 2 triangles each
    
    // Check that materials were loaded
    assert(mesh.GetMaterialCount() == 3);
    assert(mesh.GetMaterial("RedMaterial") != nullptr);
    assert(mesh.GetMaterial("BlueMaterial") != nullptr);
    
    // Verify we can get interleaved data
    std::vector<float> vertexData = mesh.GetInterleavedVertexData();
    assert(vertexData.size() == mesh.GetVertexCount() * 8); // 3 pos + 3 normal + 2 texcoord
    
    // Verify we can get index data
    std::vector<unsigned int> indexData = mesh.GetIndexData();
    assert(indexData.size() == mesh.GetFaceCount() * 3); // 3 indices per triangle
    
    std::cout << "  ✓ OBJ parsing test passed" << std::endl;
    std::cout << "    - Loaded " << mesh.GetVertexCount() << " vertices" << std::endl;
    std::cout << "    - Loaded " << mesh.GetFaceCount() << " faces" << std::endl;
    std::cout << "    - Loaded " << mesh.GetMaterialCount() << " materials" << std::endl;
}

// Test mesh data retrieval
void TestMeshDataRetrieval() {
    std::cout << "Testing Mesh data retrieval..." << std::endl;
    
    Mesh mesh;
    
    // Create a simple triangle
    Vertex v0, v1, v2;
    v0.position = {0.0f, 0.0f, 0.0f};
    v0.normal = {0.0f, 0.0f, 1.0f};
    v0.texCoords = {0.0f, 0.0f};
    
    v1.position = {1.0f, 0.0f, 0.0f};
    v1.normal = {0.0f, 0.0f, 1.0f};
    v1.texCoords = {1.0f, 0.0f};
    
    v2.position = {0.0f, 1.0f, 0.0f};
    v2.normal = {0.0f, 0.0f, 1.0f};
    v2.texCoords = {0.0f, 1.0f};
    
    unsigned int idx0 = mesh.AddVertex(v0);
    unsigned int idx1 = mesh.AddVertex(v1);
    unsigned int idx2 = mesh.AddVertex(v2);
    
    Face face;
    face.indices = {idx0, idx1, idx2};
    mesh.AddFace(face);
    
    // Get interleaved data
    std::vector<float> data = mesh.GetInterleavedVertexData();
    assert(data.size() == 24); // 3 vertices * 8 floats each
    
    // Check first vertex data
    assert(FloatEqual(data[0], 0.0f));  // pos x
    assert(FloatEqual(data[1], 0.0f));  // pos y
    assert(FloatEqual(data[2], 0.0f));  // pos z
    assert(FloatEqual(data[3], 0.0f));  // normal x
    assert(FloatEqual(data[4], 0.0f));  // normal y
    assert(FloatEqual(data[5], 1.0f));  // normal z
    assert(FloatEqual(data[6], 0.0f));  // texcoord u
    assert(FloatEqual(data[7], 0.0f));  // texcoord v
    
    // Get index data
    std::vector<unsigned int> indices = mesh.GetIndexData();
    assert(indices.size() == 3);
    assert(indices[0] == 0);
    assert(indices[1] == 1);
    assert(indices[2] == 2);
    
    std::cout << "  ✓ Mesh data retrieval test passed" << std::endl;
}

// Test mesh clear
void TestMeshClear() {
    std::cout << "Testing Mesh clear..." << std::endl;
    
    Mesh mesh("TestMesh");
    Vertex v;
    mesh.AddVertex(v);
    Material mat("TestMat");
    mesh.AddMaterial(mat);
    Face f;
    mesh.AddFace(f);
    
    assert(mesh.GetVertexCount() > 0);
    assert(mesh.GetMaterialCount() > 0);
    assert(mesh.GetFaceCount() > 0);
    
    mesh.Clear();
    
    assert(mesh.GetVertexCount() == 0);
    assert(mesh.GetMaterialCount() == 0);
    assert(mesh.GetFaceCount() == 0);
    assert(mesh.GetName().empty());
    
    std::cout << "  ✓ Mesh clear test passed" << std::endl;
}

int main() {
    std::cout << "=== Running Mesh and OBJ/MTL Parser Tests ===" << std::endl;
    std::cout << std::endl;
    
    try {
        TestMaterialCreation();
        TestMeshBasics();
        TestMeshDataRetrieval();
        TestMeshClear();
        TestMTLParsing();
        TestOBJParsing();
        
        std::cout << std::endl;
        std::cout << "=== All tests passed! ===" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
