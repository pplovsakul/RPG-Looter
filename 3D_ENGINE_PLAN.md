# RPG-Looter 3D Engine - Projektplanung

## Projektübersicht

Dieses Dokument beschreibt die Architektur und Implementierung eines 3D-Rendering-Engines mit Fokus auf:
- 3D Mesh Rendering (OBJ-Dateien)
- Kamera-Steuerung (Orbital, First-Person)
- Player-Steuerung mit Physik
- ImGui-Integration für Debug-Informationen
- Moderne OpenGL 3.3+ Pipeline

## 1. Bestehende Basis (bereits vorhanden)

### 1.1 Rendering-Infrastruktur
✅ **VertexBuffer** (`src/VertexBuffer.h/cpp`)
- Bereits implementiert
- Unterstützt dynamische Daten-Updates
- OpenGL Buffer Abstraktion

✅ **IndexBuffer** (`src/IndexBuffer.h/cpp`)
- Bereits implementiert
- Unterstützt dynamische Index-Updates
- Count-Tracking für Draw Calls

✅ **VertexArray** (`src/VertexArray.h/cpp`)
- Bereits implementiert
- VAO Abstraktion
- Vertex Attribute Layout

✅ **Shader** (`src/Shader.h/cpp`)
- Bereits implementiert
- Shader Kompilierung und Linking
- Uniform Management

### 1.2 Input System
✅ **InputSystem** (`src/InputSystem.h/cpp`)
- Context-basiertes Input Management
- Key/Mouse State Tracking
- Action Mapping System
- Bereits GLFW-integriert

### 1.3 ECS-Architektur
✅ **Entity-Component-System**
- Entity, Component, EntityManager vorhanden
- TransformComponent (Position, Rotation, Scale)
- System-Architektur vorhanden

### 1.4 Asset Management
✅ **AssetManager** (`src/AssetManager.h/cpp`)
- Texture Loading
- Shader Management
- Sound Management (OpenAL)

### 1.5 UI-Infrastruktur
✅ **ImGui** bereits integriert
- imgui_impl_glfw
- imgui_impl_opengl3
- Läuft in main.cpp

## 2. Neu zu implementierende Komponenten

### 2.1 3D Mesh System

#### 2.1.1 Mesh-Klasse
**Datei:** `src/Mesh3D.h/cpp`

**Zweck:** 
- Speichert Vertex-Daten (Position, Normal, UV)
- Verwaltet VBO/IBO/VAO
- Unterstützt verschiedene Primitive (Triangles, Quads)

**Datenstruktur:**
```cpp
struct Vertex3D {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color; // Optional für Vertex-Coloring
};

class Mesh3D {
private:
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<VertexBuffer> vbo;
    std::unique_ptr<IndexBuffer> ibo;
    
    unsigned int materialID; // Referenz zu Material/Texture
    
public:
    Mesh3D(const std::vector<Vertex3D>& vertices, 
           const std::vector<unsigned int>& indices);
    
    void setupBuffers();
    void draw() const;
    void updateVertices(const std::vector<Vertex3D>& newVertices);
    
    // Getters
    const std::vector<Vertex3D>& getVertices() const;
    const std::vector<unsigned int>& getIndices() const;
    size_t getVertexCount() const;
    size_t getIndexCount() const;
};
```

**Features:**
- Effizientes Vertex-Layout
- Normale für Beleuchtung
- UV-Koordinaten für Texturen
- Index-basiertes Rendering
- Update-Methode für dynamische Meshes

#### 2.1.2 OBJ-Loader
**Datei:** `src/OBJLoader.h/cpp`

**Zweck:**
- Lädt .obj Dateien
- Parst Vertex-Daten, Normalen, UVs
- Unterstützt mehrere Meshes pro Datei
- Integration mit AssetManager

**Funktionalität:**
```cpp
class OBJLoader {
public:
    // Lädt ein einzelnes Mesh
    static std::unique_ptr<Mesh3D> loadOBJ(const std::string& filepath);
    
    // Lädt mehrere Meshes (für komplexe Modelle)
    static std::vector<std::unique_ptr<Mesh3D>> loadOBJMulti(const std::string& filepath);
    
    // Mit Material-Support
    static std::unique_ptr<Mesh3D> loadOBJWithMaterial(
        const std::string& filepath,
        const std::string& mtlFilepath
    );

private:
    static bool parseOBJFile(const std::string& filepath, 
                            std::vector<glm::vec3>& positions,
                            std::vector<glm::vec3>& normals,
                            std::vector<glm::vec2>& uvs,
                            std::vector<unsigned int>& indices);
    
    static void calculateNormals(std::vector<Vertex3D>& vertices,
                                const std::vector<unsigned int>& indices);
};
```

