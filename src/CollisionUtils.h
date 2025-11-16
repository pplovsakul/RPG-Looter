#pragma once

#include "Components.h"
#include "vendor/glm/glm.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace CollisionUtils {

// small local clamp to avoid relying on std::clamp availability
static inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Convert a shape into a world-space polygon (convex). Uses same order as renderer.
static inline std::vector<glm::vec2> ShapeToWorldPolygon(const ModelComponent::Shape& s, const TransformComponent& t) {
    const glm::vec2 localCorners[4] = {
        {-0.5f, -0.5f},
        { 0.5f, -0.5f},
        { 0.5f,  0.5f},
        {-0.5f,  0.5f}
    };

    glm::vec2 S = s.size * s.scale; // shape size in world units
    float totalRot = t.rotation + glm::radians(s.rotation);
    float cs = std::cos(totalRot), sn = std::sin(totalRot);

    std::vector<glm::vec2> out;
    if (s.type == ModelComponent::ShapeType::Triangle) {
        int triIdx[3] = {0,1,3};
        out.reserve(3);
        for (int k = 0; k < 3; ++k) {
            glm::vec2 v = localCorners[triIdx[k]] * S;
            glm::vec2 vr = glm::vec2(v.x * cs - v.y * sn, v.x * sn + v.y * cs);
            glm::vec2 world = vr + s.position + t.position;
            out.push_back(world);
        }
    } else {
        out.reserve(4);
        for (int k = 0; k < 4; ++k) {
            glm::vec2 v = localCorners[k] * S;
            glm::vec2 vr = glm::vec2(v.x * cs - v.y * sn, v.x * sn + v.y * cs);
            glm::vec2 world = vr + s.position + t.position;
            out.push_back(world);
        }
    }
    return out;
}

static inline bool ShapeToWorldCircle(const ModelComponent::Shape& s, const TransformComponent& t, glm::vec2& outCenter, float& outRadius) {
    if (s.type != ModelComponent::ShapeType::Circle) return false;
    outCenter = s.position + t.position;
    outRadius = (s.size.x * s.scale.x) * 0.5f;
    return true;
}

// Project polygon onto axis
static inline void ProjectPolygon(const std::vector<glm::vec2>& poly, const glm::vec2& axis, float& outMin, float& outMax) {
    outMin = outMax = glm::dot(poly[0], axis);
    for (size_t i = 1; i < poly.size(); ++i) {
        float p = glm::dot(poly[i], axis);
        if (p < outMin) outMin = p;
        if (p > outMax) outMax = p;
    }
}

// Intersection tests (SAT)
static inline bool PolygonIntersectPolygon(const std::vector<glm::vec2>& A, const std::vector<glm::vec2>& B) {
    auto testEdges = [](const std::vector<glm::vec2>& P, const std::vector<glm::vec2>& Q) -> bool {
        for (size_t i = 0; i < P.size(); ++i) {
            glm::vec2 a = P[i];
            glm::vec2 b = P[(i + 1) % P.size()];
            glm::vec2 edge = b - a;
            glm::vec2 axis = glm::normalize(glm::vec2(-edge.y, edge.x));

            float minA, maxA, minB, maxB;
            ProjectPolygon(P, axis, minA, maxA);
            ProjectPolygon(Q, axis, minB, maxB);

            if (maxA < minB || maxB < minA) return false; // separation found
        }
        return true;
    };

    if (A.empty() || B.empty()) return false;
    return testEdges(A,B) && testEdges(B,A);
}

static inline bool PolygonIntersectCircle(const std::vector<glm::vec2>& poly, const glm::vec2& center, float radius) {
    if (poly.empty()) return false;
    // Test against polygon edges
    for (size_t i = 0; i < poly.size(); ++i) {
        glm::vec2 a = poly[i];
        glm::vec2 b = poly[(i+1) % poly.size()];
        glm::vec2 edge = b - a;
        glm::vec2 axis = glm::normalize(glm::vec2(-edge.y, edge.x));

        float minP, maxP;
        ProjectPolygon(poly, axis, minP, maxP);
        float projCenter = glm::dot(center, axis);
        if (projCenter + radius < minP || projCenter - radius > maxP) return false;
    }

    // point in poly test (winding)
    bool inside = true;
    for (size_t i = 0; i < poly.size(); ++i) {
        glm::vec2 a = poly[i];
        glm::vec2 b = poly[(i+1) % poly.size()];
        glm::vec2 edge = b - a;
        glm::vec2 axis = glm::vec2(-edge.y, edge.x);
        if (glm::dot(axis, center - a) > 0) { inside = false; break; }
    }
    if (inside) return true;

    // distance to edges
    for (size_t i = 0; i < poly.size(); ++i) {
        glm::vec2 a = poly[i];
        glm::vec2 b = poly[(i+1) % poly.size()];
        glm::vec2 ab = b - a;
        float denom = glm::dot(ab, ab);
        if (denom == 0.0f) continue;
        float t = glm::dot(center - a, ab) / denom;
        t = clampf(t, 0.0f, 1.0f);
        glm::vec2 closest = a + ab * t;
        float dist2 = glm::dot(center - closest, center - closest);
        if (dist2 <= radius * radius) return true;
    }
    return false;
}

static inline bool CircleIntersectCircle(const glm::vec2& c1, float r1, const glm::vec2& c2, float r2) {
    float dx = c1.x - c2.x;
    float dy = c1.y - c2.y;
    float dist2 = dx*dx + dy*dy;
    float rsum = r1 + r2;
    return dist2 <= rsum * rsum;
}

