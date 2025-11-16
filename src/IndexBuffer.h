#pragma once
#include "Debug.h"

class IndexBuffer {
private:
    unsigned int m_RendererID;
    unsigned int m_Count;

public:
    IndexBuffer(const unsigned int* indices, unsigned int count);
    ~IndexBuffer();
	void UpdateData(const unsigned int* indices, unsigned int count);
    unsigned int getCount() const { return m_Count; }

    void Bind() const;
    void Unbind() const;

    inline unsigned int GetCount() const { return m_Count; }
};
