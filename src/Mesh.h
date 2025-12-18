#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <glad/glad.h>
#include "OBJLoader.h" // benutzt die vorhandene OBJLoader::MeshData

// Einfache Mesh-Klasse:
// - Speichert vertices (hier: interleaved x,y,z) und indices
// - Optional: Create GL buffers (VAO/VBO/EBO) und Draw()
// Hinweis: Wenn dein OBJ-Loader später Normals/Texcoords liefert, passe stride/Attribs an.
class Mesh
{
public:
    Mesh();
    Mesh(const OBJLoader::MeshData& data);
    ~Mesh();

    // Setzt die Rohdaten (kopiert)
    void SetData(const OBJLoader::MeshData& data);

    // Erzeuge OpenGL-Objekte (VAO/VBO/EBO). Erwartet, dass gl context + glad initialisiert sind.
    // Diese Methode richtet aktuell nur Attribut 0 (position: vec3) ein.
    // Rückgabe: true wenn erfolgreich (GL-Kontext vorhanden und Daten vorhanden)
    bool SetupGL();

    // Zeichne das Mesh (benötigt ein gebundenes Shader-Programm)
    void Draw() const;

    // Löscht die GL-Objekte (wird auch im Destruktor aufgerufen)
    void DestroyGL();

    // Zugriffe auf Rohdaten
    const std::vector<float>& GetVertices() const { return m_vertices; }
    const std::vector<unsigned int>& GetIndices() const { return m_indices; }

    bool HasGL() const { return m_hasGL; }
    bool IsValid() const { return !m_vertices.empty() && !m_indices.empty(); }

private:
    std::vector<float> m_vertices;         // interleaved vertex attributes (aktuell: x,y,z)
    std::vector<unsigned int> m_indices;

    // GL handles
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    GLsizei m_indexCount;
    bool m_hasGL;

    // interne Helfer
    void CleanupGLHandles();
};