# Minimal Graphics Framework

A stripped-down, minimal OpenGL graphics framework with only the essential components needed for rendering.

## Features

This framework includes only the core graphics infrastructure:

- **Window Management** - GLFW-based window creation and management
- **OpenGL Initialization** - Core OpenGL 3.3 setup via GLAD
- **Vertex Buffer System** - Efficient vertex data management
- **Index Buffer System** - Indexed rendering support
- **Shader System** - Basic vertex and fragment shader support
- **Vertex Array Objects** - VAO management
- **Input System** - Basic keyboard and mouse input handling
- **Rendering Pipeline** - Clean, minimal rendering abstraction

## What Was Removed

This project was stripped down from a full game engine. The following features were removed:

- All ImGui UI code and windows
- Entity Component System (ECS)
- 3D engine components (cameras, mesh loaders, OBJ loading)
- Game logic and game-specific code
- Audio system (OpenAL)
- Texture and material systems
- Asset management
- Scene management
- Collision detection
- Serialization systems
- All editor windows and tools

## Building

### Prerequisites

- CMake 3.13 or higher
- C++17 compatible compiler
- OpenGL 3.3+ support

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

For systems without X11/Wayland (headless environments):
```bash
cmake -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF ..
make
```

### Running

```bash
./RPG-Looter
```

The application will create a window and display a simple colored triangle. Press ESC to exit.

## Project Structure

```
├── src/
│   ├── main.cpp              # Entry point and main loop
│   ├── Renderer.h/cpp        # Rendering abstraction
│   ├── VertexBuffer.h/cpp    # Vertex buffer management
│   ├── IndexBuffer.h/cpp     # Index buffer management
│   ├── VertexArray.h/cpp     # VAO management
│   ├── VertexBufferLayout.h/cpp  # Vertex layout specification
│   ├── Shader.h/cpp          # Shader compilation and management
│   ├── InputSystem.h         # Basic input handling
│   ├── Debug.h               # OpenGL error checking macros
│   └── vendor/
│       ├── glad/             # OpenGL loader
│       └── glm/              # Math library
├── res/
│   └── shaders/
│       └── basic.shader      # Basic vertex/fragment shader
└── CMakeLists.txt            # Build configuration
```

## Dependencies

- **GLFW 3.4** - Window and context creation (included)
- **GLAD** - OpenGL function loader (included)
- **GLM** - Mathematics library (included)

## License

See the original project for license information.