**Unterstützte Formate:**
- Vertex Positions (v)
- Vertex Normals (vn)
- Texture Coordinates (vt)
- Faces (f)
- Material-Referenzen (usemtl) - Optional

#### 2.1.3 Primitive Generator
**Datei:** `src/PrimitiveGenerator.h/cpp`

**Zweck:**
- Generiert Standard-3D-Formen
- Für Testing und Prototyping
- Prozedural generierte Geometrie

**Formen:**
```cpp
class PrimitiveGenerator {
public:
    static std::unique_ptr<Mesh3D> createCube(float size = 1.0f);
    static std::unique_ptr<Mesh3D> createSphere(float radius = 1.0f, 
                                                 int segments = 32);
    static std::unique_ptr<Mesh3D> createCylinder(float radius = 1.0f, 
                                                   float height = 2.0f, 
                                                   int segments = 32);
    static std::unique_ptr<Mesh3D> createPlane(float width = 1.0f, 
                                               float height = 1.0f,
                                               int subdivisionsX = 1,
                                               int subdivisionsY = 1);
    static std::unique_ptr<Mesh3D> createCone(float radius = 1.0f,
                                              float height = 2.0f,
                                              int segments = 32);
};
```

### 2.2 Camera System

#### 2.2.1 Camera-Basisklasse
**Datei:** `src/Camera.h/cpp`

**Zweck:**
- Abstrakte Basis für verschiedene Kamera-Modi
- View/Projection Matrix Berechnung
- Frustum für Culling (Optional)

**Struktur:**
```cpp
class Camera {
protected:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
    // Euler Angles
    float yaw;
    float pitch;
    
    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom; // FOV
    
    // Projection
    float aspectRatio;
    float nearPlane;
    float farPlane;
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    bool needsViewUpdate;
    bool needsProjectionUpdate;

public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);
    
    virtual ~Camera() = default;
    
    // Interface für unterschiedliche Kamera-Modi
    virtual void processKeyboard(CameraMovement direction, float deltaTime) = 0;
    virtual void processMouseMovement(float xoffset, float yoffset, 
                                     bool constrainPitch = true) = 0;
    virtual void processMouseScroll(float yoffset) = 0;
    
    // Matrix Getters
    const glm::mat4& getViewMatrix();
    const glm::mat4& getProjectionMatrix();
    
    // Setters
    void setAspectRatio(float aspect);
    void setClippingPlanes(float near, float far);
    
    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::vec3 getRight() const { return right; }
    float getZoom() const { return zoom; }
    
protected:
    void updateCameraVectors();
    void updateViewMatrix();
    void updateProjectionMatrix();
};

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};
```

#### 2.2.2 FPS-Kamera
**Datei:** `src/FPSCamera.h/cpp`

**Zweck:**
- First-Person Shooter Stil
- WASD + Maus-Look
- Feste Y-Achse (kein Roll)

```cpp
class FPSCamera : public Camera {
public:
    FPSCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
    
    void processKeyboard(CameraMovement direction, float deltaTime) override;
    void processMouseMovement(float xoffset, float yoffset, 
                             bool constrainPitch = true) override;
    void processMouseScroll(float yoffset) override;
    
    // Zusätzliche FPS-spezifische Funktionen
    void setHeight(float height); // Fixe Höhe für FPS
    void enableGroundClipping(bool enable, float groundHeight = 0.0f);

private:
    bool groundClipping = false;
    float groundHeight = 0.0f;
};
```

#### 2.2.3 Orbital-Kamera
**Datei:** `src/OrbitalCamera.h/cpp`

**Zweck:**
- Umkreist einen Fokuspunkt
- Ideal für Modellanzeige
- Zoom, Pan, Rotate

