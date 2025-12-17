# Anleitung: cube.obj mit der Mesh-Klasse rendern

Diese Anleitung zeigt, wie Sie die neue Mesh-Klasse und den OBJ-Parser verwenden, um `cube.obj` zu rendern.

## Schnellstart - Cube.obj in main.cpp rendern

### Schritt 1: Includes hinzufügen

Fügen Sie diese Zeilen am Anfang Ihrer `main.cpp` hinzu:

```cpp
#include "Mesh.h"
#include "ObjParser.h"
```

### Schritt 2: Mesh laden

Ersetzen Sie die bestehenden Vertex- und Index-Arrays durch das Laden der OBJ-Datei:

**VORHER** (alte manuelle Vertex-Daten):
```cpp
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
    // ...
};
unsigned int indices[] = {
    0, 1, 2,
    // ...
};
```

**NACHHER** (mit OBJ-Parser):
```cpp
// Mesh laden
Mesh mesh;
bool success = ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);

if (!success) {
    std::cerr << "Fehler beim Laden von cube.obj!" << std::endl;
    return -1;
}

std::cout << "Mesh erfolgreich geladen:" << std::endl;
std::cout << "  - Vertices: " << mesh.GetVertexCount() << std::endl;
std::cout << "  - Faces: " << mesh.GetFaceCount() << std::endl;
std::cout << "  - Materials: " << mesh.GetMaterialCount() << std::endl;

// Daten für OpenGL vorbereiten
std::vector<float> vertexData = mesh.GetInterleavedVertexData();
std::vector<unsigned int> indexData = mesh.GetIndexData();
```

### Schritt 3: Buffers erstellen

Passen Sie die Buffer-Erstellung an:

**VORHER**:
```cpp
VertexBuffer vb(vertices, sizeof(vertices));
IndexBuffer ib(indices, sizeof(indices));
```

**NACHHER**:
```cpp
VertexBuffer vb(vertexData.data(), vertexData.size() * sizeof(float));
IndexBuffer ib(indexData.data(), indexData.size() * sizeof(unsigned int));
```

### Schritt 4: Vertex Layout anpassen

Das geladene Mesh enthält mehr Daten (Position + Normal + Texcoord):

**VORHER**:
```cpp
VertexBufferLayout layout;
layout.AddFloat(3); // Nur Position
va.AddBuffer(vb, layout);
```

**NACHHER**:
```cpp
VertexBufferLayout layout;
layout.AddFloat(3);  // Position (x, y, z)
layout.AddFloat(3);  // Normal (nx, ny, nz)
layout.AddFloat(2);  // Texture coordinates (u, v)
va.AddBuffer(vb, layout);
```

### Schritt 5: (Optional) Material-Farben verwenden

Sie können die Material-Farben aus der MTL-Datei verwenden:

```cpp
// Im Render-Loop, vor dem Zeichnen:
const Material* mat = mesh.GetMaterial("RedMaterial");
if (mat) {
    shader.SetUniform4f("u_Color", 
        mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], 1.0f);
} else {
    shader.SetUniform4f("u_Color", 1.0f, 0.5f, 0.2f, 1.0f);
}

renderer.Draw(va, ib, shader);
```

## Vollständiges Beispiel

Ein vollständiges Beispiel finden Sie in `docs/integration_example.cpp`.

## Mehrere Meshes mit unterschiedlichen Materialien rendern

Wenn Sie mehrere Objekte mit verschiedenen Materialien rendern möchten:

```cpp
// Mesh laden
Mesh mesh;
ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);

// Vertex/Index Daten erstellen
std::vector<float> vertexData = mesh.GetInterleavedVertexData();
std::vector<unsigned int> indexData = mesh.GetIndexData();

VertexBuffer vb(vertexData.data(), vertexData.size() * sizeof(float));
IndexBuffer ib(indexData.data(), indexData.size() * sizeof(unsigned int));

// Layout Setup
VertexArray va;
VertexBufferLayout layout;
layout.AddFloat(3);  // Position
layout.AddFloat(3);  // Normal
layout.AddFloat(2);  // TexCoord
va.AddBuffer(vb, layout);

// Im Render-Loop
shader.Bind();
shader.SetUniformMat4f("u_MVP", mvp);

// Durch alle Faces iterieren und nach Material gruppieren
const std::vector<Face>& faces = mesh.GetFaces();
std::string currentMaterial = "";

for (size_t i = 0; i < faces.size(); ++i) {
    const Face& face = faces[i];
    
    // Material wechseln wenn nötig
    if (face.materialName != currentMaterial) {
        currentMaterial = face.materialName;
        const Material* mat = mesh.GetMaterial(currentMaterial);
        if (mat) {
            shader.SetUniform4f("u_Color", 
                mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], 1.0f);
        }
    }
    
    // Einzelnes Face zeichnen (3 Indices)
    // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(i * 3 * sizeof(unsigned int)));
}

// Oder einfach alles auf einmal zeichnen:
renderer.Draw(va, ib, shader);
```

## Eigene OBJ-Dateien verwenden

Sie können auch Ihre eigenen OBJ-Dateien laden:

```cpp
Mesh myMesh;
if (ObjParser::ParseOBJ("pfad/zu/meinem/model.obj", myMesh, true)) {
    // Mesh wurde erfolgreich geladen
    // Jetzt wie oben verwenden
}
```

**Wichtig**: Die MTL-Datei muss im gleichen Verzeichnis wie die OBJ-Datei liegen!

## Debugging-Tipps

Falls das Laden nicht funktioniert:

```cpp
Mesh mesh;
bool success = ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);

if (!success) {
    std::cerr << "Laden fehlgeschlagen!" << std::endl;
    // Prüfen Sie:
    // - Existiert die Datei?
    // - Ist der Pfad korrekt?
    // - Liegt die MTL-Datei im gleichen Verzeichnis?
}

// Nach erfolgreichem Laden:
std::cout << "Vertices: " << mesh.GetVertexCount() << std::endl;
std::cout << "Faces: " << mesh.GetFaceCount() << std::endl;
std::cout << "Materials: " << mesh.GetMaterialCount() << std::endl;

// Material-Details anzeigen:
const std::map<std::string, Material>& materials = mesh.GetMaterials();
for (const auto& [name, mat] : materials) {
    std::cout << "Material: " << name << std::endl;
    std::cout << "  Diffuse: (" << mat.diffuse[0] << ", " 
              << mat.diffuse[1] << ", " << mat.diffuse[2] << ")" << std::endl;
}
```

## Weitere Beispiele

Weitere Code-Beispiele finden Sie in:
- `tests/test_mesh.cpp` - Unit Tests mit Beispielen
- `docs/mesh_usage_examples.cpp` - Verschiedene Verwendungsszenarien
- `docs/MESH_DOCUMENTATION.md` - Vollständige Dokumentation
