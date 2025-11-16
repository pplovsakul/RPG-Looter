#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

class IndexBuffer;

class VertexArray {
private:
    unsigned int m_RendererID;
    std::unique_ptr<IndexBuffer> m_IndexBuffer;

public:
    VertexArray();
    ~VertexArray();

    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
    void Bind() const;
    void Unbind() const;

	void SetIndexBuffer(std::unique_ptr<IndexBuffer> ib) { 
        m_IndexBuffer = std::move(ib); 
    }
    unsigned int getIndexCount() const {
        return m_IndexBuffer ? m_IndexBuffer->getCount() : 0;
    }
};
