#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <vector>

class Shader;
class Texture;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class VertexBufferLayout;
class ECSSound;
class ModelComponent;

class AssetManager {
public:
    static AssetManager* getInstance();
    ~AssetManager();

    // 🔹 Shader
    Shader* loadShader(const std::string& name, const std::string& filepath);
    Shader* getShader(const std::string& name);

    // 🔹 Texture
    Texture* loadTexture(const std::string& name, const std::string& filepath);
    Texture* getTexture(const std::string& name);
    std::vector<std::string> getTextureNames() const;

	// 🔹 Sound
    ECSSound* loadSound(const std::string& name, const std::string& filepath);
	ECSSound* getSound(const std::string& name);
    std::vector<std::string> getSoundNames() const;

    // 🔹 Model (Model assets)
    bool addModel(const std::string& name, const ModelComponent& model);                 // register model in memory
    bool loadModelFromFile(const std::string& name, const std::string& filepath);       // parse file and register
    ModelComponent* getModel(const std::string& name);
    std::vector<std::string> getModelNames() const;

    // 🔹 Mesh
    VertexArray* createMesh(const std::string& name,
        float* vertices, unsigned int vertexSize,
        unsigned int* indices, unsigned int indexCount);
    VertexArray* getMesh(const std::string& name);

    void clear();

private:
    AssetManager() = default;

    static AssetManager* instance;

    struct MeshData {
        std::unique_ptr<VertexArray> vao;
        std::unique_ptr<VertexBuffer> vbo;
        std::unique_ptr<IndexBuffer> ibo;
    };

    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string, std::unique_ptr<ECSSound>> sounds;
    std::unordered_map<std::string, MeshData> meshes;

    // models stored as ModelComponent instances
    std::unordered_map<std::string, std::unique_ptr<ModelComponent>> models;
};