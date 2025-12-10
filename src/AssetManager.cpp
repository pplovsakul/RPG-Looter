#include "AssetManager.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "ECSSound.h"
#include "Font.h"
#include "Components.h"
#include "OBJLoader.h"
#include <algorithm>

AssetManager* AssetManager::instance = nullptr;

AssetManager* AssetManager::getInstance() {
    if (!instance)
        instance = new AssetManager();
    return instance;
}

AssetManager::~AssetManager() {
    clear();
}

void AssetManager::clear() {
    shaders.clear();
    textures.clear();
    meshes.clear();
    sounds.clear();
    fonts.clear();
    models.clear();
    std::cout << "[AssetManager] Cleared all assets.\n";
}

// === SHADERS =================================================================

Shader* AssetManager::loadShader(const std::string& name, const std::string& filepath) {
    if (shaders.find(name) != shaders.end()) {
        std::cout << "[AssetManager] Shader already loaded: " << name << "\n";
        return shaders[name].get();
    }

    try {
        auto shader = std::make_unique<Shader>(filepath);
        Shader* ptr = shader.get();
        shaders[name] = std::move(shader);
        std::cout << "[AssetManager] Loaded shader: " << name << " from " << filepath << "\n";
        return ptr;
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetManager][Error] Failed to load shader " << name << ": " << e.what() << "\n";
        return nullptr;
    }
}

Shader* AssetManager::getShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it == shaders.end()) {
        std::cerr << "[AssetManager][Warning] Shader not found: " << name << "\n";
        return nullptr;
    }
    return it->second.get();
}

// === TEXTURES ================================================================

Texture* AssetManager::loadTexture(const std::string& name, const std::string& filepath) {
    if (textures.find(name) != textures.end()) {
        std::cout << "[AssetManager] Texture already loaded: " << name << "\n";
        return textures[name].get();
    }

    try {
        auto texture = std::make_unique<Texture>(filepath);
        Texture* ptr = texture.get();
        textures[name] = std::move(texture);
        std::cout << "[AssetManager] Loaded texture: " << name << " from " << filepath << "\n";
        return ptr;
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetManager][Error] Failed to load texture " << name << ": " << e.what() << "\n";
        return nullptr;
    }
}

Texture* AssetManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end()) {
        std::cerr << "[AssetManager][Warning] Texture not found: " << name << "\n";
        return nullptr;
    }
    return it->second.get();
}

// NEW: return all loaded texture names
std::vector<std::string> AssetManager::getTextureNames() const {
    std::vector<std::string> names;
    names.reserve(textures.size());
    for (const auto& kv : textures) names.push_back(kv.first);
    return names;
}

// === SOUNDS ===================================================================

ECSSound* AssetManager::loadSound(const std::string& name, const std::string& filepath) {
    if (sounds.find(name) != sounds.end()) {
        std::cout << "[AudioSystem] Sound already loaded: " << name << "\n";
        return sounds[name].get();
    }

    try{
        // ECSSound anlegen (ohne Datei laden im Konstruktor)
        auto sound = std::make_unique<ECSSound>();

        // Datei laden über Methode
        if (!sound->LoadFromFile(filepath)) {
            std::cerr << "[AssetManager][Error] Failed to load sound file: " << filepath << "\n";
            return nullptr;
        }

		ECSSound* ptr = sound.get();
		sounds[name] = std::move(sound);
        std::cout << "[AssetManager] Loaded sound: " << name << " from " << filepath << "\n";
        return ptr;
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetManager][Error] Failed to load sound " << name << ": " << e.what() << "\n";
        return nullptr;
	}
}

ECSSound* AssetManager::getSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        std::cerr << "[AssetManager][Warning] Sound not found: " << name << "\n";
        return nullptr;
    }
    return it->second.get();
}

// NEW: return all loaded sound names
std::vector<std::string> AssetManager::getSoundNames() const {
    std::vector<std::string> names;
    names.reserve(sounds.size());
    for (const auto& kv : sounds) names.push_back(kv.first);
    return names;
}

// === FONTS ===================================================================

