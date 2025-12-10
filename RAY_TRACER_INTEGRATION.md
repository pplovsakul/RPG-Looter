# Ray Tracer Integration - Implementation Summary

## Übersicht

Ein einfacher CPU-basierter Ray Tracer wurde erfolgreich in das RPG-Looter Framework integriert. Der Ray Tracer kann über die **R-Taste** als alternative Render-Option zum Standard-OpenGL-Rasterizer umgeschaltet werden.

## Implementierte Komponenten

### 1. Kern-Klassen für Ray Tracing

#### **Ray.h**
- Repräsentiert einen Strahl mit Ursprung und Richtung
- Methode `at(t)` zur Berechnung eines Punktes entlang des Strahls

#### **Hit.h**
- Struktur für Schnittpunkt-Informationen
- Speichert Parameter t, Schnittpunkt, Normale und Vorder-/Rückseite
- Hilfsfunktion `setFaceNormal()` zur korrekten Normalen-Orientierung

#### **Sphere.h**
- Kugel-Primitive für Ray Tracing
- Ray-Sphere Intersection mit quadratischer Gleichung
- Funktioniert mit beliebigem Radius und Zentrum

#### **Box.h** (NEU)
- Axis-Aligned Bounding Box (AABB) Primitive
- Slab-Method für effiziente Ray-Box Intersection
- Statische Hilfsfunktion `fromCenterSize()` für einfache Konstruktion
- Automatische Normalen-Berechnung basierend auf getroffener Fläche

#### **Camera.h**
- Kamera-Modell zur Generierung von Strahlen
- Unterstützt Field-of-View (FOV), Aspect Ratio, Position und Look-At
- Methode `getRay(u, v)` für Strahlgenerierung pro Pixel

### 2. Ray Tracer Engine

#### **RayTracer.h**
- Haupt-Ray-Tracing-Logik
- Verwaltet Szenen-Objekte (Spheres und Boxes getrennt)
- Implementiert:
  - **Lambert-Beleuchtung**: Diffuse Reflexion mit Richtungslicht
  - **Hintergrund-Gradient**: Blau-Weiß Himmel basierend auf Strahl-Richtung
  - **Ray-Object Intersection**: Testet alle Objekte und findet nächsten Treffer
  - **Pixel-Rendering**: Berechnet RGBA8-Buffer für gesamtes Bild

#### **RayTraceRenderer.h**
- Integration des Ray Tracers mit OpenGL
- Workflow:
  1. CPU berechnet Bild via `RayTracer::render()`
  2. Bild wird als GL_TEXTURE_2D hochgeladen
  3. Fullscreen-Quad zeigt die Textur an
- Verwendet Nearest-Neighbor Filtering für scharfe Pixel

### 3. Shader

#### **neuer_shader.shader** (NEU)
- Einfacher Vertex/Fragment Shader
- Zeigt Textur auf Fullscreen-Quad an
- Vertex Shader: Position und UV-Koordinaten
- Fragment Shader: Texture Sampling

### 4. Haupt-Integration (main.cpp)

#### Konfiguration
- **RT_WIDTH = 400, RT_HEIGHT = 300**: Reduzierte Auflösung um CPU-Last zu minimieren
- `useRayTracer`: Boolean Flag zum Umschalten zwischen Modi

#### Hotkey-System mit Debouncing
```cpp
bool rKeyIsPressed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
if (rKeyIsPressed && !rKeyWasPressed) {
    useRayTracer = !useRayTracer;
    std::cout << "Switched to " << (useRayTracer ? "Ray Tracer" : "Rasterizer") << " mode" << std::endl;
}
rKeyWasPressed = rKeyIsPressed;
```
- Nur bei Tastendruck-Flanke (nicht gedrückt → gedrückt) wird umgeschaltet
- Verhindert mehrfaches Toggle pro Frame

#### Kamera-Synchronisation
```cpp
rt.tracer.camera.position = cameraPos;
rt.tracer.camera.target = cameraPos + cameraFront;
rt.tracer.camera.up = cameraUp;
rt.tracer.camera.vfov = 45.0f; // Match Rasterizer FOV
```
- Jedes Frame werden Kamera-Parameter vom Rasterizer zum Ray Tracer synchronisiert
- Beide Modi zeigen identische Ansicht

