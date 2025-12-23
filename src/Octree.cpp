#include "Octree.h"
#include <iostream>

// =============================================================================
// OctreeNode Implementation
// =============================================================================

OctreeNode::OctreeNode(const AABB& bounds, int depth)
    : m_bounds(bounds)
    , m_depth(depth)
{
    // Initialisiere alle Kindknoten-Pointer auf nullptr
    for (auto& child : m_children)
    {
        child = nullptr;
    }
}

void OctreeNode::Insert(size_t triangleIndex, const Triangle& triangle)
{
    // Speichere das Dreieck temporär für spätere Subdivision
    if (m_depth < MAX_DEPTH && m_tempTriangles.size() <= triangleIndex)
    {
        m_tempTriangles.resize(triangleIndex + 1);
    }
    if (triangleIndex < m_tempTriangles.size())
    {
        m_tempTriangles[triangleIndex] = triangle;
    }

    // Wenn dies ein Blattknoten ist
    if (IsLeaf())
    {
        m_triangleIndices.push_back(triangleIndex);

        // Prüfe ob wir aufteilen sollten
        if (m_triangleIndices.size() > MAX_TRIANGLES_LEAF && m_depth < MAX_DEPTH)
        {
            Subdivide();
        }
    }
    else
    {
        // Verteile das Dreieck auf die entsprechenden Kindknoten
        std::vector<int> childIndices = GetChildIndicesForTriangle(triangle);
        for (int childIndex : childIndices)
        {
            if (m_children[childIndex])
            {
                m_children[childIndex]->Insert(triangleIndex, triangle);
            }
        }
    }
}

void OctreeNode::Query(const AABB& queryBounds, std::vector<size_t>& outIndices) const
{
    // Schneller Ausschlusstest
    if (!AABBUtils::Intersects(m_bounds, queryBounds))
    {
        return;
    }

    if (IsLeaf())
    {
        // Füge alle Dreieck-Indizes dieses Blattknotens hinzu
        for (size_t index : m_triangleIndices)
        {
            outIndices.push_back(index);
        }
    }
    else
    {
        // Rekursiv alle Kindknoten abfragen
        for (const auto& child : m_children)
        {
            if (child)
            {
                child->Query(queryBounds, outIndices);
            }
        }
    }
}

void OctreeNode::GetAllTriangles(std::vector<size_t>& outIndices) const
{
    if (IsLeaf())
    {
        for (size_t index : m_triangleIndices)
        {
            outIndices.push_back(index);
        }
    }
    else
    {
        for (const auto& child : m_children)
        {
            if (child)
            {
                child->GetAllTriangles(outIndices);
            }
        }
    }
}

void OctreeNode::Subdivide()
{
    // Erstelle die 8 Kindknoten
    for (int i = 0; i < 8; ++i)
    {
        AABB childBounds = GetChildBounds(i);
        m_children[i] = std::make_unique<OctreeNode>(childBounds, m_depth + 1);
    }

    // Verteile die bestehenden Dreiecke auf die Kindknoten
    for (size_t triangleIndex : m_triangleIndices)
    {
        if (triangleIndex < m_tempTriangles.size())
        {
            const Triangle& triangle = m_tempTriangles[triangleIndex];
            std::vector<int> childIndices = GetChildIndicesForTriangle(triangle);
            for (int childIndex : childIndices)
            {
                if (m_children[childIndex])
                {
                    m_children[childIndex]->Insert(triangleIndex, triangle);
                }
            }
        }
    }

    // Lösche die Dreieck-Indizes in diesem Knoten (nur innere Knoten speichern keine)
    m_triangleIndices.clear();
    m_triangleIndices.shrink_to_fit();
    m_tempTriangles.clear();
    m_tempTriangles.shrink_to_fit();
}

std::vector<int> OctreeNode::GetChildIndicesForTriangle(const Triangle& triangle) const
{
    std::vector<int> result;
    glm::vec3 center = m_bounds.GetCenter();

    // Berechne die AABB des Dreiecks
    AABB triangleBounds = AABBUtils::ComputeFromTriangle(triangle.v0, triangle.v1, triangle.v2);

    // Prüfe jedes der 8 Kinder
    for (int i = 0; i < 8; ++i)
    {
        AABB childBounds = GetChildBounds(i);
        if (AABBUtils::Intersects(childBounds, triangleBounds))
        {
            result.push_back(i);
        }
    }

    return result;
}

