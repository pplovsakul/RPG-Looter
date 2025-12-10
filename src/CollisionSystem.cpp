#include "CollisionSystem.h"
#include "Components.h"
#include "Entity.h"
#include "AssetManager.h"
#include <unordered_map>
#include <unordered_set>
#include <cmath>

struct ShapeWorld {
    bool isCircle;
    glm::vec2 center; // for circle
    float radius;
    std::vector<glm::vec2> poly; // for polygon
};

void CollisionSystem::update(EntityManager& em, float /*deltaTime*/) {
    collisions.clear();
    auto entities = em.getAllEntities();

    // Build lists of candidate entities: those with ModelComponent OR RenderComponent
    struct EntData {
        Entity* e = nullptr;
        TransformComponent* t = nullptr;
        std::vector<ShapeWorld> shapes;
        glm::vec2 bboxCenter;
        float bboxRadius = 0.0f; // bounding circle radius for broad-phase
    };

    std::vector<EntData> candidates;
    candidates.reserve(entities.size());

    for (Entity* e : entities) {
        if (!e || !e->active) continue;
        // Only process entities with RenderComponent (ModelComponent is now for 3D meshes)
        if (!e->hasComponent<RenderComponent>()) continue;
        auto* t = e->getComponent<TransformComponent>();
        if (!t) continue;

        EntData ed;
        ed.e = e;
        ed.t = t;

        // gather shapes once per entity - only from RenderComponent
        auto* r = e->getComponent<RenderComponent>();
        ShapeWorld sw;
        // Use 2D projection for collision (ignore Z)
        glm::vec2 pos2D = glm::vec2(t->position.x, t->position.y);
        glm::vec2 scale2D = glm::vec2(t->scale.x, t->scale.y);
        
        // ✅ OPTIMIZATION: Use enum comparison instead of string comparison
        if (r->meshType == RenderComponent::MeshType::Circle) {
            sw.isCircle = true;
            sw.center = pos2D;
            sw.radius = (scale2D.x) * 0.5f;
        } else {
            sw.isCircle = false;
            // Build a simple quad polygon for collision
            float halfW = scale2D.x * 0.5f;
            float halfH = scale2D.y * 0.5f;
            float rot = t->rotation.y; // Use Y rotation for 2D
            float cs = std::cos(rot);
            float sn = std::sin(rot);
            
            glm::vec2 corners[4] = {
                {-halfW, -halfH},
                { halfW, -halfH},
                { halfW,  halfH},
                {-halfW,  halfH}
            };
            
            sw.poly.reserve(4);
            for (int i = 0; i < 4; i++) {
                glm::vec2 rotated;
                rotated.x = corners[i].x * cs - corners[i].y * sn;
                rotated.y = corners[i].x * sn + corners[i].y * cs;
                sw.poly.push_back(pos2D + rotated);
            }
        }
        ed.shapes.push_back(std::move(sw));

        if (ed.shapes.empty()) continue;

        // compute simple bounding circle (center average of shape centers, radius = max distance+shapeRadius)
        glm::vec2 avg(0.0f);
        size_t count = 0;
        float maxr = 0.0f;
        for (const auto& sw : ed.shapes) {
            if (sw.isCircle) {
                avg += sw.center;
                ++count;
                if (sw.radius > maxr) maxr = sw.radius;
            } else if (!sw.poly.empty()) {
                // approximate center as centroid
                glm::vec2 c(0.0f);
                for (const auto& v : sw.poly) c += v;
                c /= (float)sw.poly.size();
                avg += c;
                ++count;
                // compute max distance from centroid to vertices
                float mr = 0.0f;
                for (const auto& v : sw.poly) {
                    float d2 = glm::dot(v - c, v - c);
                    if (d2 > mr) mr = d2;
                }
                float mradius = std::sqrt(mr);
                if (mradius > maxr) maxr = mradius;
            }
        }
        if (count == 0) continue;
        ed.bboxCenter = avg / (float)count;
        ed.bboxRadius = maxr;

        candidates.push_back(std::move(ed));
    }

    // If few candidates fallback to brute-force (but use cached shapes)
    const size_t N = candidates.size();
    if (N == 0) return;

    // Spatial hash / uniform grid
    const float cellSize = 200.0f; // tweak based on average object size
    auto cellKey = [](int x, int y) -> long long { return ( (long long)x << 32 ) | (unsigned int)y; };
    std::unordered_map<long long, std::vector<int>> grid;
    grid.reserve(N * 2);

    for (int i = 0; i < (int)N; ++i) {
        const auto& ed = candidates[i];
        int minX = (int)std::floor((ed.bboxCenter.x - ed.bboxRadius) / cellSize);
        int maxX = (int)std::floor((ed.bboxCenter.x + ed.bboxRadius) / cellSize);
        int minY = (int)std::floor((ed.bboxCenter.y - ed.bboxRadius) / cellSize);
        int maxY = (int)std::floor((ed.bboxCenter.y + ed.bboxRadius) / cellSize);
        for (int cx = minX; cx <= maxX; ++cx) {
            for (int cy = minY; cy <= maxY; ++cy) {
                grid[cellKey(cx,cy)].push_back(i);
            }
        }
    }

    std::unordered_set<unsigned long long> testedPairs;
    testedPairs.reserve(N * 4);

    // For each cell, test pairs inside cell
    for (auto& kv : grid) {
        auto& vec = kv.second;
        for (size_t ai = 0; ai < vec.size(); ++ai) {
            int i = vec[ai];
            for (size_t bi = ai + 1; bi < vec.size(); ++bi) {
                int j = vec[bi];
                int aidx = std::min(i,j);
                int bidx = std::max(i,j);
                unsigned long long key = ((unsigned long long)aidx << 32) | (unsigned int)bidx;
                if (testedPairs.find(key) != testedPairs.end()) continue;
                testedPairs.insert(key);

                EntData& A = candidates[aidx];
                EntData& B = candidates[bidx];

                // cheap bounding circle test
                glm::vec2 ca = A.bboxCenter;
                glm::vec2 cb = B.bboxCenter;
                float ra = A.bboxRadius;
                float rb = B.bboxRadius;
                if (!CollisionUtils::CircleIntersectCircle(ca, ra, cb, rb)) continue;

                // narrow-phase: test each shape pair using precomputed world shapes
                bool anyIntersection = false;
                for (const auto& sa : A.shapes) {
                    for (const auto& sb : B.shapes) {
                        glm::vec2 mtv(0.0f);
                        bool hit = false;
                        if (sa.isCircle && sb.isCircle) {
                            hit = CollisionUtils::CircleCircleMTV(sa.center, sa.radius, sb.center, sb.radius, mtv);
                            if (hit) {
                                // move A out of B (apply to X and Y only, leave Z unchanged)
                                A.t->position.x += mtv.x;
                                A.t->position.y += mtv.y;
                            }
                        } else if (sa.isCircle && !sb.isCircle) {
                            hit = CollisionUtils::PolygonCircleMTV(sb.poly, sa.center, sa.radius, mtv);
                            if (hit) {
                                A.t->position.x += mtv.x;
                                A.t->position.y += mtv.y;
                            }
                        } else if (!sa.isCircle && sb.isCircle) {
                            hit = CollisionUtils::PolygonCircleMTV(sa.poly, sb.center, sb.radius, mtv);
                            if (hit) {
                                A.t->position.x -= mtv.x;
                                A.t->position.y -= mtv.y;
                            }
                        } else {
                            hit = CollisionUtils::PolygonPolygonMTV(sa.poly, sb.poly, mtv);
                            if (hit) {
                                A.t->position.x += mtv.x;
                                A.t->position.y += mtv.y;
                            }
                        }
                        if (hit) { anyIntersection = true; break; }
                    }
                    if (anyIntersection) break;
                }

                if (anyIntersection) collisions.emplace_back(A.e, B.e);
            }
        }
    }
}

