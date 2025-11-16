#include "Renderer.h"
#include <glad/glad.h>
#include "Debug.h"
#include <iostream>

void Renderer::Clear() const {
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
    shader.Bind();
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::Draw_TrianglesFan(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
    shader.Bind();
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_TRIANGLE_FAN, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
};
void Renderer::Draw_RenderObject(RenderObject& obj,Shader& shader, const glm::mat4& proj, const glm::vec2& pos) const {
	shader.Bind();
	obj.va->Bind();
	obj.ib->Bind();
    obj.UpdateModelMatrix(pos);
   

    glm::mat4 mvp = proj * obj.model;
    
    shader.SetUniformMat4f("u_MVP", mvp);
	if (obj.texture) {
		obj.texture->Bind(0);
		shader.SetUniform1i("u_Texture", 0);
	}
	shader.SetUniform4f("u_color", obj.color.r, obj.color.g, obj.color.b, obj.color.a);
	GLCall(glDrawElements(GL_TRIANGLES, obj.ib->GetCount(), GL_UNSIGNED_INT, nullptr));
}

RenderObject::RenderObject(const void* vertices, size_t vertexSize,
    const std::vector<unsigned int>& indices,
    const glm::vec2& pos,
    std::unique_ptr<Texture> texture
)
    : va(std::make_unique<VertexArray>()),
    vb(std::make_unique<VertexBuffer>(vertices, vertexSize)),
    ib(std::make_unique<IndexBuffer>(indices.data(), indices.size())),
    texture(std::move(texture)),
    position(pos),
	m_visible(true)
{
    // --- Automatisches Layout ---
    layout.AddFloat(3); // Position
    layout.AddFloat(2); // TexCoord

    va->AddBuffer(*vb, layout);

}

void RenderObject::UpdateTexture(std::unique_ptr<Texture> newTex) {
    texture = std::move(newTex);
}

void RenderObject::UpdateVertices(const void* vertices, size_t vertexSize) {
    vb->UpdateData(vertices, vertexSize); 
}

void RenderObject::UpdateIndices(const std::vector<unsigned int>& indices) {
    ib->UpdateData(indices.data(), indices.size());
}

void RenderObject::UpdateColor(const glm::vec4& newColor) {
    color = newColor;
}

//neu->
void RenderObject::UpdateModelMatrix(const glm::vec2& pos) {
    position = pos;
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(position, 0.0f));
    //m = glm::rotate(m, glm::radians(rotationDeg), glm::vec3(0.0f, 0.0f, 1.0f));
    //m = glm::scale(m, glm::vec3(scale, 1.0f));
    model = m;


}

void RenderObject::setVisible(bool visible) {
    m_visible = visible;
}

bool RenderObject::isVisible() {
    return m_visible;
}