#include "PrimitiveGenerator.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::unique_ptr<Mesh3D> PrimitiveGenerator::createCube(float size) {
    float halfSize = size * 0.5f;
    
    std::vector<Vertex3D> vertices = {
        // Front face (z+)
        {{-halfSize, -halfSize,  halfSize}, { 0,  0,  1}, {0, 0}, {1, 1, 1}},
        {{ halfSize, -halfSize,  halfSize}, { 0,  0,  1}, {1, 0}, {1, 1, 1}},
        {{ halfSize,  halfSize,  halfSize}, { 0,  0,  1}, {1, 1}, {1, 1, 1}},
        {{-halfSize,  halfSize,  halfSize}, { 0,  0,  1}, {0, 1}, {1, 1, 1}},
        
        // Back face (z-)
        {{ halfSize, -halfSize, -halfSize}, { 0,  0, -1}, {0, 0}, {1, 1, 1}},
        {{-halfSize, -halfSize, -halfSize}, { 0,  0, -1}, {1, 0}, {1, 1, 1}},
        {{-halfSize,  halfSize, -halfSize}, { 0,  0, -1}, {1, 1}, {1, 1, 1}},
        {{ halfSize,  halfSize, -halfSize}, { 0,  0, -1}, {0, 1}, {1, 1, 1}},
        
        // Right face (x+)
        {{ halfSize, -halfSize,  halfSize}, { 1,  0,  0}, {0, 0}, {1, 1, 1}},
        {{ halfSize, -halfSize, -halfSize}, { 1,  0,  0}, {1, 0}, {1, 1, 1}},
        {{ halfSize,  halfSize, -halfSize}, { 1,  0,  0}, {1, 1}, {1, 1, 1}},
        {{ halfSize,  halfSize,  halfSize}, { 1,  0,  0}, {0, 1}, {1, 1, 1}},
        
        // Left face (x-)
        {{-halfSize, -halfSize, -halfSize}, {-1,  0,  0}, {0, 0}, {1, 1, 1}},
        {{-halfSize, -halfSize,  halfSize}, {-1,  0,  0}, {1, 0}, {1, 1, 1}},
        {{-halfSize,  halfSize,  halfSize}, {-1,  0,  0}, {1, 1}, {1, 1, 1}},
        {{-halfSize,  halfSize, -halfSize}, {-1,  0,  0}, {0, 1}, {1, 1, 1}},
        
        // Top face (y+)
        {{-halfSize,  halfSize,  halfSize}, { 0,  1,  0}, {0, 0}, {1, 1, 1}},
        {{ halfSize,  halfSize,  halfSize}, { 0,  1,  0}, {1, 0}, {1, 1, 1}},
        {{ halfSize,  halfSize, -halfSize}, { 0,  1,  0}, {1, 1}, {1, 1, 1}},
        {{-halfSize,  halfSize, -halfSize}, { 0,  1,  0}, {0, 1}, {1, 1, 1}},
        
        // Bottom face (y-)
        {{-halfSize, -halfSize, -halfSize}, { 0, -1,  0}, {0, 0}, {1, 1, 1}},
        {{ halfSize, -halfSize, -halfSize}, { 0, -1,  0}, {1, 0}, {1, 1, 1}},
        {{ halfSize, -halfSize,  halfSize}, { 0, -1,  0}, {1, 1}, {1, 1, 1}},
        {{-halfSize, -halfSize,  halfSize}, { 0, -1,  0}, {0, 1}, {1, 1, 1}},
    };
    
    std::vector<unsigned int> indices = {
        0,  1,  2,  0,  2,  3,   // front
        4,  5,  6,  4,  6,  7,   // back
        8,  9,  10, 8,  10, 11,  // right
        12, 13, 14, 12, 14, 15,  // left
        16, 17, 18, 16, 18, 19,  // top
        20, 21, 22, 20, 22, 23   // bottom
    };
    
    return std::make_unique<Mesh3D>(vertices, indices);
}

std::unique_ptr<Mesh3D> PrimitiveGenerator::createSphere(float radius, int segments) {
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    int rings = segments / 2;
    
    // Generate vertices
    for (int ring = 0; ring <= rings; ring++) {
        float phi = M_PI * float(ring) / float(rings);
        float y = radius * cos(phi);
        float ringRadius = radius * sin(phi);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * M_PI * float(seg) / float(segments);
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            
            glm::vec3 position(x, y, z);
            glm::vec3 normal = glm::normalize(position);
            glm::vec2 texCoord(float(seg) / float(segments), float(ring) / float(rings));
            
            vertices.push_back(Vertex3D(position, normal, texCoord));
        }
    }
    
    // Generate indices
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    return std::make_unique<Mesh3D>(vertices, indices);
}

