#include "CameraController.h"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../Application.h"
#include "../../Tools/Input/Input.h"

#include <iostream>
void CameraController::Update ( float dt ) {

    bool anyButtonPressed = false;
    if ( Input::Input::GetKeyboardButton ( GLFW_KEY_W ).IsDown () ) {
        postition += front * speed * dt;
        anyButtonPressed |= true;
    }
    if ( Input::Input::GetKeyboardButton ( GLFW_KEY_S ).IsDown () )
    {

        postition -= front * speed * dt;
        anyButtonPressed |= true;
    }
    if ( Input::Input::GetKeyboardButton ( GLFW_KEY_A ).IsDown () )
    {
        postition -= glm::normalize ( glm::cross ( front, up ) ) * speed * dt;
        anyButtonPressed |= true;
    }
    if ( Input::Input::GetKeyboardButton ( GLFW_KEY_D ).IsDown () )
    {
        postition += glm::normalize ( glm::cross ( front, up ) ) * speed * dt;
        anyButtonPressed |= true;
    }

    if(Input::Input::GetMouseButton(GLFW_MOUSE_BUTTON_2).IsDown() )
    {
        glm::ivec2 move = Input::Input::GetMouse ().GetCursorMove ();
        glm::ivec2 screenRes = gApplication.GetAppInfo ().mScreenSize;

        glm::vec2 delta = glm::vec2 ( -move ) / glm::vec2 ( screenRes );

        if(delta != glm::vec2(0.f) ) anyButtonPressed |= true;

        rotation.x += delta.x * 100.f;
        rotation.y += delta.y * 100.f;

        glm::vec3 dir;
        dir.x = cos ( glm::radians ( rotation.x ) ) * cos ( glm::radians ( rotation.y ) );
        dir.y = sin ( glm::radians ( rotation.y ) ) ;
        dir.z = sin ( glm::radians ( rotation.x ) ) * cos ( glm::radians ( rotation.y ) );
        front = dir;
    }

    if( anyButtonPressed )
    {
        gpuData.viewMat = glm::lookAt ( postition, postition + front, up );
        gpuData.projectionMat = glm::perspective ( glm::radians ( 45.0f ), ( float ) 1920 / ( float ) 1080, 0.1f, 100.0f );
        gpuData.camPos = postition;
        updated = false;
    }
}
