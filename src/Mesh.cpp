#include "Mesh.h"
#include <iostream>

Mesh::Mesh()
    : m_vao(0), m_vbo(0), m_ebo(0), m_indexCount(0), m_hasGL(false)
{
}

Mesh::Mesh(const OBJLoader::MeshData& data)
    : Mesh()
{
    SetData(data);
}

Mesh::~Mesh()
{
    DestroyGL();
}

void Mesh::SetData(const OBJLoader::MeshData& data)
{
    m_vertices = data.vertices;
    m_indices = data.indices;
    m_indexCount = static_cast<GLsizei>(m_indices.size());
    // Falls bereits GL-Objekte existieren, neu aufbauen
    if (m_hasGL)
    {
        // Einfach neu erstellen
        DestroyGL();
        SetupGL();
    }
}

bool Mesh::SetupGL()
{
    if (m_vertices.empty() || m_indices.empty())
    {
        std::cerr << "Mesh::SetupGL: Keine Geometriedaten vorhanden.\n";
        return false;
    }

    // Erwarte, dass GL-Kontext und glad bereits initialisiert sind
    // Generiere VAO/VBO/EBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        m_vertices.size() * sizeof(float),
        m_vertices.data(),
        GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        m_indices.size() * sizeof(unsigned int),
        m_indices.data(),
        GL_STATIC_DRAW);

    // Vertex-Layout: aktuell nur Position (location = 0) mit 3 floats (x,y,z)
    constexpr GLsizei stride = 3 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));

    // Unbind VAO (EBO bleibt an VAO gebunden)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_indexCount = static_cast<GLsizei>(m_indices.size());
    m_hasGL = true;

    return true;
}

void Mesh::Draw() const
{
    if (!m_hasGL)
    {
        std::cerr << "Mesh::Draw: GL-Objekte nicht erzeugt. Rufe SetupGL() vorher auf.\n";
        return;
    }
    if (m_indexCount == 0)
        return;

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::DestroyGL()
{
    if (!m_hasGL)
        return;

    CleanupGLHandles();
    m_hasGL = false;
}

void Mesh::CleanupGLHandles()
{
    if (m_ebo != 0) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    if (m_vbo != 0) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao != 0) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_indexCount = 0;
}