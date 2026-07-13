
#ifndef BODY_H
#define BODY_H

#include <glm.hpp>

struct Body
{
    glm::vec3 pos;
    glm::vec3 vel;
    float mass;
};

struct alignas(16) Body4
{
    glm::vec4 pos;
    glm::vec4 vel;
    float mass;
    glm::vec3 padding;
};
static_assert(sizeof(Body4) == 48);

#endif