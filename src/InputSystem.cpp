#include "InputSystem.h"
#include "Components.h"
#include "JsonParser.h"
#include <iostream>
#include <algorithm>

static std::string toUpper(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    return out;
}

static int keyNameToCode(const std::string& keyNameRaw) {
    std::string keyName = toUpper(keyNameRaw);
    if (keyName == "W") return GLFW_KEY_W;
    if (keyName == "A") return GLFW_KEY_A;
    if (keyName == "S") return GLFW_KEY_S;
    if (keyName == "D") return GLFW_KEY_D;
    if (keyName == "UP" || keyName == "UP_ARROW") return GLFW_KEY_UP;
    if (keyName == "DOWN" || keyName == "DOWN_ARROW") return GLFW_KEY_DOWN;
    if (keyName == "LEFT" || keyName == "LEFT_ARROW") return GLFW_KEY_LEFT;
    if (keyName == "RIGHT" || keyName == "RIGHT_ARROW") return GLFW_KEY_RIGHT;
    if (keyName == "SPACE") return GLFW_KEY_SPACE;
    if (keyName == "SHIFT" || keyName == "LEFT_SHIFT" || keyName == "RIGHT_SHIFT") return GLFW_KEY_LEFT_SHIFT;
    if (keyName == "E") return GLFW_KEY_E;
    if (keyName == "ESC" || keyName == "ESCAPE") return GLFW_KEY_ESCAPE;
    if (keyName == "I") return GLFW_KEY_I;
    // Generic single-letter mapping (A-Z)
    if (keyName.size() == 1 && keyName[0] >= 'A' && keyName[0] <= 'Z') {
        return GLFW_KEY_A + (keyName[0] - 'A');
    }
    return GLFW_KEY_UNKNOWN;
}

InputSystem::InputSystem(GLFWwindow* win) : window(win) {
    init(win);
}

void InputSystem::init(GLFWwindow* win) {
    if (win) window = win;

    // Create default context
    InputContext gameContext{"game"};

    // Try to load config into gameContext; on failure use fallback
    try {
        loadConfig("res/config/input_config.json");
        std::cout << "[InputSystem] Config loaded successfully\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[InputSystem] Error loading config: " << e.what() << "\n";
        std::cerr << "[InputSystem] Falling back to default bindings\n";

        gameContext.bindings["move_up"] = { {GLFW_KEY_W}, {GLFW_KEY_UP} };
        gameContext.bindings["move_down"] = { {GLFW_KEY_S}, {GLFW_KEY_DOWN} };
        gameContext.bindings["move_left"] = { {GLFW_KEY_A}, {GLFW_KEY_LEFT} };
        gameContext.bindings["move_right"] = { {GLFW_KEY_D}, {GLFW_KEY_RIGHT} };
        gameContext.bindings["sprint"] = { {GLFW_KEY_LEFT_SHIFT} };

        addContext(gameContext);
    }

    // If config created contexts, ensure game context exists
    if (!isContextActive("game") && contexts.find("game") == contexts.end()) {
        addContext(gameContext);
    }

    activateContext("game");

    // Register actions (movement & others)
    const float baseSpeed = 300.0f;
    bool* isSprinting = new bool(false); // Shared sprint state

    registerAction("sprint", [isSprinting](InputState state, EntityManager& /*em*/, float /*dt*/) {
        if(state == InputState::Pressed) {
            *isSprinting = true;
            std::cout << "Sprint started\n";
        }
        else if(state == InputState::Released) {
            *isSprinting = false;
            std::cout << "Sprint ended\n";
        }
    });

    registerAction("move_up", [baseSpeed, isSprinting](InputState state, EntityManager& em, float dt) {
        if(state == InputState::Pressed || state == InputState::Held) {
            if(auto* camera = em.getEntityByTag("MainCamera")) {
                if(auto* t = camera->getComponent<TransformComponent>()) {
                    if(auto* c = camera->getComponent<CameraComponent>()) {
                        float currentSpeed = *isSprinting ? baseSpeed * 2.0f : baseSpeed;
                        t->position += c->front * currentSpeed * dt;  // Move forward
                    }
                }
            }
        }
    });

    registerAction("move_down", [baseSpeed, isSprinting](InputState state, EntityManager& em, float dt) {
        if(state == InputState::Pressed || state == InputState::Held) {
            if(auto* camera = em.getEntityByTag("MainCamera")) {
                if(auto* t = camera->getComponent<TransformComponent>()) {
                    if(auto* c = camera->getComponent<CameraComponent>()) {
                        float currentSpeed = *isSprinting ? baseSpeed * 2.0f : baseSpeed;
                        t->position -= c->front * currentSpeed * dt;  // Move backward
                    }
                }
            }
        }
    });

    registerAction("move_left", [baseSpeed, isSprinting](InputState state, EntityManager& em, float dt) {
        if(state == InputState::Pressed || state == InputState::Held) {
            if(auto* camera = em.getEntityByTag("MainCamera")) {
                if(auto* t = camera->getComponent<TransformComponent>()) {
                    if(auto* c = camera->getComponent<CameraComponent>()) {
                        float currentSpeed = *isSprinting ? baseSpeed * 2.0f : baseSpeed;
                        t->position -= c->right * currentSpeed * dt;  // Strafe left
                    }
                }
            }
        }
    });

    registerAction("move_right", [baseSpeed, isSprinting](InputState state, EntityManager& em, float dt) {
        if(state == InputState::Pressed || state == InputState::Held) {
            if(auto* camera = em.getEntityByTag("MainCamera")) {
                if(auto* t = camera->getComponent<TransformComponent>()) {
                    if(auto* c = camera->getComponent<CameraComponent>()) {
                        float currentSpeed = *isSprinting ? baseSpeed * 2.0f : baseSpeed;
                        t->position += c->right * currentSpeed * dt;  // Strafe right
                    }
                }
            }
        }
    });

    registerAction("interact", [](InputState state, EntityManager& /*em*/, float /*dt*/) {
        if (state == InputState::Pressed) std::cout << "Interacting...\n";
    });

    registerAction("jump", [](InputState state, EntityManager& /*em*/, float /*dt*/) {
        if (state == InputState::Pressed) std::cout << "Jumping!\n";
    });

    registerAction("pause", [](InputState state, EntityManager& /*em*/, float /*dt*/) {
        if (state == InputState::Pressed) std::cout << "Game paused\n";
    });

    registerAction("inventory", [](InputState state, EntityManager& /*em*/, float /*dt*/) {
        if (state == InputState::Pressed) std::cout << "Opening inventory\n";
    });

    std::cout << "[InputSystem] Initialized with bindings\n";
}

