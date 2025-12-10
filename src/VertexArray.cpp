#include "VertexArray.h"
#include "Debug.h"
#include <glad/glad.h>

VertexArray::VertexArray() {
    GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray() {
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
    Bind();
    vb.Bind();
    
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    
    for (unsigned int i = 0; i < elements.size(); i++) {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized,
            layout.GetStride(), (const void*)(size_t)offset));
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
    }
}

void VertexArray::SetIndexBuffer(std::unique_ptr<IndexBuffer> ib) {
    Bind();
    if (ib) {
        ib->Bind();
    }
    // IndexBuffer binding is saved as part of the VAO state in OpenGL.
    // When this VAO is bound later, the IndexBuffer will be automatically bound.
    m_IndexBuffer = std::move(ib);
}

void VertexArray::Bind() const {
    GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const {
    GLCall(glBindVertexArray(0));
}
