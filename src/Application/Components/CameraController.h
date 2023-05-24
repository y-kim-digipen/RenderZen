#pragma once

#include <glm/glm.hpp>
#include "../../Renderer/BufferObjects.h"

class CameraController
{
public:
    void Update ( float dt );

    bool updated = false;
    ubCamera gpuData {};
private:
    glm::vec3 postition{0, 0, 3};
    glm::vec3 rotation { 0 };
    glm::vec3 front{0, 0, -1};
    glm::vec3 up{0, 1, 0};
    float speed {0.4f};
};