AABB OctreeNode::GetChildBounds(int childIndex) const
{
    glm::vec3 center = m_bounds.GetCenter();
    glm::vec3 halfExtents = m_bounds.GetHalfExtents() * 0.5f;

    // Bestimme die Position des Kindes basierend auf dem Index
    // Bit 0: X-Achse (0 = negativ, 1 = positiv)
    // Bit 1: Y-Achse (0 = negativ, 1 = positiv)
    // Bit 2: Z-Achse (0 = negativ, 1 = positiv)
    // 
    // Korrigierte Zuordnung:
    // 0: -X, -Y, -Z    1: -X, +Y, -Z    2: -X, -Y, +Z    3: -X, +Y, +Z
    // 4: +X, -Y, -Z    5: +X, +Y, -Z    6: +X, -Y, +Z    7: +X, +Y, +Z
    glm::vec3 offset;
    offset.x = (childIndex & 4) ? halfExtents.x : -halfExtents.x;
    offset.y = (childIndex & 1) ? halfExtents.y : -halfExtents.y;
    offset.z = (childIndex & 2) ? halfExtents.z : -halfExtents.z;

    glm::vec3 childCenter = center + offset;
    return AABB(childCenter - halfExtents, childCenter + halfExtents);
}

// =============================================================================
// Octree Implementation
// =============================================================================

void Octree::Build(const std::vector<float>& vertices,
                   const std::vector<unsigned int>& indices,
                   size_t stride)
{
    Clear();

    if (vertices.empty() || indices.empty())
    {
        std::cerr << "Octree::Build: Leere Vertex- oder Index-Daten.\n";
        return;
    }

    // Berechne die Gesamt-AABB
    m_bounds = AABBUtils::ComputeFromVertices(vertices, stride);
    
    // Erweitere die AABB leicht, um numerische Probleme an den Grenzen zu vermeiden
    glm::vec3 padding(0.001f);
    m_bounds.min -= padding;
    m_bounds.max += padding;

    // Extrahiere alle Dreiecke
    const size_t triangleCount = indices.size() / 3;
    m_triangles.reserve(triangleCount);

    for (size_t i = 0; i < triangleCount; ++i)
    {
        size_t idx0 = indices[i * 3];
        size_t idx1 = indices[i * 3 + 1];
        size_t idx2 = indices[i * 3 + 2];

        glm::vec3 v0(
            vertices[idx0 * stride],
            vertices[idx0 * stride + 1],
            vertices[idx0 * stride + 2]
        );
        glm::vec3 v1(
            vertices[idx1 * stride],
            vertices[idx1 * stride + 1],
            vertices[idx1 * stride + 2]
        );
        glm::vec3 v2(
            vertices[idx2 * stride],
            vertices[idx2 * stride + 1],
            vertices[idx2 * stride + 2]
        );

        m_triangles.emplace_back(v0, v1, v2);
    }

    // Erstelle den Wurzelknoten
    m_root = std::make_unique<OctreeNode>(m_bounds, 0);

    // Füge alle Dreiecke ein
    for (size_t i = 0; i < m_triangles.size(); ++i)
    {
        m_root->Insert(i, m_triangles[i]);
    }
}

void Octree::Query(const AABB& queryBounds, std::vector<size_t>& outTriangleIndices) const
{
    if (!m_root)
    {
        return;
    }

    outTriangleIndices.clear();
    m_root->Query(queryBounds, outTriangleIndices);

    // Entferne Duplikate (Dreiecke können in mehreren Kindknoten sein)
    std::sort(outTriangleIndices.begin(), outTriangleIndices.end());
    outTriangleIndices.erase(
        std::unique(outTriangleIndices.begin(), outTriangleIndices.end()),
        outTriangleIndices.end()
    );
}

void Octree::Clear()
{
    m_root.reset();
    m_triangles.clear();
    m_bounds = AABB();
}

