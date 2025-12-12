#pragma once
#include <vector>
#include <random>
#include "vendor/glm/glm.hpp"
#include "Ray.h"
#include "Hit.h"
#include "Sphere.h"
#include "Box.h"
#include "Camera.h"
#include "Material.h"

// Einfache RGB-Farbe für Ray Tracing Ergebnisse
struct RTColor { float r, g, b; };

// RayTracer: CPU-basierter Ray Tracer mit PBR-Materialien, MSAA und Reflexionen
// Rendert Spheres und Box-Objekte (AABBs) mit einem Hintergrund-Gradient
class RayTracer {
public:
    int width = 4;
    int height = 3;
    Camera camera;
    // Kein Richtungslicht mehr - nur Deckenlampe als Lichtquelle

    // Rendering-Einstellungen
    int samplesPerPixel = 1;  // Anti-Aliasing: 1, 4, 9, 16 samples
    int maxBounces = 1;        // Maximale Reflexionstiefe: 0-10

    // Szenen-Objekte: Spheres und AABBs können separat gerendert werden
    std::vector<Sphere> spheres;
    std::vector<Box> boxes;

    RayTracer(int w, int h) : width(w), height(h) {
        camera.aspect = float(w) / float(h);
        camera.update();
        // Keine Standard-Szene: Objekte werden vom Hauptprogramm hinzugefügt
    }

    // Hintergrund: Gleiche Farbe wie der Rasterizer (dunkles Grau)
    glm::vec3 background(const Ray& r) const {
        // Verwende die gleiche Hintergrundfarbe wie glClearColor(0.1f, 0.1f, 0.1f, 1.0f)
        return glm::vec3(0.1f, 0.1f, 0.1f);
    }

    // Random number generator für Anti-Aliasing
    float random01() const {
        static thread_local std::mt19937 generator(std::random_device{}());
        static thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        return distribution(generator);
    }

    // Berechne zufällige Richtung in Hemisphäre um Normal
    glm::vec3 randomInHemisphere(const glm::vec3& normal) const {
        // Einfache diffuse Reflexion: zufälliger Punkt auf Einheitskugel
        float z = random01() * 2.0f - 1.0f;
        float a = random01() * 2.0f * 3.14159265f;
        float r = sqrtf(1.0f - z * z);
        float x = r * cosf(a);
        float y = r * sinf(a);
        glm::vec3 dir(x, y, z);
        
        // Stelle sicher, dass Richtung in Hemisphäre zeigt
        if (glm::dot(dir, normal) < 0.0f) {
            dir = -dir;
        }
        return glm::normalize(dir);
    }

    // Berechne Reflexionsrichtung für glatte Oberflächen
    glm::vec3 reflect(const glm::vec3& incident, const glm::vec3& normal) const {
        return incident - 2.0f * glm::dot(incident, normal) * normal;
    }

    // Shading: Berechnet Farbe für einen Strahl mit PBR-basierter Beleuchtung und Reflexionen
    glm::vec3 shade(const Ray& r, int depth) const {
        // Maximale Bounce-Tiefe erreicht
        if (depth >= maxBounces) {
            return glm::vec3(0.0f);
        }

        HitRecord closestRec;
        bool hitAnything = false;
        float closestSoFar = 1e30f;

        // Teste alle Spheres auf Kollision
        for (const auto& s : spheres) {
            HitRecord rec;
            if (s.hit(r, 0.001f, closestSoFar, rec)) {
                hitAnything = true;
                closestSoFar = rec.t;
                closestRec = rec;
            }
        }

        // Teste alle Boxes (AABBs) auf Kollision
        for (const auto& b : boxes) {
            HitRecord rec;
            if (b.hit(r, 0.001f, closestSoFar, rec)) {
                hitAnything = true;
                closestSoFar = rec.t;
                closestRec = rec;
            }
        }

        // Kein Treffer: Zeige Hintergrund
        if (!hitAnything) return background(r);

        const Material& mat = closestRec.material;

        // Emission: Füge selbstleuchtende Farbe hinzu
        glm::vec3 emitted = mat.emission;

        // Kein direktionales Licht mehr - nur Emission und Ambient
        // Die Deckenlampe liefert Licht durch ihre Emission
        
        // Ambient Term
        glm::vec3 ambient = mat.albedo * 0.05f; // Reduziertes Ambient für realistischere Beleuchtung

        // Berechne Reflexion basierend auf Material
        glm::vec3 reflectedColor(0.0f);
        
        if (depth + 1 < maxBounces) {
            // Mische zwischen perfekter Reflexion und diffuser basierend auf Roughness
            glm::vec3 perfectReflection = reflect(r.direction, closestRec.normal);
            glm::vec3 diffuseReflection = randomInHemisphere(closestRec.normal);
            
            // Interpoliere basierend auf Roughness
            glm::vec3 reflectionDir = glm::normalize(
                glm::mix(perfectReflection, diffuseReflection, mat.roughness)
            );
            
            Ray reflectedRay(closestRec.point, reflectionDir);
            reflectedColor = shade(reflectedRay, depth + 1);
            
            // Metallic bestimmt, wie viel von Reflexion vs. Albedo verwendet wird
            // Metalle reflektieren ihre Farbe, Dielektrika reflektieren weiß
            glm::vec3 reflectionTint = glm::mix(glm::vec3(1.0f), mat.albedo, mat.metallic);
            reflectedColor *= reflectionTint;
        }

        // Kombiniere alle Komponenten
        // Mehr reflektierend = weniger direkte Beleuchtung
        float reflectivity = (1.0f - mat.roughness) * 0.8f;
        glm::vec3 finalColor = emitted + 
                               ambient + 
                               reflectedColor * reflectivity;

        return finalColor;
    }

    // Render: Hauptfunktion zum CPU-basierten Ray Tracing mit MSAA
    // Gibt RGBA8-Buffer zurück (32-bit pro Pixel: R, G, B, A)
    std::vector<uint32_t> render() {
        camera.update(); // Aktualisiere Kamera-Basis-Vektoren
        std::vector<uint32_t> pixels(width * height);
        
        // Für jedes Pixel im Bild...
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                glm::vec3 pixelColor(0.0f);
                
                // Multi-Sample Anti-Aliasing
                for (int s = 0; s < samplesPerPixel; ++s) {
                    // Zufälliges Sub-Pixel Offset für Anti-Aliasing
                    float offsetX = (samplesPerPixel > 1) ? random01() : 0.5f;
                    float offsetY = (samplesPerPixel > 1) ? random01() : 0.5f;
                    
                    float u = float(x + offsetX) / float(width - 1);
                    float v = float(y + offsetY) / float(height - 1);
                    
                    // Erzeuge Strahl von Kamera durch Pixel
                    Ray ray = camera.getRay(u, v);
                    
                    // Berechne Farbe für diesen Strahl (mit Reflexionen)
                    pixelColor += shade(ray, 0);
                }
                
                // Mittelwertbildung über alle Samples
                pixelColor /= float(samplesPerPixel);
                
                // Konvertiere Float-Farben [0,1] zu 8-Bit-Integers [0,255]
                auto to8 = [](float f) { return (uint8_t)(glm::clamp(f, 0.0f, 1.0f) * 255.0f + 0.5f); };
                uint32_t R = to8(pixelColor.r), G = to8(pixelColor.g), B = to8(pixelColor.b), A = 255;
                
                // Pack RGBA in 32-bit Integer (ABGR-Format für OpenGL)
                pixels[y * width + x] = (A << 24) | (B << 16) | (G << 8) | (R);
            }
        }
        return pixels;
    }
};