Font* AssetManager::loadFont(const std::string& name, const std::string& jsonPath, const std::string& atlasPath) {
    if (fonts.find(name) != fonts.end()) {
        std::cout << "[AssetManager] Font already loaded: " << name << "\n";
        return fonts[name].get();
    }

    try {
        auto font = std::make_unique<Font>(name, jsonPath, atlasPath);
        Font* ptr = font.get();
        fonts[name] = std::move(font);
        std::cout << "[AssetManager] Loaded font: " << name << "\n";
        return ptr;
    }
    catch (const std::exception& e) {
        std::cerr << "[AssetManager][Error] Failed to load font " << name << ": " << e.what() << "\n";
        return nullptr;
    }
}

Font* AssetManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end()) {
        std::cerr << "[AssetManager][Warning] Font not found: " << name << "\n";
        return nullptr;
    }
    return it->second.get();
}

std::vector<std::string> AssetManager::getFontNames() const {
    std::vector<std::string> names;
    names.reserve(fonts.size());
    for (const auto& kv : fonts) names.push_back(kv.first);
    return names;
}


// === MESHES ==================================================================

VertexArray* AssetManager::createMesh(const std::string& name,
    float* vertices, unsigned int vertexSize,
    unsigned int* indices, unsigned int indexCount)
{
    if (meshes.find(name) != meshes.end()) {
        std::cout << "[AssetManager] Mesh already exists: " << name << "\n";
        return meshes[name].vao.get();
    }

    MeshData meshData;

    meshData.vao = std::make_unique<VertexArray>();
    meshData.vbo = std::make_unique<VertexBuffer>(vertices, vertexSize);
    meshData.ibo = std::make_unique<IndexBuffer>(indices, indexCount);

    VertexBufferLayout layout;
    layout.AddFloat(3); // Position (x, y, z)
    layout.AddFloat(3); // Normal (nx, ny, nz)
    layout.AddFloat(2); // TexCoords (u, v)
    meshData.vao->AddBuffer(*meshData.vbo, layout);
    meshData.vao->SetIndexBuffer(std::move(meshData.ibo));

    VertexArray* ptr = meshData.vao.get();
    meshes[name] = std::move(meshData);

    std::cout << "[AssetManager] Created mesh: " << name << " ("
        << indexCount << " indices)\n";
    return ptr;
}

VertexArray* AssetManager::getMesh(const std::string& name) {
    auto it = meshes.find(name);
    if (it == meshes.end()) {
        std::cerr << "[AssetManager][Warning] Mesh not found: " << name << "\n";
        return nullptr;
    }
    return it->second.vao.get();
}

// === MODELS ==================================================================
bool AssetManager::addModel(const std::string& name, const ModelComponent& model) {
    if (models.find(name) != models.end()) {
        std::cout << "[AssetManager] Model already exists: " << name << "\n";
        return false;
    }
    auto m = std::make_unique<ModelComponent>();
    m->meshes = model.meshes;  // Copy mesh data
    models[name] = std::move(m);
    std::cout << "[AssetManager] Registered model: " << name << "\n";
    return true;
}

bool AssetManager::loadModelFromFile(const std::string& name, const std::string& filepath) {
    try {
        // Check file extension to determine loader
        std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        std::unique_ptr<ModelComponent> model;
        
        if (ext == "obj") {
            // Use OBJ loader for .obj files
            model = OBJLoader::loadOBJ(filepath);
        } else {
            std::cerr << "[AssetManager] Unsupported model format: " << ext << "\n";
            return false;
        }
        
        if (!model) {
            std::cerr << "[AssetManager] Failed to parse model file: " << filepath << "\n";
            return false;
        }
        return addModel(name, *model);
    } catch (const std::exception& e) {
        std::cerr << "[AssetManager] Exception loading model: " << e.what() << "\n";
        return false;
    }
}

ModelComponent* AssetManager::getModel(const std::string& name) {
    auto it = models.find(name);
    if (it == models.end()) {
        //std::cerr << "[AssetManager][Warning] Model not found: " << name << "\n";
        return nullptr;
    }
    return it->second.get();
}

std::vector<std::string> AssetManager::getModelNames() const {
    std::vector<std::string> names;
    names.reserve(models.size());
    for (const auto& kv : models) names.push_back(kv.first);
    return names;
}