# GPU Compute Shader Ray Tracer - Implementation Documentation

## Überblick

Diese Implementierung erweitert den bestehenden CPU Ray Tracer um eine moderne **GPU Compute Shader Implementation** mit erweiterten Features wie PBR-Materialien, Multi-Sample Anti-Aliasing (MSAA) und rekursiven Reflexionen.

## Hauptkomponenten

### 1. Material-System (`Material.h`)

Das Material-System basiert auf **Physically Based Rendering (PBR)** Prinzipien:

```cpp
struct Material {
    glm::vec3 albedo;      // Grundfarbe (RGB)
    float roughness;       // 0.0 = Spiegel, 1.0 = diffus
    float metallic;        // 0.0 = Dielektrikum, 1.0 = Metall
    glm::vec3 emission;    // Selbstleuchtend
};
```

**Vordefinierte Materialien:**
- `Chrome()`: Hochreflektierend (metallic=1.0, roughness=0.1)
- `Gold()`: Goldfarben metallisch (metallic=1.0, roughness=0.2)
- `Glass()`: Glatt für spätere Refraktion (metallic=0.0, roughness=0.0)
- `Rubber()`: Sehr diffus (metallic=0.0, roughness=0.9)
- `Emissive()`: Selbstleuchtend mit einstellbarer Intensität

### 2. CPU Ray Tracer (`RayTracer.h`)

**Erweiterte Features:**
- **MSAA**: 1, 4, 9, oder 16 Samples pro Pixel
- **Rekursive Reflexionen**: Bounce-Tiefe 1-10
- **PBR Shading**: Materialbasierte Beleuchtung
- **Random Sampling**: Für Anti-Aliasing und diffuse Reflexionen

**Shading-Algorithmus:**
1. Ray-Scene Intersection Test
2. Material-Properties auslesen
3. Direkte Beleuchtung (Lambert + Ambient)
4. Reflexionsrichtung berechnen (Mix aus perfekter Reflexion und diffuser basierend auf Roughness)
5. Rekursiver Ray Cast für Reflexionen
6. Energie-Verlust basierend auf Material (Metallic, Roughness)

**Performance:**
- Resolution: 400x300 (CPU-optimiert)
- Samples: 1 = ~30fps, 16 = ~2fps
- Bounces: Linear impact pro Sample

### 3. GPU Compute Shader Ray Tracer

#### 3.1 ComputeShader Wrapper (`ComputeShader.h`)

Verwaltet OpenGL Compute Shader mit **Runtime Function Loading**:

```cpp
class ComputeShader {
    // Lädt OpenGL 4.3+ Funktionen zur Laufzeit
    PFNGLDISPATCHCOMPUTEPROC glDispatchCompute_ptr;
    PFNGLBINDIMAGETEXTUREPROC glBindImageTexture_ptr;
    PFNGLMEMORYBARRIERPROC glMemoryBarrier_ptr;
    
    // Graceful Fallback wenn Compute Shaders nicht verfügbar
    bool isAvailable() const;
};
```

**Wichtig**: Verwendet `glfwGetProcAddress()` um OpenGL 4.3 Funktionen zu laden, da GLAD nur für OpenGL 3.3 kompiliert wurde.

#### 3.2 GPURayTracer Class (`GPURayTracer.h`)

**Architektur:**
```
Scene Data (CPU) → SSBO Upload → Compute Shader → Image Write → Display Texture
```

**Shader Storage Buffer Objects (SSBOs):**
1. **Sphere Buffer**: Array von Spheres mit Material-Index
2. **Box Buffer**: Array von AABBs mit Material-Index
3. **Material Buffer**: Array von PBR Materials

**GPU-Strukturen** (std430 Layout):
```glsl
struct Material {
    vec3 albedo;
    float roughness;
    vec3 emission;
    float metallic;
};

struct Sphere {
    vec3 center;
    float radius;
    int materialIndex;
    float _pad0, _pad1, _pad2; // 16-byte alignment
};
```

#### 3.3 Compute Shader (`raytracer.comp`)

**Work Group Size**: 8x8 threads (64 threads pro Work Group)