```cpp
class OrbitalCamera : public Camera {
private:
    glm::vec3 target; // Punkt um den sich die Kamera dreht
    float distance;   // Abstand zum Target
    float minDistance;
    float maxDistance;
    
    // Orbital angles
    float azimuth;   // Horizontale Rotation
    float elevation; // Vertikale Rotation
    
    float panSpeed;

public:
    OrbitalCamera(glm::vec3 target = glm::vec3(0.0f),
                  float distance = 5.0f);
    
    void processKeyboard(CameraMovement direction, float deltaTime) override;
    void processMouseMovement(float xoffset, float yoffset, 
                             bool constrainPitch = true) override;
    void processMouseScroll(float yoffset) override;
    
    // Orbital-spezifisch
    void setTarget(const glm::vec3& newTarget);
    void setDistance(float dist);
    void setDistanceLimits(float min, float max);
    
    void rotate(float deltaAzimuth, float deltaElevation);
    void pan(float deltaX, float deltaY);
    void zoom(float delta);
    
    glm::vec3 getTarget() const { return target; }
    float getDistance() const { return distance; }

private:
    void updatePositionFromAngles();
};
```

#### 2.2.4 Camera Controller Component
**Datei:** `src/CameraController.h/cpp`

**Zweck:**
- Integriert Kamera mit InputSystem
- Wechsel zwischen Kamera-Modi
- Input-Context Verwaltung

```cpp
class CameraController {
private:
    std::unique_ptr<Camera> activeCamera;
    std::unordered_map<std::string, std::unique_ptr<Camera>> cameras;
    std::string activeCameraName;
    
    InputSystem* inputSystem;
    
    // Mouse state
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    bool firstMouse = true;
    bool mouseCaptured = false;

public:
    CameraController(InputSystem* input);
    
    void update(float deltaTime);
    
    // Kamera-Verwaltung
    void addCamera(const std::string& name, std::unique_ptr<Camera> camera);
    void setActiveCamera(const std::string& name);
    Camera* getActiveCamera();
    const Camera* getActiveCamera() const;
    
    // Input
    void processInput(GLFWwindow* window, float deltaTime);
    void processMouseMovement(float xpos, float ypos);
    void processMouseScroll(float yoffset);
    
    // Maus-Capture für Kamera-Steuerung
    void captureMouse(bool capture);
    bool isMouseCaptured() const { return mouseCaptured; }
};
```

### 2.3 Player Controller

#### 2.3.1 Character Controller Component
**Datei:** Erweiterung von `src/Components.h`

```cpp
class CharacterControllerComponent : public Component {
public:
    // Movement
    float moveSpeed = 5.0f;
    float sprintMultiplier = 2.0f;
    float jumpForce = 5.0f;
    
    // Physics
    glm::vec3 velocity{0.0f};
    bool isGrounded = false;
    bool isJumping = false;
    
    // Input state
    glm::vec2 moveInput{0.0f}; // x: left/right, y: forward/backward
    bool wantsToJump = false;
    bool isSprinting = false;
    
    // Configuration
    bool useGravity = true;
    float gravity = -9.81f;
    float groundCheckDistance = 0.1f;
    
    // Collision
    float radius = 0.5f; // Capsule Radius
    float height = 2.0f; // Capsule Höhe
};
```

#### 2.3.2 Player Controller System
**Datei:** `src/PlayerControllerSystem.h/cpp`

**Zweck:**
- Verarbeitet Player-Input
- Physik-basierte Bewegung
- Kollisionserkennung (optional)

```cpp
class PlayerControllerSystem : public System {
private:
    InputSystem* inputSystem;
    
public:
    PlayerControllerSystem(InputSystem* input);
    
    void update(EntityManager& em, float deltaTime) override;
    
private:
    void processMovementInput(Entity* player, CharacterControllerComponent* controller,
                             float deltaTime);
    void applyPhysics(TransformComponent* transform, 
                     CharacterControllerComponent* controller,
                     float deltaTime);
    void checkGrounded(TransformComponent* transform,
                      CharacterControllerComponent* controller);
    void handleJump(CharacterControllerComponent* controller);
};
```

### 2.4 3D Rendering System

#### 2.4.1 Mesh Component
**Datei:** Erweiterung von `src/Components.h`

```cpp
class MeshComponent : public Component {
public:
    std::string meshName;  // Referenz zu geladenem Mesh im AssetManager
    std::shared_ptr<Mesh3D> mesh; // Direkter Mesh-Pointer (alternativ)
    
    // Material
    std::string materialName;
    glm::vec3 diffuseColor{1.0f};
    glm::vec3 specularColor{0.5f};
    float shininess = 32.0f;
    
    // Rendering
    bool castShadows = true;
    bool receiveShadows = true;
    bool visible = true;
    int renderLayer = 0;
    
    // Wireframe Mode (für Debug)
    bool wireframe = false;
};
```

