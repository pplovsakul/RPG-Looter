# RPG-Looter

RPG-Looter is a 2D game engine/framework built in C++ using OpenGL for rendering. It implements an Entity Component System (ECS) architecture and provides a foundation for creating 2D RPG-style games.

## What is this? (Was ist das?)

This is a game development project that combines modern OpenGL rendering with an ECS architecture pattern. The project includes:

- **Rendering System**: OpenGL-based 2D graphics rendering with shader support
- **Entity Component System**: Modular game object architecture for flexible game development
- **Audio System**: OpenAL-based sound playback
- **Input System**: GLFW-based keyboard and mouse input handling
- **Editor Tools**: ImGui-based in-game editor for asset management and model editing
- **Collision Detection**: Basic collision system for game physics
- **Asset Management**: Centralized texture, shader, and sound asset loading

## Technologies Used

- **C++17**: Core programming language
- **OpenGL 3.3**: Graphics rendering API
- **GLFW 3.4**: Window creation and input handling
- **GLAD**: OpenGL loader
- **GLM**: OpenGL Mathematics library
- **ImGui**: Immediate mode GUI for debug and editor interfaces
- **OpenAL**: Audio playback
- **stb_image**: Image loading
- **CMake**: Build system

## Project Structure

```
RPG-Looter/
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── Game.cpp/h         # Main game class
│   ├── Components.h       # ECS component definitions
│   ├── Entity.cpp/h       # Entity class
│   ├── Entitymanager.cpp/h # Entity management
│   ├── System.h           # Base system interface
│   ├── RenderSystem.cpp/h # Rendering system
│   ├── InputSystem.cpp/h  # Input handling system
│   ├── AudioSystem.cpp/h  # Audio playback system
│   ├── CollisionSystem.cpp/h # Collision detection
│   ├── EditorSystem.cpp/h # In-game editor
│   ├── AssetManager.cpp/h # Asset loading and management
│   └── vendor/            # Third-party libraries
├── res/                   # Resources
│   ├── shaders/          # GLSL shader files
│   ├── textures/         # Image assets
│   └── sounds/           # Audio files
├── include/              # Header files and external libraries
├── extern/               # External dependencies (GLFW)
└── CMakeLists.txt        # CMake build configuration
```

## Architecture

The project uses an **Entity Component System (ECS)** pattern:

- **Entities**: Game objects that are containers for components
- **Components**: Data structures that define entity properties (Transform, Render, etc.)
- **Systems**: Logic processors that operate on entities with specific components

### Key Systems

1. **RenderSystem**: Handles all OpenGL rendering, including sprites, textures, and shaders
2. **InputSystem**: Processes keyboard and mouse input
3. **AudioSystem**: Manages sound playback using OpenAL
4. **CollisionSystem**: Detects and reports collisions between entities
5. **EditorSystem**: Provides in-game editing capabilities
6. **AssetManagerWindow**: UI for managing game assets

## Building the Project

### Prerequisites

- **CMake** 3.13 or higher
- **C++17 compatible compiler** (MSVC, GCC, or Clang)
- **OpenGL 3.3+** compatible graphics driver
- **OpenAL** libraries (included in `extern/openal-soft`)

### Build Instructions (Windows)

1. Clone the repository:
   ```bash
   git clone https://github.com/pplovsakul/RPG-Looter.git
   cd RPG-Looter
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Generate build files with CMake:
   ```bash
   cmake ..
   ```

4. Build the project:
   ```bash
   cmake --build . --config Release
   ```

### Build Instructions (Linux/macOS)

1. Clone the repository:
   ```bash
   git clone https://github.com/pplovsakul/RPG-Looter.git
   cd RPG-Looter
   ```

2. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Generate build files and compile:
   ```bash
   cmake ..
   make
   ```

## Running the Game

After building, the executable will be located in the build directory:

- **Windows**: `build/Release/RPG-Looter.exe` or `build/Debug/RPG-Looter.exe`
- **Linux/macOS**: `build/RPG-Looter`

Run the executable to start the game. A window will open showing the game with a debug overlay displaying FPS and frame time information.

### Controls

- **ESC**: Exit the application
- The game currently includes two test entities (player sprites) that can be controlled through the input system

## Features

### Current Features

- ✅ OpenGL 3.3 Core Profile rendering
- ✅ Entity Component System architecture
- ✅ Texture loading and rendering
- ✅ Shader system with GLSL support
- ✅ Audio playback with OpenAL
- ✅ ImGui-based debug interface
- ✅ Asset management system
- ✅ Collision detection system
- ✅ In-game editor tools
- ✅ Model editor system
- ✅ Entity serialization support

### Planned Features

The project is under active development. Future enhancements may include:
- Advanced gameplay mechanics
- More sophisticated collision responses
- Level/scene management
- Particle systems
- Animation support
- Comprehensive game loop

## Development

The project includes several editor tools accessible through ImGui:

- **Debug Window**: Shows FPS, frame time, and delta time
- **Asset Manager**: Manage textures, sounds, and other assets
- **Model Editor**: Edit and configure game models
- **Entity Editor**: Modify entity components at runtime

## Dependencies

All dependencies are included in the repository:

- **GLFW 3.4**: Included in `extern/glfw-3.4/`
- **GLAD**: Included in `src/vendor/glad/`
- **ImGui**: Included in `src/vendor/imgui/`
- **GLM**: Included in `src/vendor/glm/`
- **stb_image**: Included in `src/vendor/stb_image/`
- **OpenAL**: Libraries included in `extern/openal-soft/` and `libs/`

## License

This project's license is not specified. Please contact the repository owner for licensing information.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Author

This project is maintained by the RPG-Looter development team.
