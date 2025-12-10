#pragma once
#include "vendor/glm/glm.hpp"

// Material: Beschreibt die physikalischen Eigenschaften einer Oberfläche
// Basiert auf PBR (Physically Based Rendering) Prinzipien
struct Material {
    glm::vec3 albedo;      // Grundfarbe (Basisfarbe der Oberfläche)
    float roughness;       // Rauheit: 0.0 = Spiegel, 1.0 = komplett diffus
    float metallic;        // Metallisch: 0.0 = Dielektrikum, 1.0 = Metall
    glm::vec3 emission;    // Selbstleuchtende Emission (für Lichtquellen)

    // Standard-Material: Weißes diffuses Material ohne Emission
    Material() 
        : albedo(0.8f, 0.8f, 0.8f)
        , roughness(0.9f)
        , metallic(0.0f)
        , emission(0.0f, 0.0f, 0.0f)
    {}

    Material(const glm::vec3& a, float r, float m, const glm::vec3& e = glm::vec3(0.0f))
        : albedo(a), roughness(r), metallic(m), emission(e)
    {}

    // ===== VORDEFINIERTE MATERIALIEN =====
    
    // Chrom: Hochreflektierendes Metall
    static Material Chrome() {
        return Material(glm::vec3(0.75f, 0.75f, 0.75f), 0.1f, 1.0f);
    }

    // Gold: Metallisch mit goldener Farbe
    static Material Gold() {
        return Material(glm::vec3(1.0f, 0.766f, 0.336f), 0.2f, 1.0f);
    }

    // Glas: Glatt und nicht-metallisch (später für Refraktion)
    static Material Glass() {
        return Material(glm::vec3(0.95f, 0.95f, 0.95f), 0.0f, 0.0f);
    }

    // Gummi: Sehr diffus und nicht-metallisch
    static Material Rubber() {
        return Material(glm::vec3(0.2f, 0.2f, 0.2f), 0.9f, 0.0f);
    }

    // Kupfer: Metallisch mit kupferner Farbe
    static Material Copper() {
        return Material(glm::vec3(0.95f, 0.64f, 0.54f), 0.15f, 1.0f);
    }

    // Diffuses Material mit benutzerdefinierter Farbe
    static Material Diffuse(const glm::vec3& color) {
        return Material(color, 0.9f, 0.0f);
    }

    // Emission: Selbstleuchtendes Material
    static Material Emissive(const glm::vec3& color, float intensity = 1.0f) {
        return Material(glm::vec3(0.0f), 1.0f, 0.0f, color * intensity);
    }
};