#### 2.4.2 3D Render System
**Datei:** `src/RenderSystem3D.h/cpp`

**Zweck:**
- Rendert 3D Meshes
- Lighting
- Material System
- Shader Management

```cpp
class RenderSystem3D : public System {
private:
    AssetManager* assetManager;
    CameraController* cameraController;
    
    // Shaders
    unsigned int defaultShaderID;
    unsigned int lightingShaderID;
    
    // Lighting
    glm::vec3 lightDirection{-0.2f, -1.0f, -0.3f};
    glm::vec3 lightColor{1.0f, 1.0f, 1.0f};
    glm::vec3 ambientColor{0.3f, 0.3f, 0.3f};
    
public:
    RenderSystem3D(AssetManager* assets, CameraController* camera);
    
    void init();
    void update(EntityManager& em, float deltaTime) override;
    
    // Lighting
    void setDirectionalLight(const glm::vec3& direction, 
                            const glm::vec3& color);
    void setAmbientLight(const glm::vec3& color);
    
private:
    void createDefaultShaders();
    void renderMesh(const MeshComponent* meshComp,
                   const TransformComponent* transform);
    void setupLighting(unsigned int shaderID);
};
```

### 2.5 ImGui Debug Interface

#### 2.5.1 Debug-Fenster
**Datei:** `src/DebugWindow.h/cpp`

**Zweck:**
- Zeigt alle relevanten Engine-Informationen
- Performance-Metriken
- Entity-Inspektor
- Kamera-Controls

```cpp
class DebugWindow {
private:
    EntityManager* entityManager;
    CameraController* cameraController;
    RenderSystem3D* renderSystem;
    
    // UI State
    bool showPerformance = true;
    bool showCameraInfo = true;
    bool showEntityList = true;
    bool showRenderStats = true;
    
    // Performance Tracking
    float fpsHistory[100] = {0};
    int fpsHistoryIndex = 0;
    float avgFPS = 0.0f;
    
    Entity* selectedEntity = nullptr;

public:
    DebugWindow(EntityManager* em, CameraController* camera, 
                RenderSystem3D* renderer);
    
    void render();
    
private:
    void renderPerformanceWindow();
    void renderCameraWindow();
    void renderEntityListWindow();
    void renderRenderStatsWindow();
    void renderEntityInspector();
    
    void updateFPSHistory(float fps);
};
```

**Features:**
- FPS Counter mit History Graph
- Frame Time Graph
- Camera Position/Rotation Anzeige
- Camera-Modus Wechsel
- Entity-Liste mit Filter
- Transform Component Editor
- Mesh Component Inspector
- Lighting Controls
- Render Statistics (Draw Calls, Vertices, etc.)

## 3. Implementierungsreihenfolge

### Phase 1: 3D Mesh Grundlagen (Tag 1-2)
1. ✅ Mesh3D Klasse implementieren
2. ✅ Vertex3D Struktur definieren
3. ✅ PrimitiveGenerator für Test-Meshes
4. ✅ Einfaches 3D Rendering testen

### Phase 2: OBJ-Loader (Tag 2-3)
1. ✅ OBJLoader Grundfunktionalität
2. ✅ Parser für v, vn, vt, f
3. ✅ Normal-Berechnung falls fehlend
4. ✅ Integration mit AssetManager
5. ✅ Test mit einfachen OBJ-Dateien

### Phase 3: Camera System (Tag 3-4)
1. ✅ Camera Basisklasse
2. ✅ FPSCamera Implementierung
3. ✅ OrbitalCamera Implementierung
4. ✅ CameraController Integration
5. ✅ Input-Mapping für Kamera-Steuerung

### Phase 4: Player Controller (Tag 4-5)
1. ✅ CharacterControllerComponent
2. ✅ PlayerControllerSystem
3. ✅ Input-Integration (WASD)
4. ✅ Basis-Physik (Gravity, Jump)
5. ✅ Ground-Detection

### Phase 5: 3D Rendering Verbesserungen (Tag 5-6)
1. ✅ MeshComponent
2. ✅ RenderSystem3D
3. ✅ Lighting System (Directional Light)
4. ✅ Material System
5. ✅ Shader für 3D-Rendering

### Phase 6: ImGui Integration (Tag 6-7)
1. ✅ DebugWindow Grundstruktur
2. ✅ Performance Window (FPS, Frame Time)
3. ✅ Camera Info Window
4. ✅ Entity List/Inspector
5. ✅ Render Statistics
6. ✅ Live-Controls für Lighting

