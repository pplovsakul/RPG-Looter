#include "CollisionSystem.h"
#include "Mesh.h"
#include <iostream>
#include <algorithm>

// =============================================================================
// CollisionObject Implementation
// =============================================================================

CollisionObject::CollisionObject()
    : m_position(0.0f)
    , m_initialized(false)
{
}

void CollisionObject::Initialize(const std::vector<float>& vertices,
                                  const std::vector<unsigned int>& indices,
                                  size_t stride)
{
    if (vertices.empty() || indices.empty())
    {
        std::cerr << "CollisionObject::Initialize: Leere Daten.\n";
        m_initialized = false;
        return;
    }

    // Berechne die lokale AABB
    m_localAABB = AABBUtils::ComputeFromVertices(vertices, stride);

    // Baue den Octree auf
    m_octree.Build(vertices, indices, stride);

    m_initialized = true;
}

void CollisionObject::InitializeFromMesh(const Mesh& mesh)
{
    Initialize(mesh.GetVertices(), mesh.GetIndices());
}

void CollisionObject::SetPosition(const glm::vec3& position)
{
    m_position = position;
}

AABB CollisionObject::GetWorldAABB() const
{
    return m_localAABB.Transformed(m_position);
}

// =============================================================================
// CollisionSystem Implementation
// =============================================================================

CollisionSystem::CollisionSystem()
    : m_nextObjectId(0)
{
}

size_t CollisionSystem::AddObject(const std::vector<float>& vertices,
                                   const std::vector<unsigned int>& indices,
                                   size_t stride)
{
    auto object = std::make_unique<CollisionObject>();
    object->Initialize(vertices, indices, stride);

    if (!object->IsInitialized())
    {
        std::cerr << "CollisionSystem::AddObject: Initialisierung fehlgeschlagen.\n";
        return SIZE_MAX; // Ungültige ID
    }

    size_t objectId = m_nextObjectId++;
    m_objects[objectId] = std::move(object);
    
    return objectId;
}

size_t CollisionSystem::AddObject(const Mesh& mesh)
{
    return AddObject(mesh.GetVertices(), mesh.GetIndices());
}

void CollisionSystem::RemoveObject(size_t objectId)
{
    m_objects.erase(objectId);
}

void CollisionSystem::UpdatePosition(size_t objectId, const glm::vec3& position)
{
    CollisionObject* obj = GetObject(objectId);
    if (obj)
    {
        obj->SetPosition(position);
    }
}

glm::vec3 CollisionSystem::GetPosition(size_t objectId) const
{
    const CollisionObject* obj = GetObject(objectId);
    if (obj)
    {
        return obj->GetPosition();
    }
    return glm::vec3(0.0f);
}

bool CollisionSystem::CheckAABBCollision(size_t objectIdA, size_t objectIdB) const
{
    const CollisionObject* objA = GetObject(objectIdA);
    const CollisionObject* objB = GetObject(objectIdB);

    if (!objA || !objB)
    {
        return false;
    }

    return AABBUtils::Intersects(objA->GetWorldAABB(), objB->GetWorldAABB());
}

bool CollisionSystem::CheckCollision(size_t objectIdA, size_t objectIdB) const
{
    const CollisionObject* objA = GetObject(objectIdA);
    const CollisionObject* objB = GetObject(objectIdB);

    if (!objA || !objB)
    {
        return false;
    }

    // Schritt 1: Broad-Phase - AABB-Test
    if (!AABBUtils::Intersects(objA->GetWorldAABB(), objB->GetWorldAABB()))
    {
        return false; // Keine Kollision möglich
    }

    // Schritt 2+3: Mid-Phase und Narrow-Phase - Octree und Dreieck-Tests
    return CheckOctreeCollision(*objA, *objB);
}

CollisionInfo CollisionSystem::GetDetailedCollision(size_t objectIdA, size_t objectIdB) const
{
    CollisionInfo info;
    info.hasCollision = false;

    const CollisionObject* objA = GetObject(objectIdA);
    const CollisionObject* objB = GetObject(objectIdB);

    if (!objA || !objB)
    {
        return info;
    }

    // Broad-Phase
    AABB aabbA = objA->GetWorldAABB();
    AABB aabbB = objB->GetWorldAABB();
    
    if (!AABBUtils::Intersects(aabbA, aabbB))
    {
        return info;
    }

    // Mid-Phase: Finde potenziell kollidierende Dreiecke über Octree
    const Octree& octreeA = objA->GetOctree();
    const Octree& octreeB = objB->GetOctree();

    // Transformiere die Query-AABB in das lokale Koordinatensystem
    AABB queryForA(aabbB.min - objA->GetPosition(), aabbB.max - objA->GetPosition());
    AABB queryForB(aabbA.min - objB->GetPosition(), aabbA.max - objB->GetPosition());

    std::vector<size_t> triangleIndicesA;
    std::vector<size_t> triangleIndicesB;

    octreeA.Query(queryForA, triangleIndicesA);
    octreeB.Query(queryForB, triangleIndicesB);

    // Narrow-Phase: Dreieck-gegen-Dreieck Tests mit Detail-Rückgabe
    CheckTriangleCollision(*objA, *objB, triangleIndicesA, triangleIndicesB, &info);

    return info;
}

std::vector<size_t> CollisionSystem::CheckCollisionAgainstAll(size_t objectId) const
{
    std::vector<size_t> collidingObjects;

    for (const auto& pair : m_objects)
    {
        if (pair.first != objectId)
        {
            if (CheckCollision(objectId, pair.first))
            {
                collidingObjects.push_back(pair.first);
            }
        }
    }

    return collidingObjects;
}

