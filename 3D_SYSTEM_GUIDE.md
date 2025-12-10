# 3D System Guide

## Overview
The RPG-Looter engine has been converted to a full 3D rendering system with support for loading models from Blender via OBJ/MTL files.

## Key Features

### ✅ 3D Rendering
- Perspective projection
- Depth testing and backface culling
- Phong lighting model (ambient + diffuse + specular)

### ✅ Blender Integration
- Load .obj model files
- Automatic .mtl material file parsing
- Support for colors and textures
- Multiple meshes per model

### ✅ Camera System
- 3D perspective camera
- WASD movement controls
- Configurable FOV, near/far planes

### ✅ Lighting
- Configurable light position and color
- Ambient, diffuse, and specular components
- Per-vertex normal lighting

## Creating Models in Blender

1. **Create your 3D model** in Blender
2. **Apply materials** with colors and/or textures
3. **Export as Wavefront (.obj)**:
   - File → Export → Wavefront (.obj)
   - ✅ Enable "Write Materials"
   - ✅ Enable "Include Normals"
   - ✅ Enable "Include UVs"
   - ✅ Enable "Triangulate Faces"
   - Set Forward: -Z Forward, Up: Y Up
4. **Place files** in your `res/models/` directory

## Loading Models in Code

```cpp
// Load model from OBJ file
AssetManager::getInstance()->loadModelFromFile("myModel", "res/models/my_model.obj");

// Create entity with the model
auto* entity = entityManager.createEntity();
entity->tag = "MyObject";

// Add transform (3D position, rotation, scale)
entity->addComponent<TransformComponent>();
entity->getComponent<TransformComponent>()->position = glm::vec3(0.0f, 0.0f, 0.0f);
entity->getComponent<TransformComponent>()->rotation = glm::vec3(0.0f, 0.0f, 0.0f); // radians
entity->getComponent<TransformComponent>()->scale = glm::vec3(1.0f, 1.0f, 1.0f);

// Add the model component
auto* modelComp = entity->addComponent<ModelComponent>();
auto* loadedModel = AssetManager::getInstance()->getModel("myModel");
if (loadedModel) {
    modelComp->meshes = loadedModel->meshes;
}
```

## Camera Setup

```cpp
// Create camera entity
auto* camera = entityManager.createEntity();
camera->tag = "MainCamera";
camera->addComponent<TransformComponent>();
camera->getComponent<TransformComponent>()->position = glm::vec3(0.0f, 2.0f, 5.0f);

camera->addComponent<CameraComponent>();
camera->getComponent<CameraComponent>()->fov = 60.0f;
camera->getComponent<CameraComponent>()->nearPlane = 0.1f;
camera->getComponent<CameraComponent>()->farPlane = 1000.0f;
camera->getComponent<CameraComponent>()->isActive = true;
```

## Controls

### Default Camera Controls (WASD)
- **W**: Move forward
- **S**: Move backward
- **A**: Strafe left
- **D**: Strafe right
- **Shift**: Sprint (2x speed)

## Component Reference

### TransformComponent (3D)
```cpp
glm::vec3 position;  // World position
glm::vec3 rotation;  // Euler angles in radians (pitch/X, yaw/Y, roll/Z)
glm::vec3 scale;     // Scale factors
```

### CameraComponent
```cpp
float fov;           // Field of view in degrees (default: 60)
float nearPlane;     // Near clipping plane (default: 0.1)
float farPlane;      // Far clipping plane (default: 1000)
bool isActive;       // Whether this is the active camera
glm::vec3 front;     // Forward direction (auto-updated)
glm::vec3 up;        // Up direction
glm::vec3 right;     // Right direction
```

### ModelComponent
```cpp
struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;
    std::string materialName;
    glm::vec3 color;
    std::string textureName;
};
std::vector<Mesh> meshes;  // Model can contain multiple meshes
```

## Material Files (.mtl)

The .mtl file is automatically loaded when you load an .obj file. It supports:

