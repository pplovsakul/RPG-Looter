#pragma once
#include <vector>
#include "vendor/glm/glm.hpp"
#include "RayTracer.h"
#include "Debug.h"
#include <glad/glad.h>

// RayTraceRenderer: Lädt CPU-gerendertes Bild als OpenGL-Textur und zeigt es auf einem Fullscreen-Quad an
// Workflow:
// 1. CPU berechnet Bild via RayTracer::render() (langsam, aber physikalisch korrekt)
// 2. Bild wird als GL_TEXTURE_2D hochgeladen
// 3. Fullscreen-Quad wird mit dieser Textur gerendert
class RayTraceRenderer {
public:
    int width, height;
    GLuint texture = 0;  // OpenGL Textur-Handle
    GLuint vao = 0, vbo = 0; // Vertex Array Object und Buffer für Fullscreen-Quad

    RayTracer tracer; // Der eigentliche CPU Ray Tracer

    RayTraceRenderer(int w, int h)
        : width(w), height(h), tracer(w, h) {
        
        // ===== TEXTUR ERSTELLEN =====
        // Erstelle leere Textur mit entsprechender Auflösung
        GLCall(glGenTextures(1, &texture));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        // Nearest-Neighbor Filtering (kein Blur, Pixel bleiben scharf)
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        // Allokiere Textur-Speicher (noch keine Daten)
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));

        // ===== FULLSCREEN QUAD ERSTELLEN =====
        // Quad in NDC (Normalized Device Coordinates): von -1 bis +1 in X und Y
        // Mit Textur-Koordinaten (UV) von 0 bis 1
        float quad[16] = {
            // Position   // UV-Koordinaten
            -1.0f, -1.0f, 0.0f, 0.0f, // Unten-Links
             1.0f, -1.0f, 1.0f, 0.0f, // Unten-Rechts
             1.0f,  1.0f, 1.0f, 1.0f, // Oben-Rechts
            -1.0f,  1.0f, 0.0f, 1.0f  // Oben-Links
        };
        
        // VAO und VBO Setup
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));
        GLCall(glGenBuffers(1, &vbo));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW));
        
        // Attribute 0: Position (2 floats)
        GLCall(glEnableVertexAttribArray(0));
        GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        
        // Attribute 1: UV (2 floats)
        GLCall(glEnableVertexAttribArray(1));
        GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
        
        GLCall(glBindVertexArray(0));
    }

    ~RayTraceRenderer() {
        // Cleanup: OpenGL-Ressourcen freigeben
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        if (texture) glDeleteTextures(1, &texture);
    }

    void draw(GLuint shaderProgram) {
        // ===== 1. CPU RAY TRACING =====
        // Berechne Bild auf der CPU (kann mehrere Sekunden dauern bei hoher Auflösung)
        std::vector<uint32_t> pixels = tracer.render();

        // ===== 2. TEXTUR UPLOAD =====
        // Lade berechnetes Bild in GPU-Textur
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data()));
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));

        // ===== 3. FULLSCREEN QUAD RENDERING =====
        // Verwende Shader, der die Textur auf dem Quad anzeigt
        GLCall(glUseProgram(shaderProgram));
        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, texture));
        
        // Setze Shader-Uniform: u_Texture = Textur-Einheit 0
        GLint loc = glGetUniformLocation(shaderProgram, "u_Texture");
        if (loc >= 0) glUniform1i(loc, 0);

        // Zeichne Quad (als Triangle Fan: 4 Vertices)
        GLCall(glBindVertexArray(vao));
        GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
        GLCall(glBindVertexArray(0));
        
        // Cleanup
        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        GLCall(glUseProgram(0));
    }
};