#### Szenen-Setup
```cpp
rt.tracer.boxes.emplace_back(Box::fromCenterSize(
    glm::vec3(0.0f, 0.0f, 0.0f), 
    glm::vec3(1.0f, 1.0f, 1.0f)
));
```
- Würfel aus der Rasterizer-Szene wird als AABB zum Ray Tracer hinzugefügt
- Ground-Sphere (großer Radius) simuliert eine Ebene

## Technische Details

### Performance-Optimierungen

1. **Niedrige Auflösung (400x300)**
   - Verhindert Einfrieren während des CPU-Renderings
   - Bei 1280x720 würde jeder Frame mehrere Sekunden dauern
   
2. **Einfache Beleuchtung**
   - Lambert-Modell ist sehr effizient
   - Nur ein Richtungslicht, keine Schatten

3. **Direkte Rendering-Pipeline**
   - Keine Rekursion oder Reflexionen
   - Ein Strahl pro Pixel

### Limitierungen und Workarounds

1. **CPU-basiert**: Ray Tracing ist naturgemäß langsam auf der CPU
   - **Workaround**: Niedrige Auflösung (400x300)
   
2. **Keine Rekursion**: Keine Reflexionen oder Refraktionen
   - **Kommentar**: Für einfaches Demo ausreichend
   
3. **Hotkey-Debouncing erforderlich**
   - **Workaround**: State-Tracking mit `rKeyWasPressed`

## Build-System

### CMakeLists.txt Anpassungen

```cmake
# Box.h zu Source-Dateien hinzugefügt
"src/Box.h"

# Shader-Verzeichnis wird korrekt erstellt
file(MAKE_DIRECTORY ${SHADER_BUILD_DIR})

# Shader-Dateien werden einzeln kopiert
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${SHADER_BUILD_DIR}/${SHADER_NAME} COPYONLY)
endforeach()
```

## Steuerung

- **WASD**: Kamera bewegen
- **Maus**: Umsehen
- **Space**: Hoch
- **Left Shift**: Runter
- **R**: Toggle zwischen Rasterizer und Ray Tracer
- **ESC**: Beenden

## Dateistruktur

```
src/
├── Ray.h                 # Strahl-Struktur
├── Hit.h                 # Schnittpunkt-Informationen
├── Sphere.h              # Kugel-Primitive
├── Box.h                 # AABB-Primitive (NEU)
├── Camera.h              # Kamera für Ray Tracing
├── RayTracer.h           # Ray Tracing Engine
├── RayTraceRenderer.h    # OpenGL Integration
└── main.cpp              # Hauptprogramm mit Integration

res/shaders/
├── basic.shader          # Standard Rasterizer Shader
└── neuer_shader.shader   # Ray Tracer Textur-Shader (NEU)
```

## Erfolgreiche Integration

✅ Alle Anforderungen erfüllt:
- CPU-Renderer mit RGBA-Buffer
- Hotkey-Umschaltung (R-Taste mit Debouncing)
- Kamera-Synchronisation
- Sphere und Box/AABB Rendering
- OpenGL-Textur Upload und Fullscreen-Quad Display
- 400x300 Auflösung gegen Einfrieren
- Würfel aus Rasterizer-Szene im Ray Tracer
- Alle relevanten Klassen implementiert
- Lambert-Beleuchtung und Gradient-Hintergrund
- Umfassende deutsche Kommentierung
- README-Dokumentation aktualisiert

## Nutzung

```bash
# Build
mkdir build && cd build
cmake -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF ..
make

# Run
./RPG-Looter
```

Im laufenden Programm:
1. Mit WASD und Maus die Szene erkunden
2. **R drücken** um zum Ray Tracer zu wechseln
3. Ray Tracer rendert die gleiche Ansicht (kann 1-2 Sekunden dauern pro Frame)
4. **R erneut drücken** um zurück zum schnellen Rasterizer zu wechseln

## Fazit

Der Ray Tracer wurde erfolgreich integriert und bietet eine funktionierende Alternative zum Standard-Rasterizer. Durch die niedrige Auflösung und optimierte Implementation bleibt das Framework trotz CPU-Rendering verwendbar. Die Kamera-Synchronisation funktioniert einwandfrei, und beide Render-Modi zeigen identische Ansichten der Szene.
