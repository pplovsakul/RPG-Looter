# Mesh Class and OBJ/MTL Parser Documentation

## Overview

This implementation provides a comprehensive and well-structured Mesh class in C++ that supports materials and texture paths for 3D object loading and rendering. The system includes parsers for both `.obj` and `.mtl` (Wavefront) file formats.

## Features

- **Mesh Class**: Manages vertices, indices (triangles), and supports vertex attributes:
  - Position (x, y, z)
  - Normal vectors (nx, ny, nz)
  - Texture coordinates (u, v)
  - Optional vertex colors (r, g, b)

- **Material Support**: Each triangle/face can be associated with a material
  - Material properties from MTL files (ambient, diffuse, specular, shininess, etc.)
  - Texture map paths (map_Kd for diffuse, map_Bump for bump maps, etc.)
  - Supports common MTL directives

- **OBJ/MTL Parser**: Parses Wavefront OBJ and MTL files
  - No external dependencies (no tinyobjloader, assimp, etc.)
  - Supports vertex positions, normals, and texture coordinates
  - Automatic triangulation of polygon faces
  - Material library loading and association
  - Proper index handling for OBJ format (1-based to 0-based conversion)

## File Structure

```
src/
├── Material.h          # Material structure definition
├── Mesh.h              # Mesh class declaration
├── Mesh.cpp            # Mesh class implementation
├── ObjParser.h         # OBJ/MTL parser declarations
└── ObjParser.cpp       # OBJ/MTL parser implementation

tests/
└── test_mesh.cpp       # Unit tests for Mesh and parser functionality

res/models/
├── cube.obj            # Example OBJ file
└── cube.mtl            # Example MTL file
```

## Usage

### Loading an OBJ File

```cpp
#include "Mesh.h"
#include "ObjParser.h"

// Create a mesh and load an OBJ file
Mesh mesh;
bool success = ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);

if (success) {
    std::cout << "Loaded " << mesh.GetVertexCount() << " vertices" << std::endl;
    std::cout << "Loaded " << mesh.GetFaceCount() << " faces" << std::endl;
    std::cout << "Loaded " << mesh.GetMaterialCount() << " materials" << std::endl;
}
```

### Accessing Mesh Data

```cpp
// Get vertices
const std::vector<Vertex>& vertices = mesh.GetVertices();

// Get faces
const std::vector<Face>& faces = mesh.GetFaces();

// Get materials
const std::map<std::string, Material>& materials = mesh.GetMaterials();

// Get specific material
const Material* mat = mesh.GetMaterial("RedMaterial");
if (mat) {
    std::cout << "Diffuse color: " 
              << mat->diffuse[0] << ", "
              << mat->diffuse[1] << ", "
              << mat->diffuse[2] << std::endl;
    
    if (!mat->mapDiffuse.empty()) {
        std::cout << "Diffuse texture: " << mat->mapDiffuse << std::endl;
    }
}
```

### Getting Data for Rendering

```cpp
// Get interleaved vertex data (position, normal, texcoord)
// Format: [px, py, pz, nx, ny, nz, u, v, ...]
std::vector<float> vertexData = mesh.GetInterleavedVertexData();

// Get index data for indexed rendering
std::vector<unsigned int> indices = mesh.GetIndexData();

// Use with OpenGL
VertexBuffer vb(vertexData.data(), vertexData.size() * sizeof(float));
IndexBuffer ib(indices.data(), indices.size() * sizeof(unsigned int));
```

### Creating Meshes Programmatically

```cpp
Mesh mesh("MyMesh");

// Add vertices
Vertex v1;
v1.position = {0.0f, 0.0f, 0.0f};
v1.normal = {0.0f, 1.0f, 0.0f};
v1.texCoords = {0.0f, 0.0f};

unsigned int idx = mesh.AddVertex(v1);

// Add materials
Material mat("MyMaterial");
mat.diffuse = {1.0f, 0.0f, 0.0f};
mat.mapDiffuse = "texture.png";
mesh.AddMaterial(mat);

// Add faces
Face face;
face.indices = {0, 1, 2};
face.materialName = "MyMaterial";
mesh.AddFace(face);
```

