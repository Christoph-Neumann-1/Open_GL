/**
 * @file TrailRenderer.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <TrailRenderer.hpp>
#include <Data.hpp>

namespace GL
{
    TrailRenderer::TrailRenderer(uint nPoints, float lineWidth, glm::vec4 color)
        : m_shader(ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"), m_nPoints(nPoints), m_lineWidth(lineWidth), m_color(color)
    {
        m_points = new glm::vec3[m_nPoints];
        m_segments = new Segment[m_nPoints - 1];
        m_vertexBuffer.Bind(GL_ARRAY_BUFFER);
        VertexBufferLayout layout;
        layout.Push(GL_FLOAT, 3, 0);

        //TODO: Buffer mapping
        glBufferData(GL_ARRAY_BUFFER, (nPoints - 1) * sizeof(Segment), m_points, GL_DYNAMIC_DRAW);

        layout.BindAndAddToVertexArray(m_vertexArray, m_vertexBuffer);
        m_shader.Bind();
        m_shader.SetUniform4f("u_Color", m_color);
    }

    void TrailRenderer::Render(glm::mat4 view, glm::mat4 projection)
    {
        ComputeVertices(view);
        m_vertexArray.Bind();
        m_vertexBuffer.Bind(GL_ARRAY_BUFFER);
        m_shader.Bind();

        m_shader.SetUniformMat4f("u_MVP", projection);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (nRenderedPoints - 1) * sizeof(Segment), m_segments);
        //TODO: other primitive
        glDrawArrays(GL_TRIANGLES, 0, (nRenderedPoints - 1)*6);
        m_shader.UnBind();
        m_vertexArray.Unbind();
        m_vertexBuffer.Unbind(GL_ARRAY_BUFFER);
    }

    void TrailRenderer::ComputeVertices(const glm::mat4 &view)
    {
        for (uint i = 0; i < nRenderedPoints - 1; i++)
        {
            if(i==currentPoint-1)
                continue;
            auto &segment = m_segments[i];
            glm::vec3 begin = view * glm::vec4(m_points[i], 1);
            glm::vec3 end = view * glm::vec4(m_points[i + 1], 1);
            if (begin == end)
            {
                for (int j = 0; j < 6; j++)
                    segment.Vertices[j] = {0, 0, 0};
                return;
            }
            auto line = glm::normalize(end - begin);
            auto begin_normal = glm::normalize(glm::cross(line, begin));
            auto end_normal = glm::normalize(glm::cross(line, end));
            segment.Vertices[0] = begin + begin_normal * m_lineWidth;
            segment.Vertices[1] = begin - begin_normal * m_lineWidth;
            segment.Vertices[2] = end + end_normal * m_lineWidth;
            segment.Vertices[3] = end - end_normal * m_lineWidth;
            segment.Vertices[4] = end + end_normal * m_lineWidth;
            segment.Vertices[5] = begin - begin_normal * m_lineWidth;
        }
    }
}