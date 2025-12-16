# Minimal Graphics Framework

A stripped-down, minimal OpenGL graphics framework with only the essential components needed for rendering.

## Features

This framework includes only the core graphics infrastructure:

- **Window Management** - GLFW-based window creation and management
- **OpenGL Initialization** - Core OpenGL 3.3+ setup via GLAD
- **Vertex Buffer System** - Efficient vertex data management
- **Index Buffer System** - Indexed rendering support
- **Shader System** - Basic vertex and fragment shader support
- **Vertex Array Objects** - VAO management
- **Input System** - Basic keyboard and mouse input handling
- **Rendering Pipeline** - Clean, minimal rendering abstraction
- **Mesh System** - 3D mesh management with material support
- **OBJ/MTL Parser** - Load Wavefront OBJ and MTL files (no external dependencies)

## What Was Removed

This project was stripped down from a full game engine. The following features were removed:

- All ImGui UI code and windows
- Entity Component System (ECS)
- Game logic and game-specific code
- Audio system (OpenAL)
- Asset management
- Scene management
- Collision detection
- Serialization systems
- All editor windows and tools

**Note**: 3D mesh loading and materials have been re-implemented as part of this project. See [Mesh Documentation](docs/MESH_DOCUMENTATION.md) for details.

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

The application will create a window and display a simple colored cube. 

### Running Tests

```bash
./test_mesh
```

This will run unit tests for the Mesh class and OBJ/MTL parser functionality. 

### Controls

- **WASD** - Move camera
- **Mouse** - Look around
- **Space** - Move up
- **Left Shift** - Move down
- **ESC** - Exit

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
│   ├── Mesh.h/cpp            # Mesh class for 3D geometry
│   ├── Material.h            # Material structure
│   ├── ObjParser.h/cpp       # OBJ/MTL file parser
│   └── vendor/
│       ├── glad/             # OpenGL loader
│       └── glm/              # Math library
├── res/
│   ├── shaders/
│   │   └── basic.shader      # Basic vertex/fragment shader
│   └── models/
│       ├── cube.obj          # Example OBJ file
│       └── cube.mtl          # Example MTL file
├── tests/
│   └── test_mesh.cpp         # Unit tests for Mesh and parser
├── docs/
│   └── MESH_DOCUMENTATION.md # Mesh system documentation
└── CMakeLists.txt            # Build configuration
```

## Dependencies

- **GLFW 3.4** - Window and context creation (included)
- **GLAD** - OpenGL function loader (included)
- **GLM** - Mathematics library (included)

## License

See the original project for license information.
