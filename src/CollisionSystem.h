#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include "vendor/glm/glm.hpp"
#include "AABB.h"
#include "Triangle.h"
#include "Octree.h"

// Forward declaration
class Mesh;

/**
 * CollisionInfo - Detaillierte Informationen über eine erkannte Kollision
 */
struct CollisionInfo
{
    bool hasCollision = false;              // Wurde eine Kollision erkannt?
    glm::vec3 collisionPoint{0.0f};         // Ungefährer Kollisionspunkt
    glm::vec3 collisionNormal{0.0f, 1.0f, 0.0f}; // Kollisionsnormale (für Physik-Response)
    float penetrationDepth = 0.0f;          // Eindringtiefe (für Physik-Response)
    size_t triangleIndexA = 0;              // Index des kollidierenden Dreiecks (Objekt A)
    size_t triangleIndexB = 0;              // Index des kollidierenden Dreiecks (Objekt B)
};

/**
 * CollisionObject - Wrapper für kollidierende Objekte
 * 
 * Speichert die notwendigen Daten für die Kollisionserkennung eines Objekts:
 * - Position (Transformation)
 * - AABB (für Broad-Phase)
 * - Octree (für hierarchische Narrow-Phase)
 */
class CollisionObject
{
public:
    CollisionObject();
    ~CollisionObject() = default;

    /**
     * Initialisiert das CollisionObject aus Mesh-Daten
     * 
     * @param vertices Interleaved Vertex-Daten (x, y, z, u, v, r, g, b)
     * @param indices  Dreieck-Indizes
     * @param stride   Anzahl Floats pro Vertex (Standard: 8)
     */
    void Initialize(const std::vector<float>& vertices,
                    const std::vector<unsigned int>& indices,
                    size_t stride = 8);

    /**
     * Initialisiert das CollisionObject aus einem Mesh
     */
    void InitializeFromMesh(const Mesh& mesh);

    /**
     * Aktualisiert die Position des Objekts
     */
    void SetPosition(const glm::vec3& position);
    const glm::vec3& GetPosition() const { return m_position; }

    /**
     * Gibt die lokale AABB zurück (ohne Position)
     */
    const AABB& GetLocalAABB() const { return m_localAABB; }

    /**
     * Gibt die Welt-AABB zurück (mit Position)
     */
    AABB GetWorldAABB() const;

    /**
     * Gibt den Octree zurück
     */
    const Octree& GetOctree() const { return m_octree; }

    /**
     * Prüft ob das Objekt initialisiert wurde
     */
    bool IsInitialized() const { return m_initialized; }

private:
    glm::vec3 m_position;          // Position in der Welt
    AABB m_localAABB;              // AABB im lokalen Koordinatensystem
    Octree m_octree;               // Octree für hierarchische Kollisionserkennung
    bool m_initialized;
};

/**
 * CollisionSystem - Hauptklasse für die Kollisionserkennung
 * 
 * Diese Klasse bietet eine modulare und erweiterbare Schnittstelle für
 * die Kollisionserkennung. Sie verwendet einen mehrschichtigen Ansatz:
 * 
 * 1. Broad-Phase: Schneller AABB-AABB Test um offensichtlich nicht 
 *    kollidierende Paare auszuschließen
 * 
 * 2. Mid-Phase: Octree-basierte hierarchische Verfeinerung um die Anzahl
 *    der zu testenden Dreieck-Paare zu reduzieren
 * 
 * 3. Narrow-Phase: Präzise Dreieck-gegen-Dreieck Tests für echte Kollisionen
 * 
 * Verwendung:
 * @code
 * CollisionSystem collisionSystem;
 * 
 * // Objekte hinzufügen
 * size_t playerId = collisionSystem.AddObject(playerMesh);
 * size_t obstacleId = collisionSystem.AddObject(obstacleMesh);
 * 
 * // Position aktualisieren
 * collisionSystem.UpdatePosition(playerId, playerPosition);
 * collisionSystem.UpdatePosition(obstacleId, obstaclePosition);
 * 
 * // Kollision prüfen
 * if (collisionSystem.CheckCollision(playerId, obstacleId))
 * {
 *     // Kollision behandeln
 *     CollisionInfo info = collisionSystem.GetDetailedCollision(playerId, obstacleId);
 * }
 * @endcode
 */
class CollisionSystem
{
public:
    CollisionSystem();
    ~CollisionSystem() = default;

