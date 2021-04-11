
#pragma once

#include <unordered_map>
#include <string>

class SceneLoader;

/**
 * @brief Base class for all scenes.
 * 
 * Makes calling destructur easier.
 * 
 * @attention This class should never be instantited.
 */
class Scene
{
public:
    std::unordered_map<std::string,int> flags;
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