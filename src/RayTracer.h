#pragma once
#include <vector>
#include "vendor/glm/glm.hpp"
#include "Ray.h"
#include "Hit.h"
#include "Sphere.h"
#include "Box.h"
#include "Camera.h"

// Einfache RGB-Farbe für Ray Tracing Ergebnisse
struct RTColor { float r, g, b; };

// RayTracer: CPU-basierter Ray Tracer mit einfacher Lambert-Beleuchtung
// Rendert Spheres und Box-Objekte (AABBs) mit einem Hintergrund-Gradient
class RayTracer {
public:
    int width = 4;
    int height = 3;
    Camera camera;
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -1.0f, -0.5f)); // Richtungslicht

    // Szenen-Objekte: Spheres und AABBs können separat gerendert werden
    std::vector<Sphere> spheres;
    std::vector<Box> boxes;

    RayTracer(int w, int h) : width(w), height(h) {
        camera.aspect = float(w) / float(h);
        camera.update();
        // Standard-Szene: Boden-Sphere (großer Radius, um eine Ebene zu simulieren)
        spheres.emplace_back(glm::vec3(0, -100.5f, 0), 100.0f);
    }

    // Hintergrund: Blau-Weiß Gradient basierend auf Y-Richtung des Strahls
    RTColor background(const Ray& r) const {
        // Sky gradient: Interpolation zwischen Weiß (unten) und Blau (oben)
        glm::vec3 unitDir = glm::normalize(r.direction);
        float t = 0.5f * (unitDir.y + 1.0f); // Normalisiere Y von [-1,1] auf [0,1]
        glm::vec3 col = (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
        return { col.r, col.g, col.b };
    }

    // Shading: Berechnet Farbe für einen Strahl mit Lambert-Beleuchtung
    RTColor shade(const Ray& r) const {
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

        // Lambert-Beleuchtung: diffuse Reflexion basierend auf Winkel zwischen Normal und Licht
        float NdotL = glm::max(0.0f, glm::dot(closestRec.normal, -lightDir));
        
        // Basis-Farbe: Rot für Vorderseite, Grün für Rückseite (zur Visualisierung)
        glm::vec3 baseColor = closestRec.frontFace ? glm::vec3(0.8f, 0.3f, 0.3f) : glm::vec3(0.3f, 0.8f, 0.3f);
        
        // Finale Farbe: 10% Ambient + 90% Diffuse
        glm::vec3 col = (0.1f * baseColor) + (0.9f * baseColor * NdotL);
        return { col.r, col.g, col.b };
    }

    // Render: Hauptfunktion zum CPU-basierten Ray Tracing
    // Gibt RGBA8-Buffer zurück (32-bit pro Pixel: R, G, B, A)
    std::vector<uint32_t> render() {
        camera.update(); // Aktualisiere Kamera-Basis-Vektoren
        std::vector<uint32_t> pixels(width * height);
        
        // Für jedes Pixel im Bild...
        for (int y = 0; y < height; ++y) {
            float v = float(y) / float(height - 1); // Normalisierte Y-Koordinate [0,1]
            for (int x = 0; x < width; ++x) {
                float u = float(x) / float(width - 1); // Normalisierte X-Koordinate [0,1]
                
                // Erzeuge Strahl von Kamera durch Pixel
                // Y-Flip (1.0f - v) um GL-Koordinaten anzupassen
                Ray r = camera.getRay(u, 1.0f - v);
                
                // Berechne Farbe für diesen Strahl
                RTColor c = shade(r);
                
                // Konvertiere Float-Farben [0,1] zu 8-Bit-Integers [0,255]
                auto to8 = [](float f) { return (uint8_t)(glm::clamp(f, 0.0f, 1.0f) * 255.0f + 0.5f); };
                uint32_t R = to8(c.r), G = to8(c.g), B = to8(c.b), A = 255;
                
                // Pack RGBA in 32-bit Integer (ABGR-Format für OpenGL)
                pixels[y * width + x] = (A << 24) | (B << 16) | (G << 8) | (R);
            }
        }
        return pixels;
    }
};