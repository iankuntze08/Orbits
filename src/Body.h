
#ifndef BODY_H
#define BODY_H

#include <glm.hpp>

struct Body3
{
    glm::vec3 pos;
    glm::vec3 vel;
    float mass;
};

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

Body3 body4toBody3(const Body4& b)
{
    return Body3{glm::vec3(b.pos.x, b.pos.y, b.pos.z), glm::vec3(b.vel.x, b.vel.y, b.vel.z), b.mass};
}

#endif