bool CollisionSystem::wouldCollide(Entity* e, const glm::vec2& proposedPos, EntityManager& em) const {
    if (!e || !e->active) return false;
    auto* t = e->getComponent<TransformComponent>();
    if (!t) return false;
    
    // Only check collision for entities with RenderComponent
    if (!e->hasComponent<RenderComponent>()) return false;
    auto* r = e->getComponent<RenderComponent>();

    // Build simple shape for entity at proposed position
    glm::vec2 scale2D = glm::vec2(t->scale.x, t->scale.y);
    bool isCircle = (r->meshType == RenderComponent::MeshType::Circle);
    
    // check against all other entities in em
    auto ents = em.getAllEntities();
    for (Entity* other : ents) {
        if (!other || other == e || !other->active) continue;
        if (!other->hasComponent<RenderComponent>()) continue;
        
        auto* to = other->getComponent<TransformComponent>();
        if (!to) continue;
        auto* ro = other->getComponent<RenderComponent>();
        
        glm::vec2 otherPos2D = glm::vec2(to->position.x, to->position.y);
        glm::vec2 otherScale2D = glm::vec2(to->scale.x, to->scale.y);
        bool otherIsCircle = (ro->meshType == RenderComponent::MeshType::Circle);
        
        // Simple circle-circle or AABB collision check
        if (isCircle && otherIsCircle) {
            float r1 = scale2D.x * 0.5f;
            float r2 = otherScale2D.x * 0.5f;
            glm::vec2 diff = proposedPos - otherPos2D;
            float dist2 = glm::dot(diff, diff);
            float rsum = r1 + r2;
            if (dist2 <= rsum * rsum) return true;
        } else {
            // AABB check (simplified)
            float hw1 = scale2D.x * 0.5f;
            float hh1 = scale2D.y * 0.5f;
            float hw2 = otherScale2D.x * 0.5f;
            float hh2 = otherScale2D.y * 0.5f;
            
            if (std::abs(proposedPos.x - otherPos2D.x) < (hw1 + hw2) &&
                std::abs(proposedPos.y - otherPos2D.y) < (hh1 + hh2)) {
                return true;
            }
        }
    }

    return false;
}
