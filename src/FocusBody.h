#include <glfw3.h>
#include <glm.hpp>

#include "Body.h"

#ifndef FOCUS_BODY_H
#define FOCUS_BODY_H

class FocusBody
{
    private:
    GLFWwindow* window;
    float timeAtLastSwap;

    public:
    unsigned int bodyIndex;
    unsigned int maxIndex;

    FocusBody(GLFWwindow* window, const std::vector<Body4>& bodies, int index)
    {
        this->window = window;
        this->maxIndex = index;
        this->bodyIndex = 0;
        this->timeAtLastSwap = 0.0;
    }

    void swap(const float& curSeconds, glm::mat4& modelMatrix, const std::vector<Body4>& bodies)
    {
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && curSeconds - timeAtLastSwap > 0.5f)
        {
            for (int i = 0; i < maxIndex; i++)
            {
                if (bodyIndex == i)
                {
                    bodyIndex = (i + 1) % maxIndex;
                    timeAtLastSwap = curSeconds;
                    break;
                }
            }
        }
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-bodies[bodyIndex].pos.x, -bodies[bodyIndex].pos.y, -bodies[bodyIndex].pos.z));
    }
};

#endif