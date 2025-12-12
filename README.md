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
- **🆕 Ray Tracer** - CPU-based ray tracer with PBR materials, MSAA, and reflections
- **🆕 GPU Compute Shader Ray Tracer** - High-performance GPU ray tracing with OpenGL 4.3+

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
- OpenGL 3.3+ support (OpenGL 4.3+ recommended for GPU ray tracer)

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
- **R** - Toggle between rendering modes (Rasterizer / CPU Ray Tracer / GPU Ray Tracer)
- **1-4** - Set samples per pixel (1, 4, 9, 16) for anti-aliasing
- **B** - Increase bounce depth (1-10) for reflections
- **M** - Switch material sets (GPU Ray Tracer only)
- **ESC** - Exit

## Ray Tracer Integration

This framework now includes both CPU and GPU ray tracers with advanced rendering features:

### Features

#### CPU Ray Tracer
- **PBR Material System**: Physically-based rendering with albedo, roughness, metallic, and emission
- **Multi-Sample Anti-Aliasing**: 1, 4, 9, or 16 samples per pixel
- **Recursive Reflections**: Configurable bounce depth (1-10)
- **Material Presets**: Chrome, Gold, Glass, Rubber, Emissive materials
- **Resolution**: 400x300 (optimized for CPU performance)

#### GPU Compute Shader Ray Tracer (OpenGL 4.3+)
- **High Performance**: Dramatically faster than CPU implementation
- **Full Resolution**: 1280x720 rendering
- **Same Features**: PBR materials, MSAA, reflections
- **Shader Storage Buffer Objects**: Efficient scene data management
- **Runtime Detection**: Automatically falls back to CPU if GPU compute shaders unavailable

### Technical Details

The ray tracer implementation consists of:

1. **Material.h** - PBR material system with physically-based properties
2. **Ray.h** - Ray representation with origin and direction
3. **Hit.h** - Hit record structure for ray-object intersections with material information
4. **Sphere.h** - Sphere primitive with ray intersection and materials
5. **Box.h** - AABB (Axis-Aligned Bounding Box) primitive with slab-method ray intersection and materials
6. **Camera.h** - Camera model for generating rays
7. **RayTracer.h** - CPU-based ray tracing with PBR, MSAA, and reflections
8. **RayTraceRenderer.h** - Uploads CPU-rendered image to OpenGL texture and displays on fullscreen quad
9. **ComputeShader.h** - OpenGL Compute Shader wrapper with runtime function loading
10. **GPURayTracer.h** - GPU-based ray tracing using compute shaders
11. **res/shaders/raytracer.comp** - Compute shader implementing GPU ray tracing

### Material System

Materials are defined with the following PBR properties:

- **Albedo**: Base color of the surface (RGB)
- **Roughness**: Surface roughness from 0.0 (mirror) to 1.0 (completely diffuse)
- **Metallic**: Metallic factor from 0.0 (dielectric) to 1.0 (metal)
- **Emission**: Self-illuminating color for light sources (RGB)

Predefined material presets:
- **Chrome**: High-reflective metallic (metallic=1.0, roughness=0.1)
- **Gold**: Metallic with golden albedo (metallic=1.0, roughness=0.2)
- **Glass**: Smooth non-metallic for refraction (metallic=0.0, roughness=0.0)
- **Rubber**: Very diffuse non-metallic (metallic=0.0, roughness=0.9)
- **Emissive**: Self-illuminating materials

### Rendering Modes

1. **Rasterizer (Default)**: Standard OpenGL GPU rendering - fastest, but less physically accurate
2. **CPU Ray Tracer**: Pixel-by-pixel ray tracing on CPU - slower but more accurate, 400x300 resolution
3. **GPU Ray Tracer (OpenGL 4.3+)**: Parallel ray tracing on GPU via compute shaders - fast and accurate, 1280x720 resolution

### Quality Settings

- **Samples Per Pixel**: Higher samples reduce aliasing but increase render time
  - 1 sample: Fastest, visible aliasing
  - 4 samples: Good balance
  - 9 samples: Better quality
  - 16 samples: Best quality, slowest

- **Bounce Depth**: Number of light bounces for reflections
  - 1 bounce: Direct lighting only
  - 2-5 bounces: Realistic reflections
  - 6-10 bounces: More accurate but slower

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
│   ├── Material.h            # PBR material system
│   ├── Ray.h                 # Ray structure for ray tracing
│   ├── Hit.h                 # Hit record for ray intersections
│   ├── Sphere.h              # Sphere primitive for ray tracing
│   ├── Box.h                 # AABB primitive for ray tracing
│   ├── Camera.h              # Camera for ray generation
│   ├── RayTracer.h           # CPU-based ray tracer with PBR
│   ├── RayTraceRenderer.h    # Ray tracer OpenGL integration
│   ├── ComputeShader.h       # Compute shader wrapper
│   ├── GPURayTracer.h        # GPU-based ray tracer
│   └── vendor/
│       ├── glad/             # OpenGL loader
│       └── glm/              # Math library
├── res/
│   └── shaders/
│       ├── basic.shader      # Basic vertex/fragment shader (rasterizer)
│       ├── neuer_shader.shader  # Texture display shader (ray tracer)
│       └── raytracer.comp    # Compute shader for GPU ray tracing
└── CMakeLists.txt            # Build configuration
```

## Dependencies

- **GLFW 3.4** - Window and context creation (included)
- **GLAD** - OpenGL function loader (included)
- **GLM** - Mathematics library (included)

## Performance Notes

### CPU Ray Tracer
- Resolution: 400x300 to prevent UI freezing
- Single-threaded implementation
- Performance depends on:
  - Samples per pixel (linear impact)
  - Bounce depth (exponential impact)
  - Scene complexity

### GPU Ray Tracer
- Resolution: 1280x720 full HD
- Massively parallel execution
- Requires OpenGL 4.3+ for compute shader support
- Performance depends on:
  - GPU capabilities
  - Samples per pixel
  - Bounce depth
  - Scene complexity

## Compatibility

- **Minimum**: OpenGL 3.3 (CPU ray tracer only)
- **Recommended**: OpenGL 4.3+ (enables GPU ray tracer)
- The application automatically detects GPU capabilities and falls back to CPU ray tracer if compute shaders are unavailable

## License

See the original project for license information.
