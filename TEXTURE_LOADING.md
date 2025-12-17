# OBJ/MTL Texture Loading Guide

This document explains how to use OBJ models with MTL materials and textures in the RPG-Looter project.

## Overview

The project now supports loading OBJ files with:
- Vertex positions
- Texture coordinates (UVs)
- MTL material definitions
- Diffuse textures via `map_Kd`

## File Structure

Place your model files in the `res/models/` directory:

```
res/models/
├── yourmodel.obj
├── yourmodel.mtl
└── yourtexture.png
```

## OBJ File Format

Your OBJ file should reference the MTL file and include texture coordinates:

```obj
# Reference the material library
mtllib yourmodel.mtl

# Vertex positions
v -1.0 -1.0  1.0
v  1.0 -1.0  1.0
...

# Texture coordinates (UVs)
vt 0.0 0.0
vt 1.0 0.0
...

# Use a material
usemtl YourMaterialName

# Faces with vertex/texture indices
f 1/1 2/2 3/3
```

## MTL File Format

The MTL file defines materials and textures:

```mtl
# Material definition
newmtl YourMaterialName

# Colors (RGB values 0.0 to 1.0)
Ka 1.0 1.0 1.0  # Ambient color
Kd 1.0 1.0 1.0  # Diffuse color
Ks 0.5 0.5 0.5  # Specular color
Ke 0.0 0.0 0.0  # Emissive color (optional)

# Material properties
Ns 32.0   # Specular exponent
illum 2   # Illumination model (optional, 0-10)

# Diffuse texture map (relative path)
map_Kd yourtexture.png
```

### Supported MTL Keywords
- `newmtl` - Material name
- `Ka` - Ambient color
- `Kd` - Diffuse color
- `Ks` - Specular color
- `Ke` - Emissive color (parsed but not used in rendering)
- `Ns` - Specular exponent
- `Ni` - Optical density / Index of refraction
- `d` / `Tr` - Transparency (dissolve)
- `illum` - Illumination model (parsed but not used in rendering)
- `map_Ka` - Ambient texture map
- `map_Kd` - Diffuse texture map
- `map_Ks` - Specular texture map

## Important Notes

### Path Resolution
- Texture paths in `map_Kd` should be **relative** to the MTL file location
- Both forward slashes (`/`) and backslashes (`\\`) are supported
- Example: If MTL is in `res/models/`, then `map_Kd texture.png` looks for `res/models/texture.png`

### Absolute Windows Paths
- Absolute Windows paths (e.g., `D:\path\to\texture.png`) are detected
- A warning is issued as these paths will likely fail on other systems
- **Recommendation**: Use relative paths for portability

### Texture Format
- Supported formats: PNG, JPG, TGA, BMP (via stb_image)
- Recommended: PNG with transparency (RGBA)
- Textures are automatically flipped vertically for OpenGL

### Error Handling
- If a texture file is not found, an error is logged with the full path attempted
- The model will still load but will render with the fallback color
- Check console output for detailed error messages

## Example Usage in Code

```cpp
#include "OBJLoader.h"

// Load the OBJ file (automatically loads MTL and textures)
OBJLoader::MeshData mesh;
if (OBJLoader::LoadOBJ("res/models/yourmodel.obj", mesh)) {
    // Check if texture was loaded
    for (const auto& matPair : mesh.materials) {
        const Material& mat = matPair.second;
        if (mat.diffuseTexture && mat.diffuseTexture->IsValid()) {
            // Bind and use the texture
            mat.diffuseTexture->Bind(0);
        }
    }
}
```

## Testing Your Model

Run the included test to verify your OBJ/MTL/texture loads correctly:

```bash
cd build
./test_obj
```

The test will report:
- ✓ If the OBJ file loads
- ✓ If materials are found
- ✓ If textures are loaded
- ✗ Any errors encountered

## Troubleshooting

### "Failed to load texture: can't fopen"
- **Most common cause**: The texture file doesn't exist at the resolved path
- Check the console output to see the resolved path (e.g., `res/models/textures/Tex.png`)
- Ensure the texture file exists at that exact location
- If your MTL has `map_Kd textures\Tex.png`, make sure the `textures` subdirectory exists
- Create the directory structure: `mkdir -p res/models/textures`
- Copy your texture file to the correct location

### "Failed to load texture"
- Check the file path in the MTL file
- Ensure the texture file exists relative to the MTL file
- Verify the texture file format is supported

### "No materials loaded from MTL file"
- Verify the MTL file is in the same directory as the OBJ
- Check that `mtllib` in OBJ matches the actual MTL filename
- Ensure MTL file contains at least one `newmtl` definition

### "map_Kd not parsed"
- Verify the `map_Kd` line in the MTL file is correctly formatted
- Check that the texture filename doesn't have special characters
- Try using a simple filename without spaces

### Model renders without texture
- Check console output for texture loading errors
- Verify the shader is configured to use textures (`u_UseTexture = 1`)
- Ensure the texture is bound before rendering

## See Also

- Test model: `res/models/Test.obj` with `Test.mtl` and `Test.png`
- Source code: `src/OBJLoader.cpp` for implementation details
- Parsers: `src/objparser.cpp` and `src/mtlparser.cpp`
