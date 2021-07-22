/**
 * @file Model.cpp
 * @brief A demonstration of the model loader class.
 */

#include <Scene.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <ModelLoader.hpp>
#include <Camera/Flycam.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Buffer.hpp>

//The namespace I used for this project
using namespace GL;

//All scense must have one class derived from Scene
class Star final : public Scene
{

    Shader shader{ROOT_Directory + "/shader/Star.vs", ROOT_Directory + "/shader/Star.fs"};
    Model model{ROOT_Directory + "/res/Models/star.obj"};
    Buffer buff; //A buffer holding offsets for multiple instances of the model

    //Standard projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, 100.0f);
    Camera3D cam;
    Flycam fc{&cam, loader->GetWindow()}; // A camera controller that allows free movement and rotation

    //This will be called once per frame
    void Render()
    {
        shader.Bind();
        //First move the camera
        fc.Update(loader->GetTimeInfo().RenderDeltaTime());
        //Compute a new mvp matrix, the model part is left out since I will store that info in the buffer
        shader.SetUniformMat4f("u_MVP", proj * cam.ComputeMatrix());

        //Draw free instances using the offsets in the buffer
        model.Draw(shader, 3);

        shader.UnBind();
    }

public:
    //All the setup is done here. During loading the update thread waits so you don't have to worry about thread safety here.
    Star(SceneLoader *_loader) : Scene(_loader)
    {
        RegisterFunc(CallbackType::Render, &Star::Render, this); //Register the method to be called each frame. This will be bound to the method automatically.

        //The location of the model instances
        float offsets[9]{
            2, 2, -2,
            -2, -2, -4,
            8, 8, -6};

        buff.Bind(GL_ARRAY_BUFFER);
        //Since nothing changes later on Static draw is used
        glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), offsets, GL_STATIC_DRAW);

        //This tells the model, how the instance buffer looks.
        //In this case attribure 3 will be 3 floats aka a vec3 and there are no other atrributes, so we can use that as the stride
        InstanceBufferLayout layout;
        layout.stride = 3 * sizeof(float);
        layout.attributes.push_back({GL_FLOAT, 3, 0});
        model.AddInstanceBuffer(layout, buff);

        //Sets a value in the loader, the menu checks this value to decide if it needs to be visible.
        GetFlag("hide_menu")= true;
    }

    //The threads will be synchronized here as well
    ~Star()
    {
        RemoveFunctions();//Remove all callbacks created by this class
    }
};

// Calls the constructor of Star
SCENE_LOAD_FUNC(Star)