// Helper: compute polygon centroid (approx average)
static inline glm::vec2 PolygonCentroid(const std::vector<glm::vec2>& poly) {
    glm::vec2 c(0.0f);
    for (const auto& v : poly) c += v;
    return c / (float)poly.size();
}

// Helper: closest point on segment ab to point p
static inline glm::vec2 ClosestPointOnSegment(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p) {
    glm::vec2 ab = b - a;
    float denom = glm::dot(ab, ab);
    if (denom == 0.0f) return a;
    float t = glm::dot(p - a, ab) / denom;
    t = clampf(t, 0.0f, 1.0f);
    return a + ab * t;
}

// Compute minimal translation vector to separate two convex polygons. Returns true if intersecting and sets outMTV (vector to move A out of B)
static inline bool PolygonPolygonMTV(const std::vector<glm::vec2>& A, const std::vector<glm::vec2>& B, glm::vec2& outMTV) {
    if (A.empty() || B.empty()) return false;
    float minOverlap = FLT_MAX;
    glm::vec2 smallestAxis(0.0f);

    auto testEdges = [&](const std::vector<glm::vec2>& P, const std::vector<glm::vec2>& Q) -> bool {
        for (size_t i = 0; i < P.size(); ++i) {
            glm::vec2 a = P[i];
            glm::vec2 b = P[(i + 1) % P.size()];
            glm::vec2 edge = b - a;
            glm::vec2 axis = glm::normalize(glm::vec2(-edge.y, edge.x));

            float minA, maxA, minB, maxB;
            ProjectPolygon(A, axis, minA, maxA);
            ProjectPolygon(B, axis, minB, maxB);

            float overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap <= 0.0f) return false; // separation
            if (overlap < minOverlap) { minOverlap = overlap; smallestAxis = axis; }
        }
        return true;
    };

    if (!testEdges(A,B)) return false;
    if (!testEdges(B,A)) return false;

    // determine correct direction: move A out of B
    glm::vec2 centerA = PolygonCentroid(A);
    glm::vec2 centerB = PolygonCentroid(B);
    glm::vec2 dir = centerA - centerB;
    if (glm::dot(dir, smallestAxis) < 0.0f) smallestAxis = -smallestAxis;
    outMTV = smallestAxis * minOverlap;
    return true;
}

// Polygon vs Circle MTV
static inline bool PolygonCircleMTV(const std::vector<glm::vec2>& poly, const glm::vec2& center, float radius, glm::vec2& outMTV) {
    if (poly.empty()) return false;
    float minOverlap = FLT_MAX;
    glm::vec2 smallestAxis(0.0f);

    // test polygon edge normals
    for (size_t i = 0; i < poly.size(); ++i) {
        glm::vec2 a = poly[i];
        glm::vec2 b = poly[(i+1) % poly.size()];
        glm::vec2 edge = b - a;
        glm::vec2 axis = glm::normalize(glm::vec2(-edge.y, edge.x));

        float minP, maxP;
        ProjectPolygon(poly, axis, minP, maxP);
        float projCenter = glm::dot(center, axis);
        float minC = projCenter - radius;
        float maxC = projCenter + radius;
        float overlap = std::min(maxP, maxC) - std::max(minP, minC);
        if (overlap <= 0.0f) return false;
        if (overlap < minOverlap) { minOverlap = overlap; smallestAxis = axis; }
    }

    // also axis from center to closest point on polygon
    float bestDist2 = FLT_MAX;
    glm::vec2 closestPt(0.0f);
    for (size_t i = 0; i < poly.size(); ++i) {
        glm::vec2 a = poly[i];
        glm::vec2 b = poly[(i+1) % poly.size()];
        glm::vec2 c = ClosestPointOnSegment(a,b,center);
        float d2 = glm::dot(center - c, center - c);
        if (d2 < bestDist2) { bestDist2 = d2; closestPt = c; }
    }
    glm::vec2 axis2 = center - closestPt;
    if (glm::length(axis2) > 1e-6f) {
        axis2 = glm::normalize(axis2);
        float minP, maxP; ProjectPolygon(poly, axis2, minP, maxP);
        float projCenter = glm::dot(center, axis2);
        float overlap = std::min(maxP, projCenter + radius) - std::max(minP, projCenter - radius);
        if (overlap <= 0.0f) return false;
        if (overlap < minOverlap) { minOverlap = overlap; smallestAxis = axis2; }
    }

    // direction should point from other (poly) to circle (center)
    glm::vec2 dir = center - PolygonCentroid(poly);
    if (glm::dot(dir, smallestAxis) < 0.0f) smallestAxis = -smallestAxis;
    outMTV = smallestAxis * minOverlap;
    return true;
}

// Circle vs Circle MTV
static inline bool CircleCircleMTV(const glm::vec2& c1, float r1, const glm::vec2& c2, float r2, glm::vec2& outMTV) {
    glm::vec2 d = c1 - c2;
    float dist2 = glm::dot(d,d);
    float rsum = r1 + r2;
    if (dist2 >= rsum * rsum) return false;
    float dist = std::sqrt(dist2);
    glm::vec2 axis = (dist > 1e-6f) ? (d / dist) : glm::vec2(1.0f,0.0f);
    float overlap = rsum - dist;
    outMTV = axis * overlap;
    return true;
}

} // namespace CollisionUtils
