//This is a simple voxel world

#include <Scene.hpp>
#include <glm/glm.hpp>
#include <Data.hpp>
#include <Voxel/Block.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Camera/Fplocked.hpp>
#include <Image/stb_image.h>
#include <Voxel/Chunk.hpp>
#include <Voxel/ConfigReader.hpp>
#include <Voxel/ChunkManager.hpp>
#include <Voxel/Inventory.hpp>
#include <Input.hpp>
#include <Buffer.hpp>
#include <VertexArray.hpp>

const double raydist = 8;   //How far the player can mine/place blocks
const double raysteps = 32; //How often the ray should be sampled
class Voxel_t final : public GL::Scene
{
    //The handle for the texture atlas
    uint tex_id;

    GL::Shader chunkShader{ROOT_Directory + "/shader/Voxel/Chunk.vs", ROOT_Directory + "/shader/Voxel/Block.fs"};

    //Shader for ui. Right now this means the square acting as the crosshair
    GL::Shader shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->GetWindow().GetWidth() / loader->GetWindow().GetHeigth(), 0.05f, 400.0f);

    GL::Camera3D camera{{0, 30, 0}};
    ///This camera controller does not allow roll or looking more than 90 degrees up.
    GL::Fplocked cameraController{&camera, loader->GetWindow(), 22};
    GL::Voxel::TexConfig blockTextures{ROOT_Directory + "/res/Textures/block.cfg"};
    GL::Voxel::ChunkManager chunks{blockTextures, loader->GetCallback()};

    GL::InputHandler::MouseCallback leftButton{*loader->GetWindow().inputptr, GLFW_MOUSE_BUTTON_LEFT, GL::InputHandler::Action::Press, &Voxel_t::Mine, this},
        right_button{*loader->GetWindow().inputptr, GLFW_MOUSE_BUTTON_RIGHT, GL::InputHandler::Action::Press, &Voxel_t::Place, this},
        middle_button{*loader->GetWindow().inputptr, GLFW_MOUSE_BUTTON_MIDDLE, GL::InputHandler::Action::Press, &Voxel_t::Pick, this};

    float crosshair[8]{0.005, 0.005, -0.005, 0.005, -0.005, -0.005, 0.005, -0.005};
    GL::Buffer vbo, ibo;
    GL::VertexArray vao;

    GL::Voxel::Inventory inventory;

    //Player data
    GL::Voxel::FileLayout file{ROOT_Directory + "/res/world/PLAYER"};

    //This key regenerates the world
    GL::InputHandler::KeyCallback r_key{*loader->GetWindow().inputptr};

    ///@brief Ray plane intersect. Used to determine which side of a block the player is looking at.
    glm::vec3 intersectPoint(glm::vec3 rayVector, glm::vec3 rayPoint, glm::vec3 planeNormal, glm::vec3 planePoint)
    {
        glm::vec3 diff = rayPoint - planePoint;
        float prod1 = glm::dot(diff, planeNormal);
        float prod2 = glm::dot(rayVector, planeNormal);
        float prod3 = prod1 / prod2;
        return rayPoint - rayVector * prod3;
    }

    ///@brief Mine the block the player is looking at
    void Mine(int)
    {
        auto ray_pos = camera.position;
        auto stepvec = camera.Forward() / raysteps * raydist;
        for (int i = 0; i < raysteps; i++)
        {
            ray_pos += stepvec;
            int x = round(ray_pos.x);
            int y = round(ray_pos.y);
            int z = round(ray_pos.z);
            y = std::clamp(y, 0, 63);

            uint *block = chunks.GetBlockAt(x, y, z);
            if (*block != GL::Voxel::BAir && *block != GL::Voxel::BWater)
            {
                if (GL::Voxel::IsStorable((GL::Voxel::BlockTypes)*block))
                {
                    inventory.AddTo(*block);
                }
                *block = 0;
                auto chunk = chunks.GetChunkPos(x, z);
                chunks.GetChunk(chunk)->regen_mesh = true;
                return;
            }
        }
    }

    ///@brief Select a block for placing
    void Pick(int)
    {
        auto ray_pos = camera.position;
        auto stepvec = camera.Forward() / raysteps * raydist;
        for (int i = 0; i < raysteps; i++)
        {
            ray_pos += stepvec;
            int x = round(ray_pos.x);
            int y = round(ray_pos.y);
            int z = round(ray_pos.z);
            y = std::clamp(y, 0, 63);

            uint *block = chunks.GetBlockAt(x, y, z);
            if (*block != GL::Voxel::BAir && *block != GL::Voxel::BWater)
            {
                inventory.Select((GL::Voxel::BlockTypes)*block);
                return;
            }
        }
    }

    ///@brief If there are blocks in the players inventory, place one.
    void Place(int)
    {
        auto ray_pos = camera.position;
        auto stepvec = camera.Forward() / raysteps * raydist;
        for (int i = 0; i < raysteps; i++)
        {
            ray_pos += stepvec;
            int x = round(ray_pos.x);
            int y = round(ray_pos.y);
            int z = round(ray_pos.z);
            y = std::clamp(y, 0, 63);

            uint *block = chunks.GetBlockAt(x, y, z);
            if (*block != GL::Voxel::BAir && *block != GL::Voxel::BWater)
            {
                glm::vec3 forward = camera.Forward();
                glm::vec3 intersect_face;
                for (int j = 0; j < 6; j++)
                {
                    if (glm::dot(GL::Voxel::bnormals[j], forward) < 0)
                    {
                        glm::vec3 p = intersectPoint(forward, (glm::vec3)camera.position, GL::Voxel::bnormals[j], GL::Voxel::bvertices[j * 6].pos + glm::vec3(x, y, z));
                        switch (j)
                        {
                        case 0:
                        Front:
                            if (p.x > x - 0.5 && p.x < x + 0.5 && p.y > y - 0.5 && p.y < y + 0.5)
                                intersect_face = GL::Voxel::bnormals[j];
                            break;
                        case 1:
                            goto Front;
                        case 2:
                        Bottom:
                            if (p.x > x - 0.5 && p.x < x + 0.5 && p.z > z - 0.5 && p.z < z + 0.5)
                                intersect_face = GL::Voxel::bnormals[j];
                            break;
                        case 3:
                            goto Bottom;
                        case 4:
                        Right:
                            if (p.y > y - 0.5 && p.y < y + 0.5 && p.z > z - 0.5 && p.z < z + 0.5)
                                intersect_face = GL::Voxel::bnormals[j];
                            break;
                        case 5:
                            goto Right;
                        }
                    }
                }

                auto block2 = chunks.GetBlockAt(x + intersect_face.x, y + intersect_face.y, z + intersect_face.z);
                if ((*block2 == GL::Voxel::BAir || *block2 == GL::Voxel::BWater) && inventory.GetCount() > 0)
                {
                    *block2 = inventory.GetSelected();
                    auto chunk = chunks.GetChunkPos(x + intersect_face.x, z + intersect_face.z);
                    chunks.GetChunk(chunk)->regen_mesh = true;
                    inventory.Remove();
                }
                return;
            }
        }
    }

    void Render()
    {
        auto dt = loader->GetTimeInfo().RenderDeltaTime();
        glm::ivec2 lastpos = {round(camera.position.x), round(camera.position.z)};
        cameraController.Update(dt);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id);

        chunkShader.Bind();
        chunkShader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        chunks.DrawChunks();

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        //The square in the middle.
        shader.Bind();
        vao.Bind();
        ibo.Bind(GL_ELEMENT_ARRAY_BUFFER);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        GL::Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
        GL::VertexArray::Unbind();
        shader.UnBind();

        if (chunks.HasCrossedChunk(lastpos, {round(camera.position.x), round(camera.position.z)}))
            chunks.MoveChunk(chunks.GetChunkPos({round(camera.position.x), round(camera.position.z)}));
    }

    void TexSetup();

