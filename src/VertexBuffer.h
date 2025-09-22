#pragma once
#include "Debug.h"

class VertexBuffer {
private:
    unsigned int m_RendererID;

public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void UpdateData(const void* data, unsigned int size);
    void Bind() const;
    void Unbind() const;
};
