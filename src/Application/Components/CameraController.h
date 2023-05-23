#pragma once

#include <glm/glm.hpp>

struct cbCamera
{
    glm::mat4 viewMat;
    glm::mat4 projectionMat;
    glm::vec3 camPos;
    float padding;
};

class CameraController
{
public:
    void Update ( float dt );

    bool updated = false;
    cbCamera gpuData {};
private:
    glm::vec3 postition{0, 0, 3};
    glm::vec3 rotation { 0 };
    glm::vec3 front{0, 0, -1};
    glm::vec3 up{0, 1, 0};
    float speed {0.03f};
};

