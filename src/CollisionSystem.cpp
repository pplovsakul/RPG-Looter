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
        if (!e->hasComponent<ModelComponent>() && !e->hasComponent<RenderComponent>()) continue;
        auto* t = e->getComponent<TransformComponent>();
        if (!t) continue;

        EntData ed;
        ed.e = e;
        ed.t = t;

        // gather shapes once per entity
        if (e->hasComponent<ModelComponent>()) {
            auto* m = e->getComponent<ModelComponent>();
            if (m) {
                for (const auto& s : m->shapes) {
                    ShapeWorld sw;
                    if (s.type == ModelComponent::ShapeType::Circle) {
                        sw.isCircle = true;
                        sw.center = s.position + t->position;
                        sw.radius = (s.size.x * s.scale.x) * 0.5f; // same as ShapeToWorldCircle
                    } else {
                        sw.isCircle = false;
                        sw.poly = CollisionUtils::ShapeToWorldPolygon(s, *t);
                    }
                    ed.shapes.push_back(std::move(sw));
                }
            }
        } else if (e->hasComponent<RenderComponent>()) {
            auto* r = e->getComponent<RenderComponent>();
            ModelComponent::Shape s;
            // ✅ OPTIMIZATION: Use enum comparison instead of string comparison
            if (r->meshType == RenderComponent::MeshType::Circle) {
                s.type = ModelComponent::ShapeType::Circle;
                s.size = t->scale;
            } else {
                s.type = ModelComponent::ShapeType::Rectangle;
                s.size = t->scale;
            }
            s.position = glm::vec2(0.0f);

            ShapeWorld sw;
            if (s.type == ModelComponent::ShapeType::Circle) {
                sw.isCircle = true;
                sw.center = s.position + t->position;
                sw.radius = (s.size.x * s.scale.x) * 0.5f;
            } else {
                sw.isCircle = false;
                sw.poly = CollisionUtils::ShapeToWorldPolygon(s, *t);
            }
            ed.shapes.push_back(std::move(sw));
        }

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
                                // move A out of B
                                A.t->position += mtv;
                            }
                        } else if (sa.isCircle && !sb.isCircle) {
                            hit = CollisionUtils::PolygonCircleMTV(sb.poly, sa.center, sa.radius, mtv);
                            if (hit) A.t->position += mtv;
                        } else if (!sa.isCircle && sb.isCircle) {
                            hit = CollisionUtils::PolygonCircleMTV(sa.poly, sb.center, sb.radius, mtv);
                            if (hit) A.t->position -= mtv;
                        } else {
                            hit = CollisionUtils::PolygonPolygonMTV(sa.poly, sb.poly, mtv);
                            if (hit) A.t->position += mtv;
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

    // copy transform and replace position
    TransformComponent tempT = *t;
    tempT.position = proposedPos;

    // build shapes for the entity like in update
    std::vector<ModelComponent::Shape> shapesE;
    if (e->hasComponent<ModelComponent>()) {
        auto* m = e->getComponent<ModelComponent>();
        if (m) shapesE = m->shapes;
    } else if (e->hasComponent<RenderComponent>()) {
        auto* r = e->getComponent<RenderComponent>();
        ModelComponent::Shape s;
        // ✅ OPTIMIZATION: Use enum comparison instead of string comparison
        if (r->meshType == RenderComponent::MeshType::Circle) { s.type = ModelComponent::ShapeType::Circle; s.size = t->scale; }
        else { s.type = ModelComponent::ShapeType::Rectangle; s.size = t->scale; }
        s.position = glm::vec2(0.0f);
        shapesE.push_back(s);
    }

    if (shapesE.empty()) return false;

    // check against all other entities in em
    auto ents = em.getAllEntities();
    for (Entity* other : ents) {
        if (!other || other == e || !other->active) continue;
        // build shapes for other similar to update
        std::vector<ModelComponent::Shape> shapesO;
        auto* to = other->getComponent<TransformComponent>();
        if (!to) continue;
        if (other->hasComponent<ModelComponent>()) {
            auto* mo = other->getComponent<ModelComponent>();
            if (mo) shapesO = mo->shapes;
        } else if (other->hasComponent<RenderComponent>()) {
            auto* ro = other->getComponent<RenderComponent>();
            ModelComponent::Shape so;
            // ✅ OPTIMIZATION: Use enum comparison instead of string comparison
            if (ro->meshType == RenderComponent::MeshType::Circle) { so.type = ModelComponent::ShapeType::Circle; so.size = to->scale; }
            else { so.type = ModelComponent::ShapeType::Rectangle; so.size = to->scale; }
            so.position = glm::vec2(0.0f);
            shapesO.push_back(so);
        }
        if (shapesO.empty()) continue;

        // now test shapesE (with tempT) vs shapesO (with to)
        for (const auto& se : shapesE) {
            glm::vec2 ce; float re;
            bool ec = CollisionUtils::ShapeToWorldCircle(se, tempT, ce, re);
            std::vector<glm::vec2> polyE;
            if (!ec) polyE = CollisionUtils::ShapeToWorldPolygon(se, tempT);

            for (const auto& so : shapesO) {
                glm::vec2 co; float rof;
                bool oc = CollisionUtils::ShapeToWorldCircle(so, *to, co, rof);
                std::vector<glm::vec2> polyO;
                if (!oc) polyO = CollisionUtils::ShapeToWorldPolygon(so, *to);

                glm::vec2 mtv(0.0f);
                bool hit = false;
                if (ec && oc) hit = CollisionUtils::CircleCircleMTV(ce, re, co, rof, mtv);
                else if (ec && !oc) hit = CollisionUtils::PolygonCircleMTV(polyO, ce, re, mtv);
                else if (!ec && oc) hit = CollisionUtils::PolygonCircleMTV(polyE, co, rof, mtv);
                else hit = CollisionUtils::PolygonPolygonMTV(polyE, polyO, mtv);

                if (hit) return true;
            }
        }
    }

    return false;
}
