///@file
#include <Curve.hpp>
#include <Data.hpp>
#include <cmath>
#include <cstring>

Curve::Curve(unsigned int n_Points, bool dynamic)
    : shader(Shader(ROOT_Directory + "/res/Shaders/Line.glsl")), vb((n_Points - 1) * 4 * 2 * sizeof(float), nullptr, dynamic), nPoints(n_Points), thickness(0.0f)
{
    if (nPoints < 2)
    {
        throw std::invalid_argument{"Invalid Curve: Tried creating Curve with less than 2 Points."};
    }
    vertices = new glm::vec2[(nPoints - 1) * 4];
    memset(vertices, 0, (nPoints - 1) * 4 * sizeof(glm::vec2));
    indices = new unsigned int[(nPoints - 1) * 6];
    Points = new glm::vec2[nPoints];

    for (unsigned int i = 0; i < n_Points - 1; i++)
    {
        indices[i * 6] = i * 4;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        indices[i * 6 + 3] = i * 4 + 1;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }

    ib.SetData(indices, (nPoints - 1) * 6);
    va.Bind();
    VertexBufferLayout layout;
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);
    memset(color, 0, 4 * sizeof(float));
}

void Curve::Update(const float thickness, const float color[4])
{
    UpdateColor(color);
    this->thickness = thickness;
    Update();
}

void Curve::Update(const float thickness)
{
    this->thickness = thickness;
    Update();
}

void Curve::Update(const float color[4])
{
    UpdateColor(color);
    Update();
}

void Curve::Update()
{
    if (Points == nullptr)
    {
        throw std::logic_error{"Curve not initialized correctly. Please check how previous exceptions were handled."};
    }

    for (unsigned int i = 0; i < nPoints - 1; i++)
    {
        glm::vec2 *pos = &vertices[i * 4];
        glm::vec2 &start = Points[i];
        glm::vec2 &end = Points[i + 1];
        glm::vec2 nv = end - start;
        float length = sqrtf(nv.x * nv.x + nv.y * nv.y);
        nv = glm::vec2(-nv.y / length, nv.x / length);
        nv *= thickness;
        pos[0] = start + nv;
        pos[1] = start - nv;
        pos[2] = end + nv;
        pos[3] = end - nv;
    }
    vb.Bind();
    vb.SetData((nPoints - 1) * 4 * 2 * sizeof(float), vertices);
}

void Curve::UpdateColor(const float color[4])
{
    shader.Bind();
    shader.SetUniform4f("u_Color", color);
    std::memcpy(this->color, color, 4 * sizeof(float));
}

void Curve::Resize(unsigned int n_Points)
{
    if (nPoints < 2)
    {
        throw std::invalid_argument{"Invalid Curve. Tried setting Points to less than 2."};
    }

    delete[] Points;
    delete[] vertices;
    delete[] indices;
    nPoints = n_Points;

    vertices = new glm::vec2[(nPoints - 1) * 4];
    memset(vertices, 0, (nPoints - 1) * 4 * sizeof(glm::vec2));
    indices = new unsigned int[(nPoints - 1) * 6];
    Points = new glm::vec2[nPoints];

    for (unsigned int i = 0; i < n_Points - 1; i++)
    {
        indices[i * 6] = i * 4;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        indices[i * 6 + 3] = i * 4 + 1;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }

    ib.SetData(indices, (nPoints - 1) * 6);
    vb.SetSize((nPoints - 1) * 4 * 2 * sizeof(float), vertices, true);
}

Curve::Curve(const Curve &other)
    : ib(other.indices, (other.nPoints - 1) * 6), shader(Shader(ROOT_Directory + "/res/Shaders/Line.glsl")),
      vb((other.nPoints - 1) * 4 * 2 * sizeof(float), other.vertices), va(), nPoints(other.nPoints),
      thickness(other.thickness)
{
    vertices = new glm::vec2[(nPoints - 1) * 4];
    std::memcpy(vertices, other.vertices, (nPoints - 1) * 4 * sizeof(glm::vec2));
    indices = new unsigned int[(nPoints - 1) * 6];
    std::memcpy(indices, other.indices, (nPoints - 1) * 6 * sizeof(unsigned int));
    Points = new glm::vec2[nPoints];
    std::memcpy(Points, other.Points, nPoints * sizeof(glm::vec2));

    UpdateColor(other.color);

    va.Bind();
    VertexBufferLayout layout;
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);
    vb.Bind();
    vb.SetSize((nPoints - 1) * 4 * 2 * sizeof(float), vertices);
}

Curve &Curve::operator=(const Curve &other)
{
    if (&other == this)
        return *this;

    delete[] vertices;
    delete[] indices;
    delete[] Points;

    nPoints = other.nPoints;

    vertices = new glm::vec2[(nPoints - 1) * 4];
    std::memcpy(vertices, other.vertices, (nPoints - 1) * 4 * sizeof(glm::vec2));
    indices = new unsigned int[(nPoints - 1) * 6];
    std::memcpy(indices, other.indices, (nPoints - 1) * 6 * sizeof(unsigned int));
    Points = new glm::vec2[nPoints];
    std::memcpy(Points, other.Points, nPoints * sizeof(glm::vec2));
    UpdateColor(other.color);
    thickness = other.thickness;

    ib.SetData(indices, (nPoints - 1) * 6);
    vb.Bind();
    vb.SetSize((nPoints - 1) * 4 * sizeof(glm::vec2), vertices);

    return *this;
}

Curve &Curve::operator=(Curve &&other)
{

    delete[] vertices;
    delete[] indices;
    delete[] Points;

    nPoints = other.nPoints;

    vertices = other.vertices;
    other.vertices = nullptr;
    indices = other.indices;
    other.indices = nullptr;
    Points = other.Points;
    other.Points = nullptr;
    UpdateColor(color);
    thickness = other.thickness;

    ib.SetData(indices, (nPoints - 1) * 6);
    vb.Bind();
    vb.SetSize((nPoints - 1) * 4 * 2 * sizeof(float), vertices);

    return *this;
}