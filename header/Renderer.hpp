///@file
#pragma once

#include <Drawable.hpp>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

/**
 * @brief Renders Object on screen.
 * 
 * Keeps track of View and Projection Matrix as well.
 * 
 */
class Renderer
{

public:
    glm::mat4 proj; ///< Projection Matrix
    glm::mat4 view; ///< View Matrix (aka Camera position)

    /**
 * @brief Draw a generic object on screen.
 * 
 * Bind Texture first if necessary.
 * 
 * @param va VertexArray
 * @param ib IndexBuffer
 * @param shader Shader u_MVP will be set
 * @param model glm::mat4 for position
 */
    void Draw(const VertexArray &va, const IndexBuffer &ib, Shader &shader, const glm::mat4 &model) const;

    /**
     * @brief Draw stuff.
     * 
     * @param obj a drawable object
     * @param model glm::mat4 for position
     */
    void Draw(Drawable *obj, const glm::mat4 &model) const;

};
