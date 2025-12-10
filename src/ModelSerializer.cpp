
#include "ModelSerializer.h"
#include "Components.h"
#include <iostream>

// NOTE: ModelSerializer is deprecated. ModelComponent now stores 3D mesh data from OBJ files.
// Use OBJLoader for loading 3D models instead.

bool ModelSerializer::saveModelToFile(const ModelComponent& /*model*/, const std::string& /*path*/) {
    std::cerr << "[ModelSerializer] Deprecated: Use OBJLoader for 3D models.\n";
    return false;
}

std::unique_ptr<ModelComponent> ModelSerializer::loadModelFromFile(const std::string& /*path*/) {
    std::cerr << "[ModelSerializer] Deprecated: Use OBJLoader for 3D models.\n";
    return nullptr;
}
