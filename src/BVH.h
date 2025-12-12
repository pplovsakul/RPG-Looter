#pragma once
#include "vendor/glm/glm.hpp"
#include "Triangle.h"
#include <vector>
#include <algorithm>

// BVH Node für GPU (kompakte Struktur)
struct BVHNode {
    glm::vec3 aabbMin;
    int leftChild;      // Index zum linken Kind (oder erster Triangle bei Leaf)
    glm::vec3 aabbMax;
    int triangleCount;  // 0 = internal node, > 0 = leaf mit dieser Anzahl Triangles
    
    BVHNode() : aabbMin(0.0f), aabbMax(0.0f), leftChild(0), triangleCount(0) {}
    
    bool isLeaf() const { return triangleCount > 0; }
};

// BVH Builder
class BVHBuilder {
private:
    std::vector<Triangle> triangles;
    std::vector<BVHNode> nodes;
    std::vector<Triangle> orderedTriangles;
    
    struct BuildNode {
        glm::vec3 aabbMin, aabbMax;
        int firstTriIndex;
        int triangleCount;
        int leftChild, rightChild;
        
        BuildNode() : aabbMin(FLT_MAX), aabbMax(-FLT_MAX), 
                     firstTriIndex(0), triangleCount(0), 
                     leftChild(-1), rightChild(-1) {}
    };
    
    // Berechne AABB für Triangle-Range
    void computeAABB(const std::vector<Triangle>& tris, int start, int count, 
                     glm::vec3& minBounds, glm::vec3& maxBounds) {
        minBounds = glm::vec3(FLT_MAX);
        maxBounds = glm::vec3(-FLT_MAX);
        
        for (int i = start; i < start + count; ++i) {
            glm::vec3 triMin, triMax;
            tris[i].getBounds(triMin, triMax);
            minBounds = glm::min(minBounds, triMin);
            maxBounds = glm::max(maxBounds, triMax);
        }
    }
    
    // Rekursiver BVH Build (Mid-Point Split)
    int buildRecursive(std::vector<Triangle>& tris, int start, int count, int depth = 0) {
        BuildNode node;
        node.firstTriIndex = start;
        node.triangleCount = count;
        
        // Berechne AABB für diesen Node
        computeAABB(tris, start, count, node.aabbMin, node.aabbMax);
        
        // Leaf-Kriterium: wenige Triangles oder max Tiefe
        if (count <= 4 || depth > 20) {
            // Leaf Node
            int nodeIdx = nodes.size();
            BVHNode gpuNode;
            gpuNode.aabbMin = node.aabbMin;
            gpuNode.aabbMax = node.aabbMax;
            gpuNode.leftChild = orderedTriangles.size(); // Index zum ersten Triangle
            gpuNode.triangleCount = count;
            nodes.push_back(gpuNode);
            
            // Kopiere Triangles in geordnetes Array
            for (int i = start; i < start + count; ++i) {
                orderedTriangles.push_back(tris[i]);
            }
            
            return nodeIdx;
        }
        
        // Finde längste Achse
        glm::vec3 extent = node.aabbMax - node.aabbMin;
        int axis = 0;
        if (extent.y > extent.x) axis = 1;
        if (extent.z > extent[axis]) axis = 2;
        
        // Sortiere nach Centroid entlang der Achse
        std::sort(tris.begin() + start, tris.begin() + start + count,
            [axis](const Triangle& a, const Triangle& b) {
                return a.centroid()[axis] < b.centroid()[axis];
            });
        
        // Split in der Mitte
        int mid = count / 2;
        
        // Internal Node
        int nodeIdx = nodes.size();
        BVHNode gpuNode;
        gpuNode.aabbMin = node.aabbMin;
        gpuNode.aabbMax = node.aabbMax;
        gpuNode.triangleCount = 0; // Internal node
        nodes.push_back(gpuNode); // Reserve Platz
        
        // Baue Kinder rekursiv
        int leftIdx = buildRecursive(tris, start, mid, depth + 1);
        int rightIdx = buildRecursive(tris, start + mid, count - mid, depth + 1);
        
        // Update Parent mit Kind-Indizes
        nodes[nodeIdx].leftChild = leftIdx;
        // rightChild ist implizit leftChild + 1 (Layout-Optimierung)
        
        return nodeIdx;
    }
    
public:
    BVHBuilder() {}
    
    void build(const std::vector<Triangle>& inputTriangles) {
        if (inputTriangles.empty()) return;
        
        triangles = inputTriangles;
        nodes.clear();
        orderedTriangles.clear();
        
        // Baue BVH
        buildRecursive(triangles, 0, triangles.size());
        
        std::cout << "BVH gebaut: " << nodes.size() << " Nodes, " 
                  << orderedTriangles.size() << " Triangles" << std::endl;
    }
    
    const std::vector<BVHNode>& getNodes() const { return nodes; }
    const std::vector<Triangle>& getOrderedTriangles() const { return orderedTriangles; }
};
