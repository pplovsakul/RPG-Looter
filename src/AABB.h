#pragma once

#include <vector>
#include <limits>
#include "vendor/glm/glm.hpp"

/**
 * AABB (Axis-Aligned Bounding Box) - Achsenparallele Hüllbox
 * 
 * Die AABB wird durch zwei Punkte definiert:
 * - min: Punkt mit den kleinsten Koordinaten in allen drei Achsen
 * - max: Punkt mit den größten Koordinaten in allen drei Achsen
 * 
 * Diese einfache Struktur ermöglicht sehr schnelle Überschneidungstests
 * und ist ideal als erste Stufe der Kollisionserkennung (Broad Phase).
 */
struct AABB
{
    glm::vec3 min;  // Kleinste Koordinaten (-X, -Y, -Z Ecke)
    glm::vec3 max;  // Größte Koordinaten (+X, +Y, +Z Ecke)

    // Standard-Konstruktor: Initialisiert eine "umgekehrte" AABB,
    // die beim Hinzufügen von Punkten automatisch korrekt wächst
    AABB()
        : min(std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max())
        , max(std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest(),
              std::numeric_limits<float>::lowest())
    {
    }

    // Konstruktor mit expliziten Grenzen
    AABB(const glm::vec3& minCorner, const glm::vec3& maxCorner)
        : min(minCorner)
        , max(maxCorner)
    {
    }

    /**
     * Prüft, ob diese AABB gültige Grenzen hat
     * Eine AABB ist ungültig, wenn min > max in einer Achse
     */
    bool IsValid() const
    {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    /**
     * Berechnet das Zentrum der AABB
     */
    glm::vec3 GetCenter() const
    {
        return (min + max) * 0.5f;
    }

    /**
     * Berechnet die Ausdehnung (Größe) der AABB
     */
    glm::vec3 GetExtents() const
    {
        return max - min;
    }

    /**
     * Berechnet das halbe Ausmaß der AABB (Halbachsenlängen)
     */
    glm::vec3 GetHalfExtents() const
    {
        return (max - min) * 0.5f;
    }

    /**
     * Erweitert die AABB um einen einzelnen Punkt
     */
    void ExpandToInclude(const glm::vec3& point)
    {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    /**
     * Erweitert die AABB um eine andere AABB
     */
    void ExpandToInclude(const AABB& other)
    {
        ExpandToInclude(other.min);
        ExpandToInclude(other.max);
    }

    /**
     * Transformiert die AABB mit einer Position (Translation)
     * Gibt eine neue, transformierte AABB zurück
     */
    AABB Transformed(const glm::vec3& position) const
    {
        return AABB(min + position, max + position);
    }
};

/**
 * Namespace für AABB-Hilfsfunktionen
 */
namespace AABBUtils
{
    /**
     * Berechnet die AABB für ein Mesh aus interleaved Vertex-Daten
     * 
     * @param vertices Interleaved Vertex-Daten (x, y, z, u, v, r, g, b pro Vertex)
     * @param stride   Anzahl der Floats pro Vertex (Standard: 8 für Position + TexCoord + Color)
     * @return         Die berechnete AABB
     * 
     * Das Format entspricht dem in Mesh.cpp verwendeten Layout:
     * - Offset 0: Position (vec3: x, y, z)
     * - Offset 3: Texture Coordinates (vec2: u, v)
     * - Offset 5: Color (vec3: r, g, b)
     */
    inline AABB ComputeFromVertices(const std::vector<float>& vertices, size_t stride = 8)
    {
        AABB result;
        
        if (vertices.empty() || stride < 3)
        {
            return result; // Ungültige AABB zurückgeben
        }

        const size_t vertexCount = vertices.size() / stride;
        
        for (size_t i = 0; i < vertexCount; ++i)
        {
            const size_t baseIndex = i * stride;
            glm::vec3 position(
                vertices[baseIndex],
                vertices[baseIndex + 1],
                vertices[baseIndex + 2]
            );
            result.ExpandToInclude(position);
        }

        return result;
    }

    /**
     * Berechnet die AABB für ein einzelnes Dreieck
     * 
     * @param v0, v1, v2 Die drei Eckpunkte des Dreiecks
     * @return           Die AABB, die das Dreieck umschließt
     */
    inline AABB ComputeFromTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
    {
        AABB result;
        result.ExpandToInclude(v0);
        result.ExpandToInclude(v1);
        result.ExpandToInclude(v2);
        return result;
    }

    /**
     * AABB-AABB Kollisionserkennung (Überschneidungstest)
     * 
     * Zwei AABBs überschneiden sich, wenn sie sich in allen drei Achsen überlappen.
     * Dies ist ein sehr schneller Test (nur 6 Vergleiche).
     * 
     * @param a Erste AABB
     * @param b Zweite AABB
     * @return  true wenn die AABBs sich überschneiden, sonst false
     */
    inline bool Intersects(const AABB& a, const AABB& b)
    {
        // Separating Axis Test: Wenn es eine Achse gibt, bei der sich die
        // Projektionen nicht überlappen, dann gibt es keine Kollision.
        // 
        // Überlappung auf einer Achse: a.min <= b.max && b.min <= a.max
        
        if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
        if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
        if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
        
        return true;
    }

    /**
     * Prüft ob ein Punkt innerhalb einer AABB liegt
     * 
     * @param aabb Die AABB
     * @param point Der zu prüfende Punkt
     * @return true wenn der Punkt innerhalb der AABB liegt
     */
    inline bool ContainsPoint(const AABB& aabb, const glm::vec3& point)
    {
        return point.x >= aabb.min.x && point.x <= aabb.max.x &&
               point.y >= aabb.min.y && point.y <= aabb.max.y &&
               point.z >= aabb.min.z && point.z <= aabb.max.z;
    }

    /**
     * Prüft ob eine AABB vollständig in einer anderen enthalten ist
     * 
     * @param outer Die äußere AABB
     * @param inner Die innere AABB
     * @return true wenn inner vollständig in outer enthalten ist
     */
    inline bool Contains(const AABB& outer, const AABB& inner)
    {
        return inner.min.x >= outer.min.x && inner.max.x <= outer.max.x &&
               inner.min.y >= outer.min.y && inner.max.y <= outer.max.y &&
               inner.min.z >= outer.min.z && inner.max.z <= outer.max.z;
    }
}

