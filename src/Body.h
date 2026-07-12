
#ifndef BODY_H
#define BODY_H

#include <glm.hpp>

struct Body
{
    glm::vec3 pos;
    glm::vec3 vel;
    float mass;
};

#endif