#include "VertexBufferLayout.h"

unsigned int VertexBufferElement::GetSizeOfType(unsigned int type) {
    switch (type) {
    case GL_FLOAT: return sizeof(GLfloat);
    case GL_UNSIGNED_INT: return sizeof(GLuint);
    case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
    }
    ASSERT(false);
    return 0;
}

VertexBufferLayout::VertexBufferLayout()
    : m_Stride(0) {
}

void VertexBufferLayout::AddFloat(unsigned int count) { Push(GL_FLOAT, count, GL_FALSE); }
void VertexBufferLayout::AddUnsignedInt(unsigned int count) { Push(GL_UNSIGNED_INT, count, GL_FALSE); }
void VertexBufferLayout::AddUnsignedByte(unsigned int count) { Push(GL_UNSIGNED_BYTE, count, GL_TRUE); }

void VertexBufferLayout::Push(unsigned int type, unsigned int count, unsigned char normalized) {
    m_Elements.push_back({ type, count, normalized });
    m_Stride += count * VertexBufferElement::GetSizeOfType(type);
}
