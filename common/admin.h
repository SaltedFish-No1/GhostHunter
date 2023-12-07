#pragma once
# include"camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Add jump function
class Admin : public Camera
{
public:
    Admin(GLFWwindow* window) :Camera(window)
    {
        moveSpeed = 10.0f;
    }
    Admin(GLFWwindow* window, glm::vec3 pos) :Camera(window)
    {
        moveSpeed = 10.0f;
        viewPosition = pos;
    }
    //things that camera won't do
    
    void processInput();
protected:
    void fly(float deltaTime);
    void land(float deltaTime);
    void processKeyEvent();
};
