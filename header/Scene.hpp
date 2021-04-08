
#pragma once

class SceneLoader;

/**
 * @brief Base class for all scenes.
 * 
 * This class is used together with SceneLoader to dynamically load scenes.
 * Scene.hpp
 * 
 * @note Yes void* would also work to keep track of instances, but this makes the destructor thing easier. And I may need to add functionality.
 * 
 * @attention This class should never be instantited.
 */
class Scene
{
public:
    virtual ~Scene() {}
};

//Required:

// extern "C" void _INIT_(SceneLoader *loader);
// extern "C" void _EXIT_(Scene* scene);

/**
 * @file
 * @brief Base file for all scenes.
 * 
 * @details A valid scene **must** have the following two methods: 
 * 
 * @code{.cpp}
 * 
 * //return a pointer to a new Scene with all Methods of Scene implemented.
 * extern "C" Scene *_INIT_(SceneLoader *loader);
 * 
 * //delete the Scene.
 * extern "C" void _EXIT_(Scene* scene);
 * @endcode
 * 
 * as well as a class derived from Scene.
 */