public:
    Voxel_t(GL::SceneLoader *_loader) : Scene(_loader)
    {
        RegisterFunc(GL::CallbackType::Render, &Voxel_t::Render, this);
        chunkShader.Bind();
        chunkShader.SetUniform1i("u_Texture", 0); //The texture array is using slot 0

        TexSetup();
        chunkShader.UnBind();

        GetFlag("hide_menu") = 1;

        file.AddElement<glm::dvec3>(&camera.position);
        file.AddElement<double>(&cameraController.pitch);
        file.AddElement<double>(&cameraController.yaw);

        file.Load();

        chunks.LoadChunks(chunks.GetChunkPos(camera.position.x, camera.position.z));

        vao.Bind();
        vbo.Bind(GL_ARRAY_BUFFER);
        glVertexAttribPointer(0, 2, GL_FLOAT, 0, 2 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair), crosshair, GL_STATIC_DRAW);

        ibo.Bind(GL_ELEMENT_ARRAY_BUFFER);
        uint indices[]{
            0, 1, 2,
            2, 3, 0};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        GL::VertexArray::Unbind();
        GL::Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
        GL::Buffer::Unbind(GL_ARRAY_BUFFER);
        shader.Bind();

        glm::mat4 mat;

        auto &window = loader->GetWindow();

        //This makes sure the the square always looks the same.
        if (window.GetWidth() > window.GetHeigth())
        {
            mat = glm::ortho(-(float)window.GetWidth() / window.GetHeigth(), (float)window.GetWidth() / window.GetHeigth(), -1.0f, 1.0f);
        }
        else
        {
            mat = glm::ortho(-1.0f, 1.0f, -(float)window.GetHeigth() / window.GetWidth(), (float)window.GetHeigth() / window.GetWidth());
        }

        shader.SetUniformMat4f("u_MVP", mat);
        shader.SetUniform4f("u_Color", 1, 1, 1, 1);
        shader.UnBind();

        inventory.Load();

        //Regenerate World and reset player position.
        r_key.Bind(
            glfwGetKeyScancode(GLFW_KEY_R), GL::InputHandler::Action::Press, [&](int)
            {
                camera.position = glm::dvec3(0, 40, 0);
                cameraController.pitch = 0;
                cameraController.yaw = 0;
                chunks.Regenerate();
                inventory.Load();
            });
    }
    ~Voxel_t()
    {
        RemoveFunctions();
        file.Store();
        inventory.Store();
    }
};

//Load the texture array
void Voxel_t::TexSetup()
{
    stbi_set_flip_vertically_on_load(1);
    int w, h, bpp;
    auto local_buffer = stbi_load((ROOT_Directory + "/res/Textures/block.png").c_str(), &w, &h, &bpp, 4);

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 64, 64, h / 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    if (local_buffer)
        stbi_image_free(local_buffer);
}

SCENE_LOAD_FUNC(Voxel_t)