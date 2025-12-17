#include "IndexBuffer.h"
#include "Debug.h"
#include "BufferLimits.h"
#include <glad/glad.h>
#include <iostream>

IndexBuffer::IndexBuffer(const unsigned int* indices, unsigned int count)
    : m_Count(count) {
    
    // Plausibility check: Ensure index data is not null
    if (indices == nullptr) {
        std::cerr << "ERROR: IndexBuffer received null pointer for indices!" << std::endl;
        ASSERT(false);
    }
    
    // Plausibility check: Ensure index count is not zero
    if (count == 0) {
        std::cerr << "WARNING: IndexBuffer created with zero indices!" << std::endl;
    }
    
    // Plausibility check: Ensure index count doesn't exceed maximum limit
    // This prevents accidental upload of unreasonably large index buffers
    if (count > MAX_INDEX_COUNT) {
        std::cerr << "ERROR: Attempting to create IndexBuffer with " << count 
                  << " indices, which exceeds maximum allowed (" << MAX_INDEX_COUNT << ")!" << std::endl;
        std::cerr << "This may indicate a faulty mesh or parsing error." << std::endl;
        ASSERT(false);
    }
    
    GLCall(glGenBuffers(1, &m_RendererID));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() {
    GLCall(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::UpdateData(const unsigned int* indices, unsigned int count) {
    Bind();
    ASSERT(count <= m_Count); // Ensure buffer is large enough
    GLCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(unsigned int), indices));
}
