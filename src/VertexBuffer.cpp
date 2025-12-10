#include "VertexBuffer.h"
#include "Debug.h"
#include <glad/glad.h>

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
    : m_Size(size) {
    GLCall(glGenBuffers(1, &m_RendererID));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() {
    GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::UpdateData(const void* data, unsigned int size) {
    Bind();
    ASSERT(size <= m_Size); // Ensure buffer is large enough
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
}

void VertexBuffer::Bind() const {
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const {
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
