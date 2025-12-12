# Room and Ceiling Light Implementation

## Overview
This document describes the implementation of a large room around the existing cube with a physical ceiling light source that is only visible through ray bouncing.

## Implementation Details

### Scene Configuration
The scene now consists of:

1. **Original Cube**
   - Position: (0, 0, 0.5)
   - Size: 1x1x1 units
   - Material: Red diffuse (0.8, 0.3, 0.3)

2. **Room Dimensions**
   - Size: 10x10x10 units
   - Wall thickness: 0.1 units
   - Material: Gray diffuse (0.7, 0.7, 0.7)

3. **Room Components**
   - **Floor**: Y = -5.0, horizontal plane at the bottom
   - **Ceiling**: Y = +5.0, horizontal plane at the top
   - **Back Wall**: Z = 0.0, vertical plane
   - **Front Wall**: Z = 10.0, vertical plane
   - **Left Wall**: X = -5.0, vertical plane
   - **Right Wall**: X = +5.0, vertical plane

4. **Ceiling Light Source**
   - Position: (0, 4.7, 5.0) - center of ceiling, slightly below
   - Size: 1x0.1x1 units (flat panel)
   - Material: Emissive warm white (1.0, 1.0, 0.9) with intensity 5.0
   - **Physical Light**: Only visible when rays bounce off it (no direct visibility)

### Camera Configuration
- Initial Position: (0, 0, 6.0)
- Looking at: (0, 0, 0.5) - toward the cube
- FOV: 45 degrees
- This position allows viewing the cube and parts of the room

### Ray Tracer Integration
The room and light are added to both rendering modes:

1. **CPU Ray Tracer** (`cpuRT.tracer.boxes`)
   - All 6 walls (floor, ceiling, 4 sides)
   - Ceiling light panel

2. **GPU Ray Tracer** (`gpuRT->boxes`)
   - Same configuration as CPU version
   - Benefits from GPU parallel processing

### How the Physical Light Works

The ceiling light is an **emissive material** with the following properties:
- Albedo: (0, 0, 0) - no reflected color
- Roughness: 1.0 - fully diffuse
- Metallic: 0.0 - non-metallic
- Emission: (5.0, 5.0, 4.5) - bright warm white light

**Key Characteristic**: The light is only visible through ray bouncing:
1. Rays from the camera don't directly "see" the emission
2. When rays bounce off surfaces (cube, walls), they can hit the light
3. The emission from the light contributes to the indirect illumination
4. This creates realistic soft lighting as in a real room

### Testing the Implementation

To see the effect:
1. Build and run the application
2. Press **R** to switch to CPU or GPU Ray Tracer mode
3. Press **B** to increase bounce depth (2+ bounces recommended)
4. Press **2**, **3**, or **4** to increase samples per pixel for better quality
5. Use **WASD** and mouse to move around and view the room from different angles

Expected visual results:
- The cube should be illuminated by the ceiling light
- Shadows should appear on surfaces away from the light
- The light panel itself should appear bright when visible
- Walls should show subtle lighting from indirect bounces

## Code Changes

### Modified Files
- `src/main.cpp`: Added room walls and ceiling light to scene configuration

### Lines of Code Changed
- Added ~100 lines for room and light setup
- Modified camera initial position for better viewing

## Technical Notes

### Performance Considerations
- **CPU Ray Tracer**: 400x300 resolution
  - Adding 7 boxes (6 walls + 1 light) has minimal impact
  - Ray-box intersection is efficient with AABB slab method
  
- **GPU Ray Tracer**: 160x90 resolution (1280x720 / 8)
  - Parallel processing handles additional geometry well
  - Compute shader processes all boxes in parallel

### Material Properties
The emissive material uses PBR (Physically Based Rendering):
- No albedo (pure emission)
- Maximum roughness (diffuse emission)
- Zero metallic (non-metal light source)
- Emission intensity of 5.0 for bright illumination

### Coordinate System
- **X-axis**: Left (-) to Right (+)
- **Y-axis**: Down (-) to Up (+)
- **Z-axis**: Back (0) to Front (+)
- Origin: Center of the room at (0, 0, 5)
- Cube slightly offset in Z for better visibility

## Future Enhancements

Possible improvements:
1. Add multiple light sources at different positions
2. Implement colored lights (RGB emissive materials)
3. Add light intensity controls via keyboard
4. Support for spot/directional lights
5. Add more geometry (furniture, decorations)

## Conclusion

The implementation successfully adds:
✅ A large room (10x10x10 units) around the existing cube
✅ A physical ceiling light source that only illuminates through ray bouncing
✅ Support for both CPU and GPU ray tracing modes
✅ Proper camera positioning for optimal viewing

The ray tracer now demonstrates realistic indirect lighting in a closed room environment.
