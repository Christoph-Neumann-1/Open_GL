#pragma once

#include <glm/glm.hpp>
#include <stdexcept>
#include <Drawable.hpp>

/**
 * @brief Used to draw curves on the screen. 
 */
class Curve : public Drawable
{

    glm::vec2 *vertices;
    unsigned int *indices;

    IndexBuffer ib;
    Shader shader;
    VertexBuffer vb;
    VertexArray va;

    unsigned int nPoints;
    glm::vec2 *Points;
    float color[4] = {1, 1, 1, 1};
    float thickness;

public:
    /**
     * @brief Construct a new Curve object.
     * 
     * Can be resized later if necessary. Use Update() before drawing it or after changing something.
     * 
     * @param n_Points Number of Points to draw a curve through must be at least 2
     * @param dynamic Do you want to modify the curve later on? Can still be modified but the buffer will use GL_STATIC_DRAW
     */
    explicit Curve(unsigned int n_Points = 2, bool dynamic = false);

    ~Curve()
    {
        delete[] Points;
        delete[] vertices;
        delete[] indices;
    }

    /**
    * @brief Updates the vertices and color of the curve.
    * Must be called at least once to initialize the curve.
    * 
    * @param thickness How thick the line should be.
    * @param color What color to make the line (RGBA)
    */
    void Update(const float thickness, const float color[4]);
    ///@overload
    void Update(const float thickness);
    ///@overload
    void Update(const float color[4]);
    ///@overload
    void Update();
    ///@brief Changes the color;
    void UpdateColor(const float color[4]);

    /**
 * @brief Resizes the curve. 
 * Please assign values to all points and call Update() afterwards.
 * 
 * @param n_Points How many points the curve should use.
 */
    void Resize(unsigned int n_Points);

    /// @return a const reference to the IndexBuffer
    const IndexBuffer &GetIndexBuffer() const override { return ib; }

    ///@return a reference to this curves Shader. The shader can be modified.
    Shader &GetShader() override { return shader; }

    ///@return a const reference to the VertexArray
    const VertexArray &GetVertexArray() const override { return va; }

    /**
 * @brief Access the coordinates of a point.
 * 
 * Throws std::invalid_argument if the point does not exist.
 * 
 * @param index The index starting at 0
 * @return a reference to the vector at the index
 */
    glm::vec2 &operator[](unsigned int index)
    {
        if (index < nPoints)
            return Points[index];
        else
            throw std::invalid_argument{"Curve: Index out of bounds"};
    }

    ///@return The number of points in the Curve.
    unsigned int GetNumberPoints() const { return nPoints; }

    ///@brief Makes a copy of the curve.
    Curve(const Curve &other);

    ///@brief Copy assignment.
    Curve &operator=(const Curve &other);

    ///@brief move assignment
    Curve &operator=(Curve &&other);
};