AABB CollisionSystem::GetObjectAABB(size_t objectId) const
{
    const CollisionObject* obj = GetObject(objectId);
    if (obj)
    {
        return obj->GetWorldAABB();
    }
    return AABB();
}

void CollisionSystem::Clear()
{
    m_objects.clear();
    m_nextObjectId = 0;
}

const CollisionObject* CollisionSystem::GetObject(size_t objectId) const
{
    auto it = m_objects.find(objectId);
    if (it != m_objects.end())
    {
        return it->second.get();
    }
    return nullptr;
}

CollisionObject* CollisionSystem::GetObject(size_t objectId)
{
    auto it = m_objects.find(objectId);
    if (it != m_objects.end())
    {
        return it->second.get();
    }
    return nullptr;
}

bool CollisionSystem::CheckOctreeCollision(const CollisionObject& objA,
                                            const CollisionObject& objB) const
{
    const Octree& octreeA = objA.GetOctree();
    const Octree& octreeB = objB.GetOctree();

    if (!octreeA.IsBuilt() || !octreeB.IsBuilt())
    {
        return false;
    }

    // Transformiere die Welt-AABB von B in das lokale Koordinatensystem von A
    AABB worldAABB_B = objB.GetWorldAABB();
    AABB queryForA(worldAABB_B.min - objA.GetPosition(), 
                   worldAABB_B.max - objA.GetPosition());

    // Finde alle Dreiecke in A, die potenziell mit B kollidieren
    std::vector<size_t> triangleIndicesA;
    octreeA.Query(queryForA, triangleIndicesA);

    if (triangleIndicesA.empty())
    {
        return false;
    }

    // Transformiere die Welt-AABB von A in das lokale Koordinatensystem von B
    AABB worldAABB_A = objA.GetWorldAABB();
    AABB queryForB(worldAABB_A.min - objB.GetPosition(),
                   worldAABB_A.max - objB.GetPosition());

    // Finde alle Dreiecke in B, die potenziell mit A kollidieren
    std::vector<size_t> triangleIndicesB;
    octreeB.Query(queryForB, triangleIndicesB);

    if (triangleIndicesB.empty())
    {
        return false;
    }

    // Narrow-Phase: Teste alle Dreieck-Paare
    return CheckTriangleCollision(objA, objB, triangleIndicesA, triangleIndicesB);
}

bool CollisionSystem::CheckTriangleCollision(const CollisionObject& objA,
                                              const CollisionObject& objB,
                                              const std::vector<size_t>& triangleIndicesA,
                                              const std::vector<size_t>& triangleIndicesB,
                                              CollisionInfo* outInfo) const
{
    const Octree& octreeA = objA.GetOctree();
    const Octree& octreeB = objB.GetOctree();
    const std::vector<Triangle>& trianglesA = octreeA.GetTriangles();
    const std::vector<Triangle>& trianglesB = octreeB.GetTriangles();

    glm::vec3 posA = objA.GetPosition();
    glm::vec3 posB = objB.GetPosition();

    // Teste alle Dreieck-Paare
    for (size_t idxA : triangleIndicesA)
    {
        if (idxA >= trianglesA.size()) continue;

        // Transformiere Dreieck A in Weltkoordinaten
        const Triangle& localTriA = trianglesA[idxA];
        Triangle worldTriA(
            localTriA.v0 + posA,
            localTriA.v1 + posA,
            localTriA.v2 + posA
        );

        for (size_t idxB : triangleIndicesB)
        {
            if (idxB >= trianglesB.size()) continue;

            // Transformiere Dreieck B in Weltkoordinaten
            const Triangle& localTriB = trianglesB[idxB];
            Triangle worldTriB(
                localTriB.v0 + posB,
                localTriB.v1 + posB,
                localTriB.v2 + posB
            );

            // Schneller AABB-Test für die Dreiecke
            AABB triAABB_A = AABBUtils::ComputeFromTriangle(worldTriA.v0, worldTriA.v1, worldTriA.v2);
            AABB triAABB_B = AABBUtils::ComputeFromTriangle(worldTriB.v0, worldTriB.v1, worldTriB.v2);

            if (!AABBUtils::Intersects(triAABB_A, triAABB_B))
            {
                continue; // Diese Dreiecke können nicht kollidieren
            }

            // Präziser Dreieck-gegen-Dreieck Test
            if (TriangleCollision::Intersects(worldTriA, worldTriB))
            {
                if (outInfo)
                {
                    outInfo->hasCollision = true;
                    outInfo->triangleIndexA = idxA;
                    outInfo->triangleIndexB = idxB;
                    
                    // Berechne einen ungefähren Kollisionspunkt
                    outInfo->collisionPoint = (worldTriA.GetCentroid() + worldTriB.GetCentroid()) * 0.5f;
                    
                    // Verwende die Normale von Dreieck B als Kollisionsnormale
                    outInfo->collisionNormal = worldTriB.GetNormalizedNormal();
                    
                    // Schätze die Eindringtiefe (vereinfacht)
                    glm::vec3 centerDiff = worldTriA.GetCentroid() - worldTriB.GetCentroid();
                    outInfo->penetrationDepth = std::abs(glm::dot(centerDiff, outInfo->collisionNormal));
                }
                return true;
            }
        }
    }

    return false;
}

