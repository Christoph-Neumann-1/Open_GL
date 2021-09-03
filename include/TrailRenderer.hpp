/**
 * @file TrailRenderer.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */

#pragma once

#include <Buffer.hpp>
#include <Shader.hpp>
#include <VertexArray.hpp>

class BallInBox;

namespace GL
{
    /**
     * @brief This class is used to render trails, or other curved lines.
     * 
     * The main capabilities of this class are:
     * add points, after a certain number of points, start overwriting the oldest points.
     * compute the vertices every frame using the camera position
     * render the points
     * 
     * //TODO: Fade effect
     * //TODO: find cause of the holes in the trail
     * //TODO: allow calling nextpoint from update thread
     */
    class TrailRenderer
    {
        friend class ::BallInBox;

        Buffer m_vertexBuffer;
        VertexArray m_vertexArray;
        Shader m_shader;
        uint m_nPoints;
        float m_lineWidth;
        glm::vec3 *m_points;
        glm::vec4 m_color;
        uint nRenderedPoints = 0;
        uint currentPoint = 0;
        struct Segment
        {
            glm::vec3 Vertices[6];
        };
        Segment *m_segments;

    public:
        TrailRenderer(uint nPoints, float lineWidth, glm::vec4 color);
        ~TrailRenderer()
        {
            delete[] m_points;
            delete[] m_segments;
        }
        void Render(glm::mat4 view, glm::mat4 projection);
        void NextPoint(glm::vec3 point)
        {
            if (currentPoint > 0 ? point == m_points[currentPoint - 1] : nRenderedPoints?point==m_points[m_nPoints-1]:false)
                return;
            if (currentPoint == m_nPoints)
                currentPoint = 0;
            m_points[currentPoint++] = point;
            if (nRenderedPoints < m_nPoints)
                nRenderedPoints++;
        }
        void Clear()
        {
            nRenderedPoints = 0;
            currentPoint = 0;
        }
        void SetColor(glm::vec4 color)
        {
            m_color = color;
        }
        void SetLineWidth(float lineWidth)
        {
            m_lineWidth = lineWidth;
        }

    private:
        void ComputeVertices(const glm::mat4 &view);
    };
}