**Hauptalgorithmus:**
```glsl
layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba8, binding = 0) uniform writeonly image2D outputImage;

void main() {
    // 1. Pixel-Koordinaten berechnen
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    
    // 2. Random Seed initialisieren (PCG Hash)
    uint seed = hash(coords + frameCount);
    
    // 3. Multi-Sample Loop
    for (int s = 0; s < samplesPerPixel; ++s) {
        // 3.1 Zufälliges Sub-Pixel Offset
        float offsetX = random01(seed);
        float offsetY = random01(seed);
        
        // 3.2 Ray Generation
        Ray ray = cameraGetRay(u, v);
        
        // 3.3 Ray Tracing mit Bounces
        color += shade(ray, seed);
    }
    
    // 4. Average und Write
    color /= samplesPerPixel;
    imageStore(outputImage, coords, vec4(color, 1.0));
}
```

**Ray Tracing Loop:**
```glsl
vec3 shade(vec3 rayOrigin, vec3 rayDir, inout uint seed) {
    vec3 color = vec3(0.0);
    vec3 throughput = vec3(1.0);
    
    for (int bounce = 0; bounce < maxBounces; ++bounce) {
        // Hit Test
        if (!hitWorld(rayOrigin, rayDir, rec)) {
            color += throughput * background();
            break;
        }
        
        // Material Properties
        Material mat = materials[rec.materialIndex];
        
        // Emission
        color += throughput * mat.emission;
        
        // Direct Lighting
        color += throughput * directLight(mat, rec);
        
        // Reflection Direction (mix based on roughness)
        vec3 reflDir = mix(perfectReflection, diffuseReflection, mat.roughness);
        
        // Update for next bounce
        throughput *= reflectionTint * reflectivity;
        rayOrigin = rec.point;
        rayDir = reflDir;
        
        // Russian Roulette termination
        if (bounce > 3 && random01(seed) > maxThroughput) break;
    }
    
    return color;
}
```

## Integration in main.cpp

### Rendering Modi

```cpp
int renderMode = 0; // 0=Rasterizer, 1=CPU RT, 2=GPU RT

if (renderMode == 1) {
    cpuRT.draw(rtshader.GetRendererID());
}
else if (renderMode == 2 && gpuRT) {
    gpuRT->draw(rtshader.GetRendererID());
}
else {
    // Standard Rasterizer
    renderer.Draw(va, ib, shader);
}
```

### Hotkey System

**Debouncing** für saubere Tastenerkennung:

```cpp
bool rKeyWasPressed = false;

// In Main Loop:
bool rKeyIsPressed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
if (rKeyIsPressed && !rKeyWasPressed) {
    renderMode = (renderMode + 1) % 3;
}
rKeyWasPressed = rKeyIsPressed;
```

**Vollständige Hotkeys:**
- `R`: Cycle Rendering Mode
- `1-4`: Set Samples (1, 4, 9, 16)
- `B`: Increase Bounces (1-10, cycles)
- `M`: Cycle Material Sets (GPU only)

### Kamera-Synchronisation

```cpp
// Update für beide Ray Tracer
cpuRT.tracer.camera.position = cameraPos;
cpuRT.tracer.camera.target = cameraPos + cameraFront;
cpuRT.tracer.camera.up = cameraUp;
cpuRT.tracer.camera.vfov = 45.0f;

if (gpuRT) {
    gpuRT->camera = cpuRT.tracer.camera; // Sync
}
```

## Performance-Optimierungen

### CPU Ray Tracer
1. **Early Termination**: Bei schwarzen Pixeln abbrechen
2. **Single Sample**: Default = 1 für interaktive Performance
3. **Reduzierte Resolution**: 400x300 statt 1280x720

### GPU Ray Tracer
1. **Work Group Size**: 8x8 optimal für meiste GPUs
2. **Russian Roulette**: Frühe Terminierung nach Bounce 3
3. **PCG Hash**: Schneller Pseudo-Zufallsgenerator
4. **Shared Memory**: Potential für zukünftige Optimierung
5. **Adaptive Sampling**: Könnte basierend auf Varianz implementiert werden

## Kompatibilität