void InputSystem::addContext(const InputContext& ctx) {
    contexts[ctx.name] = ctx;
}

void InputSystem::activateContext(const std::string& name) {
    if (std::find(activeContexts.begin(), activeContexts.end(), name) == activeContexts.end()) {
        activeContexts.push_back(name);
        std::cout << "[InputSystem] Activated context: " << name << "\n";
    }
}

void InputSystem::deactivateContext(const std::string& name) {
    activeContexts.erase(std::remove(activeContexts.begin(), activeContexts.end(), name), activeContexts.end());
    std::cout << "[InputSystem] Deactivated context: " << name << "\n";
}

bool InputSystem::isContextActive(const std::string& name) const {
    return std::find(activeContexts.begin(), activeContexts.end(), name) != activeContexts.end();
}

void InputSystem::registerAction(const std::string& name, std::function<void(InputState, EntityManager&, float)> callback) {
    actions[name] = { name, callback };
}

void InputSystem::loadConfig(const std::string& path) {
    JsonParser parser;
    JsonValue root = parser.parseFile(path);

    if (!root.isObject()) throw std::runtime_error("Invalid JSON root");

    const auto& obj = root.asObject();

    // Debug top-level keys
    std::cout << "[InputSystem] loadConfig top-level keys:";
    for (const auto& kv : obj) std::cout << ' ' << kv.first;
    std::cout << '\n';

    // Case 1: bindings object
    if (obj.count("bindings")) {
        const auto& bindings = obj.at("bindings").asObject();
        InputContext ctx{"game"};
        int count = 0;
        for (const auto& kv : bindings) {
            const std::string action = kv.first;
            const JsonValue& keysVal = kv.second;
            if (!keysVal.isArray()) continue;
            std::vector<InputBinding> actionBindings;
            for (const auto& keyVal : keysVal.asArray()) {
                if (!keyVal.isString()) continue;
                std::string keyName = keyVal.asString();
                int code = keyNameToCode(keyName);
                if (code != GLFW_KEY_UNKNOWN) {
                    actionBindings.push_back({ code });
                    std::cout << "[InputSystem] Binding loaded: action='" << action << "' key='" << keyName << "' code=" << code << "\n";
                } else {
                    std::cout << "[InputSystem] Unknown key in config: '" << keyName << "' for action '" << action << "'\n";
                }
            }
            if (!actionBindings.empty()) { ctx.bindings[action] = actionBindings; count += (int)actionBindings.size(); }
        }
        addContext(ctx);
        std::cout << "[InputSystem] Loaded bindings: " << ctx.bindings.size() << " actions, " << count << " keys\n";
        return;
    }

    // Case 2: contexts array
    if (obj.count("contexts")) {
        const auto& ctxArray = obj.at("contexts").asArray();
        int ctxCount = 0;
        int totalBindings = 0;
        for (const auto& ctxVal : ctxArray) {
            if (!ctxVal.isObject()) continue;
            const auto& ctxObj = ctxVal.asObject();
            if (!ctxObj.count("name") || !ctxObj.count("bindings")) continue;
            InputContext ctx;
            ctx.name = ctxObj.at("name").asString();
            const auto& bindsObj = ctxObj.at("bindings").asObject();
            for (const auto& kv : bindsObj) {
                const std::string actionName = kv.first;
                const JsonValue& keyArrayVal = kv.second;
                if (!keyArrayVal.isArray()) continue;
                std::vector<InputBinding> binds;
                for (const auto& keyVal : keyArrayVal.asArray()) {
                    if (!keyVal.isString()) continue;
                    std::string keyName = keyVal.asString();
                    int code = keyNameToCode(keyName);
                    if (code != GLFW_KEY_UNKNOWN) { binds.push_back({ code }); totalBindings++; 
                        std::cout << "[InputSystem] Binding loaded: context='" << ctx.name << "' action='" << actionName << "' key='" << keyName << "' code=" << code << "\n";
                    } else {
                        std::cout << "[InputSystem] Unknown key in config: '" << keyName << "' for action '" << actionName << "' in context '" << ctx.name << "'\n";
                    }
                }
                if (!binds.empty()) ctx.bindings[actionName] = binds;
            }
            addContext(ctx);
            ctxCount++;
        }
        std::cout << "[InputSystem] Loaded contexts: " << ctxCount << ", total bindings: " << totalBindings << "\n";
        return;
    }

    throw std::runtime_error("No 'bindings' or 'contexts' found in config");
}