- `Kd`: Diffuse color (R G B)
- `Ka`: Ambient color (R G B)
- `Ks`: Specular color (R G B)
- `Ns`: Shininess (float)
- `map_Kd`: Diffuse texture path
- `d` or `Tr`: Opacity (0-1)

Example .mtl file:
```mtl
newmtl MyMaterial
Kd 0.8 0.2 0.2    # Red diffuse color
Ka 0.2 0.2 0.2    # Dark ambient
Ks 1.0 1.0 1.0    # White specular
Ns 32.0            # Shininess
map_Kd texture.png # Texture file
```

## Lighting Configuration

```cpp
// In your render system or game setup
renderSystem->setLightPosition(glm::vec3(5.0f, 5.0f, 5.0f));
renderSystem->setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
renderSystem->setAmbientColor(glm::vec3(0.2f, 0.2f, 0.2f));
```

## Tips

1. **Model Scale**: Blender uses meters, so a 1-unit cube in Blender = 1.0 scale in engine
2. **Normals**: Always export with normals for proper lighting
3. **Triangulate**: Export as triangles to ensure compatibility
4. **Textures**: Place textures in `res/textures/` and reference them in materials
5. **Performance**: Models are cached - OpenGL buffers created once on first render

## Removed Features

The following 2D features have been removed:
- ❌ JSON model system
- ❌ 2D orthographic rendering
- ❌ 2D shape primitives (Circle, Rectangle, etc.)

## Migration from 2D

If you had existing 2D code:

**Old (2D):**
```cpp
transform->position = glm::vec2(100.0f, 200.0f);
transform->rotation = 1.57f;  // radians
transform->scale = glm::vec2(2.0f, 2.0f);
```

**New (3D):**
```cpp
transform->position = glm::vec3(1.0f, 2.0f, 0.0f);
transform->rotation = glm::vec3(0.0f, 1.57f, 0.0f);  // pitch, yaw, roll
transform->scale = glm::vec3(2.0f, 2.0f, 2.0f);
```

## Troubleshooting

**Model doesn't appear:**
- Check camera position and direction
- Verify model was loaded successfully (check console)
- Ensure entity has both TransformComponent and ModelComponent

**Model appears black:**
- Check that normals were exported
- Verify lighting is configured
- Check material colors aren't (0,0,0)

**Performance issues:**
- Reduce polygon count in Blender
- Use level-of-detail (LOD) models
- Limit number of entities

## Example: Complete Scene Setup

```cpp
void Game::setupEntities() {
    // Camera
    auto* camera = entityManager.createEntity();
    camera->tag = "MainCamera";
    camera->addComponent<TransformComponent>();
    camera->getComponent<TransformComponent>()->position = glm::vec3(0.0f, 2.0f, 5.0f);
    camera->addComponent<CameraComponent>();
    camera->getComponent<CameraComponent>()->isActive = true;
    
    // Load models
    AssetManager::getInstance()->loadModelFromFile("character", "res/models/character.obj");
    AssetManager::getInstance()->loadModelFromFile("terrain", "res/models/terrain.obj");
    
    // Create character entity
    auto* character = entityManager.createEntity();
    character->tag = "Player";
    character->addComponent<TransformComponent>();
    character->getComponent<TransformComponent>()->position = glm::vec3(0.0f, 0.0f, 0.0f);
    auto* charModel = character->addComponent<ModelComponent>();
    auto* loadedChar = AssetManager::getInstance()->getModel("character");
    if (loadedChar) charModel->meshes = loadedChar->meshes;
    
    // Create terrain entity
    auto* terrain = entityManager.createEntity();
    terrain->tag = "Terrain";
    terrain->addComponent<TransformComponent>();
    auto* terrainModel = terrain->addComponent<ModelComponent>();
    auto* loadedTerrain = AssetManager::getInstance()->getModel("terrain");
    if (loadedTerrain) terrainModel->meshes = loadedTerrain->meshes;
}
```

---

**Enjoy creating 3D games with Blender integration!** 🎮✨