std::unique_ptr<Mesh3D> PrimitiveGenerator::createPlane(float width, float height, int subdivisionsX, int subdivisionsY) {
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Generate vertices
    for (int y = 0; y <= subdivisionsY; y++) {
        for (int x = 0; x <= subdivisionsX; x++) {
            float xPos = -halfWidth + (width * x / subdivisionsX);
            float zPos = -halfHeight + (height * y / subdivisionsY);
            
            glm::vec3 position(xPos, 0.0f, zPos);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec2 texCoord(float(x) / float(subdivisionsX), float(y) / float(subdivisionsY));
            
            vertices.push_back(Vertex3D(position, normal, texCoord));
        }
    }
    
    // Generate indices
    for (int y = 0; y < subdivisionsY; y++) {
        for (int x = 0; x < subdivisionsX; x++) {
            int topLeft = y * (subdivisionsX + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * (subdivisionsX + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    return std::make_unique<Mesh3D>(vertices, indices);
}

std::unique_ptr<Mesh3D> PrimitiveGenerator::createCylinder(float radius, float height, int segments) {
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    float halfHeight = height * 0.5f;
    
    // Generate side vertices
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0f, z));
        glm::vec2 texCoord(float(i) / float(segments), 0.0f);
        
        // Bottom vertex
        vertices.push_back(Vertex3D(glm::vec3(x, -halfHeight, z), normal, texCoord));
        // Top vertex
        vertices.push_back(Vertex3D(glm::vec3(x, halfHeight, z), normal, glm::vec2(texCoord.x, 1.0f)));
    }
    
    // Generate side indices
    for (int i = 0; i < segments; i++) {
        int current = i * 2;
        int next = (i + 1) * 2;
        
        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(current + 1);
        
        indices.push_back(current + 1);
        indices.push_back(next);
        indices.push_back(next + 1);
    }
    
    // Add caps
    int baseIndex = vertices.size();
    
    // Bottom cap center
    vertices.push_back(Vertex3D(glm::vec3(0, -halfHeight, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f)));
    // Top cap center
    vertices.push_back(Vertex3D(glm::vec3(0, halfHeight, 0), glm::vec3(0, 1, 0), glm::vec2(0.5f, 0.5f)));
    
    int bottomCenterIndex = baseIndex;
    int topCenterIndex = baseIndex + 1;
    
    // Bottom cap vertices and indices
    for (int i = 0; i < segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        vertices.push_back(Vertex3D(glm::vec3(x, -halfHeight, z), glm::vec3(0, -1, 0), glm::vec2(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta))));
        vertices.push_back(Vertex3D(glm::vec3(x, halfHeight, z), glm::vec3(0, 1, 0), glm::vec2(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta))));
        
        int bottomIdx = baseIndex + 2 + i * 2;
        int topIdx = baseIndex + 3 + i * 2;
        int nextBottomIdx = baseIndex + 2 + ((i + 1) % segments) * 2;
        int nextTopIdx = baseIndex + 3 + ((i + 1) % segments) * 2;
        
        // Bottom cap
        indices.push_back(bottomCenterIndex);
        indices.push_back(nextBottomIdx);
        indices.push_back(bottomIdx);
        
        // Top cap
        indices.push_back(topCenterIndex);
        indices.push_back(topIdx);
        indices.push_back(nextTopIdx);
    }
    
    return std::make_unique<Mesh3D>(vertices, indices);
}

std::unique_ptr<Mesh3D> PrimitiveGenerator::createCone(float radius, float height, int segments) {
    std::vector<Vertex3D> vertices;
    std::vector<unsigned int> indices;
    
    float halfHeight = height * 0.5f;
    
    // Apex vertex
    vertices.push_back(Vertex3D(glm::vec3(0, halfHeight, 0), glm::vec3(0, 1, 0), glm::vec2(0.5f, 1.0f)));
    
    // Base vertices
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        glm::vec3 position(x, -halfHeight, z);
        glm::vec3 toApex = glm::normalize(glm::vec3(0, halfHeight, 0) - position);
        glm::vec3 tangent = glm::normalize(glm::vec3(-z, 0, x));
        glm::vec3 normal = glm::normalize(glm::cross(tangent, toApex));
        
        vertices.push_back(Vertex3D(position, normal, glm::vec2(float(i) / float(segments), 0.0f)));
    }
    
    // Side indices
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }
    
    // Base cap center
    int baseCenterIndex = vertices.size();
    vertices.push_back(Vertex3D(glm::vec3(0, -halfHeight, 0), glm::vec3(0, -1, 0), glm::vec2(0.5f, 0.5f)));
    
    // Base cap indices
    for (int i = 0; i < segments; i++) {
        indices.push_back(baseCenterIndex);
        indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    
    return std::make_unique<Mesh3D>(vertices, indices);
}
