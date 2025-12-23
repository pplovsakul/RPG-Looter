#pragma once

#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include "vendor/glm/glm.hpp"
#include "AABB.h"
#include "Triangle.h"

/**
 * Octree - Hierarchische räumliche Datenstruktur für Kollisionserkennung
 * 
 * Ein Octree teilt den 3D-Raum rekursiv in 8 gleichgroße Kinder auf.
 * Dies ermöglicht eine effiziente Broad-Phase-Kollisionserkennung durch
 * schnelles Ausschließen von Bereichen ohne mögliche Kollisionen.
 * 
 * Eigenschaften:
 * - Jeder Knoten hat eine AABB, die seinen Bereich definiert
 * - Blattknoten enthalten Dreieck-Indizes
 * - Innere Knoten haben 8 Kindknoten
 * - Maximale Tiefe und minimale Dreieckszahl pro Knoten sind konfigurierbar
 * 
 * Indizes der 8 Kinder (basierend auf Position relativ zum Zentrum):
 * 0: -X, -Y, -Z    4: +X, -Y, -Z
 * 1: -X, +Y, -Z    5: +X, +Y, -Z
 * 2: -X, -Y, +Z    6: +X, -Y, +Z
 * 3: -X, +Y, +Z    7: +X, +Y, +Z
 */
class OctreeNode
{
public:
    static constexpr int MAX_DEPTH = 8;           // Maximale Rekursionstiefe
    static constexpr int MIN_TRIANGLES = 4;       // Minimale Dreiecke zum Aufteilen
    static constexpr int MAX_TRIANGLES_LEAF = 16; // Max Dreiecke in einem Blattknoten

    OctreeNode(const AABB& bounds, int depth = 0);
    ~OctreeNode() = default;

    // Fügt einen Dreieck-Index hinzu und teilt bei Bedarf auf
    void Insert(size_t triangleIndex, const Triangle& triangle);

    // Gibt alle Dreieck-Indizes zurück, die mit der AABB kollidieren könnten
    void Query(const AABB& queryBounds, std::vector<size_t>& outIndices) const;

    // Gibt alle Dreieck-Indizes in diesem Knoten und seinen Kindern zurück
    void GetAllTriangles(std::vector<size_t>& outIndices) const;

    // Getters
    const AABB& GetBounds() const { return m_bounds; }
    bool IsLeaf() const { return m_children[0] == nullptr; }
    int GetDepth() const { return m_depth; }
    size_t GetTriangleCount() const { return m_triangleIndices.size(); }

private:
    // Teilt diesen Knoten in 8 Kindknoten auf
    void Subdivide();

    // Bestimmt welche Kind-Indizes ein Dreieck überlappen kann
    std::vector<int> GetChildIndicesForTriangle(const Triangle& triangle) const;

    // Berechnet die AABB für einen bestimmten Kindknoten
    AABB GetChildBounds(int childIndex) const;

    AABB m_bounds;                                      // Begrenzung dieses Knotens
    int m_depth;                                        // Tiefe im Baum (0 = Wurzel)
    std::vector<size_t> m_triangleIndices;              // Dreieck-Indizes in diesem Knoten
    std::array<std::unique_ptr<OctreeNode>, 8> m_children; // 8 Kindknoten
    std::vector<Triangle> m_tempTriangles;              // Temporäre Dreiecke für Subdivision
};

/**
 * Octree - Hauptklasse für die Octree-Datenstruktur
 * 
 * Diese Klasse verwaltet den Wurzelknoten und bietet eine einfache
 * Schnittstelle zum Aufbauen und Abfragen des Octrees.
 */
class Octree
{
public:
    Octree() = default;
    ~Octree() = default;

    /**
     * Baut den Octree aus Mesh-Daten auf
     * 
     * @param vertices Interleaved Vertex-Daten (x, y, z, u, v, r, g, b)
     * @param indices  Dreieck-Indizes
     * @param stride   Anzahl Floats pro Vertex (Standard: 8)
     */
    void Build(const std::vector<float>& vertices,
               const std::vector<unsigned int>& indices,
               size_t stride = 8);

    /**
     * Gibt alle Dreieck-Indizes zurück, die mit der AABB kollidieren könnten
     */
    void Query(const AABB& queryBounds, std::vector<size_t>& outTriangleIndices) const;

    /**
     * Gibt die Dreiecke zurück
     */
    const std::vector<Triangle>& GetTriangles() const { return m_triangles; }

    /**
     * Gibt die AABB des gesamten Meshes zurück
     */
    const AABB& GetBounds() const { return m_bounds; }

    /**
     * Prüft ob der Octree aufgebaut wurde
     */
    bool IsBuilt() const { return m_root != nullptr; }

    /**
     * Löscht den Octree
     */
    void Clear();

private:
    std::unique_ptr<OctreeNode> m_root;   // Wurzelknoten
    std::vector<Triangle> m_triangles;     // Alle Dreiecke des Meshes
    AABB m_bounds;                         // Gesamte AABB des Meshes
};