## 4. Technische Details

### 4.1 Shader-Requirements

#### Vertex Shader (3D)
```glsl
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    FragPos = vec3(uModel * vec4(aPosition, 1.0));
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    TexCoords = aTexCoords;
    
    gl_Position = uProjection * uView * vec4(FragPos, 1.0);
}
```

#### Fragment Shader (Basic Lighting)
```glsl
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uDiffuseColor;
uniform vec3 uViewPos;

void main() {
    // Ambient
    vec3 ambient = uAmbientColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // Result
    vec3 result = (ambient + diffuse) * uDiffuseColor;
    FragColor = vec4(result, 1.0);
}
```

### 4.2 Vertex Layout (3D)

```cpp
// Position (3 floats) = 12 bytes
// Normal (3 floats)   = 12 bytes
// TexCoords (2 floats)= 8 bytes
// Color (3 floats)    = 12 bytes (optional)
// Total per vertex    = 44 bytes (with color)

layout.Push<float>(3); // position
layout.Push<float>(3); // normal
layout.Push<float>(2); // texcoords
layout.Push<float>(3); // color (optional)
```

### 4.3 Input-Konfiguration

**Camera Controls:**
- W/A/S/D: Bewegung
- Maus: Look Around
- Mausrad: Zoom/FOV
- Leertaste: Hoch
- Shift: Runter
- Tab: Kamera-Modus wechseln

**Player Controls:**
- W/A/S/D: Bewegung
- Leertaste: Springen
- Shift: Sprinten
- Maus: Kamera (wenn nicht captured)

## 5. Asset-Struktur

```
res/
├── models/
│   ├── cube.obj
│   ├── sphere.obj
│   ├── player.obj
│   └── level.obj
├── shaders/
│   ├── basic3d.shader
│   ├── lighting.shader
│   └── wireframe.shader
└── textures/
    └── (existing textures)
```

## 6. Performance-Ziele

- **FPS:** 60+ bei 100k Vertices
- **Draw Calls:** < 100 pro Frame
- **Memory:** < 500 MB für typische Szene
- **Load Time:** < 1s für OBJ bis 1MB

## 7. Testing-Szenarien

### Scenario 1: Basis 3D Rendering
- Würfel, Kugel, Ebene rendern
- FPS-Kamera umherfliegen
- Lighting funktioniert

### Scenario 2: OBJ Loading
- Lade einfaches OBJ (Teapot, Bunny)
- Korrekte Normalen
- Orbital-Kamera betrachten

### Scenario 3: Player Movement
- Entity mit CharacterController
- WASD-Bewegung
- Springen funktioniert
- Gravity korrekt

### Scenario 4: ImGui Integration
- Alle Debug-Fenster funktionieren
- Live-Editing von Transform
- FPS-Graph aktualisiert
- Entity-Selektion

## 8. Erweiterungsmöglichkeiten (Future)

- **Skeletal Animation** (.fbx Support)
- **Shadow Mapping**
- **Deferred Rendering**
- **Point Lights & Spot Lights**
- **PBR Materials**
- **Skybox**
- **Particle System (3D)**
- **LOD System**
- **Frustum Culling**
- **Occlusion Culling**

## 9. Abhängigkeiten

**Bereits vorhanden:**
- OpenGL 3.3+
- GLFW
- GLAD
- GLM
- ImGui

**Neu benötigt:**
- Keine! (Alles kann mit Standard-C++ und vorhandenen Libs implementiert werden)

## 10. Zusammenfassung

Dieses Projekt erweitert die bestehende 2D-Engine zu einer vollwertigen 3D-Engine durch:

1. **Mesh System** - Laden und Rendern von 3D-Modellen
2. **Camera System** - Flexible Kamera-Modi für verschiedene Anwendungsfälle
3. **Player Controller** - Physik-basierte Charaktersteuerung
4. **3D Rendering** - Moderne Shader-Pipeline mit Lighting
5. **ImGui Debug-Interface** - Vollständige Engine-Introspection

Die Implementierung nutzt die bestehende Infrastruktur maximal aus und fügt nur die notwendigen 3D-spezifischen Komponenten hinzu. Das Ergebnis ist eine flexible, erweiterbare 3D-Engine mit professionellen Development-Tools.

**Geschätzter Zeitaufwand:** 5-7 Arbeitstage für vollständige Implementierung
**LOC-Schätzung:** ~3,000-4,000 neue Zeilen Code
