#pragma once

#include "Mesh3D.h"
#include <memory>

class PrimitiveGenerator {
public:
    static std::unique_ptr<Mesh3D> createCube(float size = 1.0f);
    static std::unique_ptr<Mesh3D> createSphere(float radius = 1.0f, int segments = 32);
    static std::unique_ptr<Mesh3D> createCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32);
    static std::unique_ptr<Mesh3D> createPlane(float width = 1.0f, float height = 1.0f, int subdivisionsX = 1, int subdivisionsY = 1);
    static std::unique_ptr<Mesh3D> createCone(float radius = 1.0f, float height = 2.0f, int segments = 32);
};
