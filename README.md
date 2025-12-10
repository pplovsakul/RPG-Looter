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
- **🆕 Ray Tracer** - CPU-based ray tracer as alternative rendering mode

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

The application will create a window and display a simple colored cube. 

### Controls

- **WASD** - Move camera
- **Mouse** - Look around
- **Space** - Move up
- **Left Shift** - Move down
- **R** - Toggle between Rasterizer (GPU) and Ray Tracer (CPU) rendering modes
- **ESC** - Exit

## Ray Tracer Integration

This framework now includes a simple CPU-based ray tracer that can be toggled alongside the standard OpenGL rasterizer:

### Features

- **Dual Rendering Modes**: Switch between GPU rasterizer and CPU ray tracer with the R key
- **Camera Synchronization**: Camera controls work identically in both modes
- **Sphere and Box Rendering**: Ray tracer supports both spheres and axis-aligned bounding boxes (AABBs)
- **Lambert Shading**: Simple diffuse lighting model with directional light
- **Background Gradient**: Blue-white sky gradient based on ray direction
- **Optimized Resolution**: Ray tracer runs at 400x300 to minimize CPU load and prevent freezing

### Technical Details

The ray tracer implementation consists of:

1. **Ray.h** - Ray representation with origin and direction
2. **Hit.h** - Hit record structure for ray-object intersections
3. **Sphere.h** - Sphere primitive with ray intersection
4. **Box.h** - AABB (Axis-Aligned Bounding Box) primitive with slab-method ray intersection
5. **Camera.h** - Camera model for generating rays
6. **RayTracer.h** - Main ray tracing logic with Lambert shading
7. **RayTraceRenderer.h** - Uploads CPU-rendered image to OpenGL texture and displays on fullscreen quad

The ray tracer renders the scene on the CPU pixel-by-pixel, then uploads the result as a texture to be displayed via OpenGL. This is much slower than GPU rasterization but demonstrates physically-based rendering concepts.

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
│   ├── Ray.h                 # Ray structure for ray tracing
│   ├── Hit.h                 # Hit record for ray intersections
│   ├── Sphere.h              # Sphere primitive for ray tracing
│   ├── Box.h                 # AABB primitive for ray tracing
│   ├── Camera.h              # Camera for ray generation
│   ├── RayTracer.h           # CPU-based ray tracer
│   ├── RayTraceRenderer.h    # Ray tracer OpenGL integration
│   └── vendor/
│       ├── glad/             # OpenGL loader
│       └── glm/              # Math library
├── res/
│   └── shaders/
│       ├── basic.shader      # Basic vertex/fragment shader (rasterizer)
│       └── neuer_shader.shader  # Texture display shader (ray tracer)
└── CMakeLists.txt            # Build configuration
```

## Dependencies

- **GLFW 3.4** - Window and context creation (included)
- **GLAD** - OpenGL function loader (included)
- **GLM** - Mathematics library (included)

## License

See the original project for license information.