### OpenGL Versionen
- **3.3**: Rasterizer + CPU Ray Tracer
- **4.3+**: Alle Modi inkl. GPU Ray Tracer

### Runtime Detection
```cpp
GLint major, minor;
glGetIntegerv(GL_MAJOR_VERSION, &major);
glGetIntegerv(GL_MINOR_VERSION, &minor);
bool computeShadersSupported = (major > 4) || (major == 4 && minor >= 3);

if (computeShadersSupported) {
    gpuRT = new GPURayTracer(...);
    if (!gpuRT->isAvailable()) {
        // Fallback to CPU
        delete gpuRT;
        gpuRT = nullptr;
    }
}
```

### Function Loading
Da GLAD nur für OpenGL 3.3 kompiliert wurde, werden OpenGL 4.3 Funktionen zur Laufzeit geladen:

```cpp
glDispatchCompute_ptr = (PFNGLDISPATCHCOMPUTEPROC)
    glfwGetProcAddress("glDispatchCompute");
```

## Zukünftige Erweiterungen

### Geplante Features
1. **Refraktion**: Für Glass-Material (Snell's Law)
2. **Importance Sampling**: Bessere Noise-Reduction
3. **BVH**: Beschleunigungsstruktur für viele Objekte
4. **Texturen**: UV-Mapping für Materialien
5. **HDR**: High Dynamic Range mit Tone Mapping
6. **Denoising**: KI-basiertes oder temporal denoising

### Debug-Modi
1. **Normal Visualization**: Zeige Normalen als Farben
2. **Material Properties**: Visualisiere Roughness/Metallic
3. **Bounce Depth**: Farbcodierung nach Bounce-Nummer
4. **Performance Metrics**: Rays/second, Frame time

## Verwendung

### Erste Schritte
1. Starte mit Rasterizer-Modus (Standard)
2. Drücke `R` um CPU Ray Tracer zu aktivieren
3. Drücke `R` erneut für GPU Ray Tracer (falls verfügbar)
4. Experimentiere mit `1-4` für Samples
5. Drücke `B` mehrmals um Reflexionen zu sehen
6. Drücke `M` um verschiedene Materialien zu sehen (GPU only)

### Empfohlene Einstellungen

**Interaktiv (GPU):**
- Samples: 1-4
- Bounces: 1-3
- ~30-60 FPS möglich

**Quality (GPU):**
- Samples: 9-16
- Bounces: 4-6
- ~5-15 FPS

**CPU Mode:**
- Samples: 1
- Bounces: 1-2
- ~10-20 FPS bei 400x300

## Troubleshooting

### GPU Ray Tracer nicht verfügbar
**Problem**: "GPU Ray Tracer nicht verfügbar (OpenGL X.X < 4.3)"
**Lösung**: 
- Update GPU-Treiber
- Oder verwende CPU Ray Tracer als Fallback

### Schwarzer Bildschirm im GPU-Modus
**Problem**: Compute Shader kompiliert nicht
**Lösung**: 
- Prüfe shader log in Console
- Fallback zu CPU Modus mit `R`

### Performance-Probleme
**Problem**: Niedrige FPS
**Lösung**:
- Reduziere Samples mit `1`
- Reduziere Bounces
- Wechsel zu Rasterizer mit `R`

## Technische Details

### Speicher-Layout
- **CPU**: Standard C++ Strukturen
- **GPU SSBO**: std430 Layout mit Padding
- **Alignment**: 16-byte für Vec3 + Float

### Precision
- **Float**: 32-bit IEEE 754
- **Random**: PCG Hash (32-bit state)

### Limits
- Max Samples: Unbegrenzt (praktisch 16)
- Max Bounces: Unbegrenzt (praktisch 10)
- Max Objects: SSBO Größe (praktisch tausende)

## Literatur & Referenzen

1. **Ray Tracing in One Weekend** - Peter Shirley
2. **Physically Based Rendering** - Matt Pharr et al.
3. **Real-Time Rendering** - Tomas Akenine-Möller et al.
4. **GPU Gems** - NVIDIA
5. **OpenGL 4.3 Specification** - Khronos Group
