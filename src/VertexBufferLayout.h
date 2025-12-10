#pragma once
#include <vector>
#include <glad/glad.h>

struct VertexBufferElement {
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    static unsigned int GetSizeOfType(unsigned int type);
};

class VertexBufferLayout {
private:
    unsigned int m_Stride;
    std::vector<VertexBufferElement> m_Elements;

public:
    VertexBufferLayout();

    void AddFloat(unsigned int count);
    void AddUnsignedInt(unsigned int count);
    void AddUnsignedByte(unsigned int count);

    inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
    inline unsigned int GetStride() const { return m_Stride; }

private:
    void Push(unsigned int type, unsigned int count, unsigned char normalized);
};