    // =========================================================================
    // Objekt-Management
    // =========================================================================

    /**
     * Fügt ein neues Kollisionsobjekt hinzu
     * 
     * @param vertices Interleaved Vertex-Daten
     * @param indices  Dreieck-Indizes
     * @param stride   Floats pro Vertex
     * @return ID des hinzugefügten Objekts
     */
    size_t AddObject(const std::vector<float>& vertices,
                     const std::vector<unsigned int>& indices,
                     size_t stride = 8);

    /**
     * Fügt ein Kollisionsobjekt aus einem Mesh hinzu
     * 
     * @param mesh Das Mesh
     * @return ID des hinzugefügten Objekts
     */
    size_t AddObject(const Mesh& mesh);

    /**
     * Entfernt ein Kollisionsobjekt
     * 
     * @param objectId ID des zu entfernenden Objekts
     */
    void RemoveObject(size_t objectId);

    /**
     * Aktualisiert die Position eines Objekts
     */
    void UpdatePosition(size_t objectId, const glm::vec3& position);

    /**
     * Gibt die Position eines Objekts zurück
     */
    glm::vec3 GetPosition(size_t objectId) const;

    // =========================================================================
    // Kollisionserkennung
    // =========================================================================

    /**
     * Schneller AABB-AABB Kollisionstest (Broad-Phase)
     * 
     * @param objectIdA Erstes Objekt
     * @param objectIdB Zweites Objekt
     * @return true wenn die AABBs sich überschneiden
     */
    bool CheckAABBCollision(size_t objectIdA, size_t objectIdB) const;

    /**
     * Vollständiger Kollisionstest mit Octree und Dreieck-Tests
     * 
     * @param objectIdA Erstes Objekt
     * @param objectIdB Zweites Objekt
     * @return true wenn eine echte Kollision vorliegt
     */
    bool CheckCollision(size_t objectIdA, size_t objectIdB) const;

    /**
     * Effiziente Kollisionsprüfung mit optionaler Detail-Rückgabe
     * Vermeidet doppelte Arbeit wenn sowohl Kollision als auch Details benötigt werden
     * 
     * @param objectIdA Erstes Objekt
     * @param objectIdB Zweites Objekt
     * @param outInfo Optional: Zeiger auf CollisionInfo für Details (kann nullptr sein)
     * @return true wenn eine echte Kollision vorliegt
     */
    bool CheckCollisionWithInfo(size_t objectIdA, size_t objectIdB, CollisionInfo* outInfo) const;

    /**
     * Detaillierter Kollisionstest mit Rückgabe von Kollisionsinformationen
     * 
     * @param objectIdA Erstes Objekt
     * @param objectIdB Zweites Objekt
     * @return CollisionInfo mit Details zur Kollision
     */
    CollisionInfo GetDetailedCollision(size_t objectIdA, size_t objectIdB) const;

    /**
     * Prüft Kollisionen eines Objekts gegen alle anderen Objekte
     * 
     * @param objectId Das zu prüfende Objekt
     * @return Liste der IDs aller kollidierenden Objekte
     */
    std::vector<size_t> CheckCollisionAgainstAll(size_t objectId) const;

    // =========================================================================
    // Utility-Funktionen
    // =========================================================================

    /**
     * Gibt die Welt-AABB eines Objekts zurück
     */
    AABB GetObjectAABB(size_t objectId) const;

    /**
     * Gibt die Anzahl der registrierten Objekte zurück
     */
    size_t GetObjectCount() const { return m_objects.size(); }

    /**
     * Löscht alle Objekte
     */
    void Clear();

private:
    // Interne Hilfsfunktionen
    const CollisionObject* GetObject(size_t objectId) const;
    CollisionObject* GetObject(size_t objectId);

    // Führt die hierarchische Octree-basierte Kollisionserkennung durch
    bool CheckOctreeCollision(const CollisionObject& objA, 
                              const CollisionObject& objB,
                              CollisionInfo* outInfo = nullptr) const;

    // Führt Dreieck-gegen-Dreieck Tests für die gegebenen Indizes durch
    bool CheckTriangleCollision(const CollisionObject& objA,
                                const CollisionObject& objB,
                                const std::vector<size_t>& triangleIndicesA,
                                const std::vector<size_t>& triangleIndicesB,
                                CollisionInfo* outInfo = nullptr) const;

    std::unordered_map<size_t, std::unique_ptr<CollisionObject>> m_objects;
    size_t m_nextObjectId;
};

