
#pragma once
#include <string>
#include <memory>

class ModelComponent;

class ModelSerializer {
public:
    // Save model to JSON file. Returns true on success.
    static bool saveModelToFile(const ModelComponent& model, const std::string& path);

    // Load model from JSON file. Returns unique_ptr<ModelComponent> or nullptr on failure.
    static std::unique_ptr<ModelComponent> loadModelFromFile(const std::string& path);
};