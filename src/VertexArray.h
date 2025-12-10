#pragma once
#include <memory>
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

class VertexArray {
private:
    unsigned int m_RendererID;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;

public:
    VertexArray();
    ~VertexArray();

    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
    void SetIndexBuffer(std::unique_ptr<IndexBuffer> ib);
    
    void Bind() const;
    void Unbind() const;

    inline unsigned int GetIndexCount() const {
        return m_IndexBuffer ? m_IndexBuffer->GetCount() : 0;
    }
};