### Parsing Only MTL Files

```cpp
#include "ObjParser.h"

std::map<std::string, Material> materials;
bool success = ObjParser::ParseMTL("path/to/materials.mtl", materials);

if (success) {
    for (const auto& [name, material] : materials) {
        std::cout << "Material: " << name << std::endl;
    }
}
```

## Material Properties

The Material structure supports the following properties from MTL files:

- `name` - Material name (from `newmtl` directive)
- `ambient` - Ambient color (Ka)
- `diffuse` - Diffuse color (Kd)
- `specular` - Specular color (Ks)
- `shininess` - Specular exponent (Ns)
- `opacity` - Transparency (d or Tr)
- `indexOfRefraction` - Optical density (Ni)
- `illuminationModel` - Illumination model (illum)
- `mapAmbient` - Ambient texture map (map_Ka)
- `mapDiffuse` - Diffuse texture map (map_Kd) - Most commonly used
- `mapSpecular` - Specular texture map (map_Ks)
- `mapBump` - Bump map (map_Bump or bump)
- `mapDisplacement` - Displacement map (disp)
- `mapAlpha` - Alpha/opacity map (map_d)

## OBJ File Format Support

The parser supports the following OBJ directives:

- `v` - Vertex positions
- `vt` - Texture coordinates
- `vn` - Vertex normals
- `f` - Face definitions (with automatic triangulation for polygons)
- `usemtl` - Use material for following faces
- `mtllib` - Material library reference
- `o` / `g` - Object/group names

Face formats supported:
- `f v1 v2 v3` - Positions only
- `f v1/vt1 v2/vt2 v3/vt3` - Positions and texture coordinates
- `f v1//vn1 v2//vn2 v3//vn3` - Positions and normals
- `f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3` - Positions, texture coordinates, and normals

## Testing

Unit tests are provided in `tests/test_mesh.cpp` and cover:

- Material creation and properties
- Mesh basic operations (add vertex, add face, add material)
- Mesh data retrieval
- MTL file parsing
- OBJ file parsing
- Interleaved vertex data generation
- Index data generation

To run tests:

```bash
cd build
./test_mesh
```

## Example OBJ/MTL Files

Example files are provided in `res/models/`:

- `cube.obj` - A simple cube with 8 vertices and 12 triangular faces
- `cube.mtl` - Material library with three materials (RedMaterial, BlueMaterial, TexturedMaterial)

## Integration with stb_image

While stb_image integration is not implemented in this version, the structure is ready for it. Texture paths are stored in Material objects and can be loaded using stb_image:

```cpp
#include "stb_image.h"

const Material* mat = mesh.GetMaterial("MaterialName");
if (mat && !mat->mapDiffuse.empty()) {
    int width, height, channels;
    unsigned char* data = stbi_load(mat->mapDiffuse.c_str(), 
                                    &width, &height, &channels, 0);
    if (data) {
        // Use texture data with OpenGL
        // glTexImage2D(...);
        stbi_image_free(data);
    }
}
```

## Design Considerations

- **Extensibility**: The design is modular and can be easily extended with additional features
- **Performance**: Uses vectors and maps for efficient data storage and retrieval
- **OpenGL Ready**: Provides methods to get data in formats suitable for OpenGL rendering
- **No External Dependencies**: Parser implementation doesn't rely on external mesh loading libraries
- **Clear Documentation**: All classes and methods are well-documented with comments

## Future Enhancements

Potential improvements for the future:

- Vertex deduplication to reduce memory usage
- Support for more MTL properties
- Binary file format support for faster loading
- Mesh optimization and LOD generation
- Normal generation for meshes without normals
- Tangent and bitangent calculation for normal mapping