bool InputSystem::isKeyDown(int key) const {
    if (!window) return false;
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool InputSystem::isKeyPressedOnce(int key) {
    bool isDown = glfwGetKey(window, key) == GLFW_PRESS;
    bool wasDown = keyState[key];
    keyState[key] = isDown;
    return (isDown && !wasDown);
}

void InputSystem::update(EntityManager& em, float deltaTime) {
    if (!window) return;

    // Update mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));

    for (auto& ctxName : activeContexts) {
        auto it = contexts.find(ctxName);
        if (it == contexts.end()) continue;
        auto& ctx = it->second;

        for (auto it2 = ctx.bindings.begin(); it2 != ctx.bindings.end(); ++it2) {
            const std::string& actionName = it2->first;
            const auto& binds = it2->second;

            if (!actions.count(actionName)) continue;

            bool isAnyKeyDown = false;
            bool wasAnyKeyDown = false;

            for (const auto& bind : binds) {
                bool isDown = glfwGetKey(window, bind.key) == GLFW_PRESS;
                bool wasDown = keyState[bind.key];
                keyState[bind.key] = isDown;

                isAnyKeyDown |= isDown;
                wasAnyKeyDown |= wasDown;
            }

            auto& action = actions[actionName];
            if (isAnyKeyDown && !wasAnyKeyDown) action.callback(InputState::Pressed, em, deltaTime);
            else if (!isAnyKeyDown && wasAnyKeyDown) action.callback(InputState::Released, em, deltaTime);
            else if (isAnyKeyDown) action.callback(InputState::Held, em, deltaTime);
        }
    }
}

glm::vec2 InputSystem::GetMousePosition() const {
    return mousePosition;
}

bool InputSystem::IsMouseButtonPressed(int button) const {
    if (!window) return false;
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void InputSystem::SetMouseOverUI(bool overUI) {
    mouseOverUI = overUI;
}

bool InputSystem::IsMouseOverUI() const {
    return mouseOverUI;
}