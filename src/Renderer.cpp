///@file
#include "Renderer.hpp"

void Renderer::Draw(const VertexArray &va, const IndexBuffer &ib, Shader &shader, const glm::mat4 &model) const
{
    shader.Bind();
    shader.SetUniformMat4f("u_MVP", proj * view * model);
    va.Bind();
    ib.Bind();

    glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::Draw(Drawable *obj, const glm::mat4 &model) const
{
    obj->GetShader().Bind();
    obj->GetShader().SetUniformMat4f("u_MVP", proj * view * model);
    obj->GetVertexArray().Bind();
    obj->GetIndexBuffer().Bind();
    glDrawElements(GL_TRIANGLES, obj->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr);
}
