#include <Module.hpp>
#include <glm/glm.hpp>
#include <Shader.hpp>
#include <vector>
#include <glad/glad.h>
#include <Data.hpp>
#include <cstring>

using GL::Shader;

struct Quad
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec4 color;
    };
    Vertex vertices[6];
};

struct Batch
{
    uint vb;
    uint va;
    Shader shader;

    std::vector<Quad> quads;

    Batch(std::string s_path_v, std::string s_path_f) : shader(s_path_v, s_path_f) {}
};

FUNCTION(void *, SetupBatch, uint, bool)
(uint n_Quads, bool dynamic)
{
    Batch *batch = new Batch(ROOT_Directory + "/shader/Batch.vs", ROOT_Directory + "/shader/Batch.fs");
    batch->quads.reserve(n_Quads);
    glGenBuffers(1, &batch->vb);
    glGenVertexArrays(1, &batch->va);
    glBindVertexArray(batch->va);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vb);
    glBufferData(GL_ARRAY_BUFFER, n_Quads * 6 * sizeof(Quad::Vertex), nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Quad::Vertex), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Quad::Vertex), (void *)offsetof(Quad::Vertex, color));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return batch;
}

FUNCTION(void *, SetupBatch)
()
{
    Batch *batch = new Batch(ROOT_Directory + "/shader/Batch.vs", ROOT_Directory + "/shader/Batch.fs");
    glGenBuffers(1, &batch->vb);
    glGenVertexArrays(1, &batch->va);
    glBindVertexArray(batch->va);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vb);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Quad::Vertex), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Quad::Vertex), (void *)offsetof(Quad::Vertex, color));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return batch;
}

FUNCTION(void, AddQuad, void *, glm::vec3 *, glm::vec4 *)
(void *_batch, glm::vec3 *v_pos, glm::vec4 *v_color)
{
    auto batch = reinterpret_cast<Batch *>(_batch);
    Quad quad;
    for (int i = 0; i < 6; i++)
    {
        quad.vertices[i].pos=v_pos[i];
        quad.vertices[i].color=v_color[i];
    }
    batch->quads.push_back(quad);
}

FUNCTION(void, ResizeBuffer, void *, bool)
(void *_batch, bool dynamic)
{
    auto batch = reinterpret_cast<Batch *>(_batch);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vb);
    glBufferData(GL_ARRAY_BUFFER, batch->quads.size() * 6 * sizeof(Quad::Vertex), &batch->quads[0].vertices[0], dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

FUNCTION(void, ResizeBuffer, void *, bool, uint)
(void *_batch, bool dynamic, uint count)
{
    auto batch = reinterpret_cast<Batch *>(_batch);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vb);
    glBufferData(GL_ARRAY_BUFFER, count * 6 * sizeof(Quad::Vertex), nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

FUNCTION(void, FillData, void *)
(void *_batch)
{
    auto batch = reinterpret_cast<Batch *>(_batch);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vb);
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch->quads.size() * 6 * sizeof(Quad::Vertex), &batch->quads[0].vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

FUNCTION(void, Draw, void *, glm::mat4)
(void *_batch, glm::mat4 mvp)
{
    auto batch = reinterpret_cast<Batch *>(_batch);
    batch->shader.Bind();
    batch->shader.SetUniformMat4f("u_MVP", mvp);
    glBindVertexArray(batch->va);
    glDrawArrays(GL_TRIANGLES, 0, batch->quads.size() * 6);
    glBindVertexArray(0);
}