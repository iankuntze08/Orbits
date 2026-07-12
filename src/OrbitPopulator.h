#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/perpendicular.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <array>

#include "RotationVector.h"
#include "Body.h"

class OrbitPopulator
{
    private:

    float orbitalVelocity(float distance, float mass)
    {
        return sqrt(mass / distance);
    }

    float visViva(float sma, float distance, float mass)
    {
        return sqrt(mass * ((2.0 / distance) - (1.0 / sma)));
    }

    public:

    std::vector<Body> bodies;
    float distance;
    float inclination;

    OrbitPopulator(int numSatellites, float distanceFromCenter, float inclination)
    {
        this->distance = distanceFromCenter;
        bodies.reserve(numSatellites);
        this->inclination = inclination;
    }

    void insertBody(int index, Body body)
    {
        bodies.emplace(bodies.begin() + index, body);
    }

    void generate()
    {
        float dtheta = 6.28318531 / bodies.size();

        glm::vec3 normalPlane = glm::vec3(0.0);

        for (int i = 0; i < bodies.size(); i++)
        {
            float angle = dtheta * i;
            bodies[i].pos = glm::vec3(distance * cos(dtheta * angle), 0.0, distance * sin(dtheta * angle));

            glm::vec3 tangent = glm::cross(normalPlane, bodies[i].pos);
            tangent = glm::normalize(tangent);
            glm::vec3 vel = tangent * orbitalVelocity(distance, bodies[0].mass);
            glm::vec3 axis = glm::normalize(bodies[i].pos);

            RotVec r = RotVec(axis, inclination);
            bodies[i].vel = r.apply(vel);
        }
    }
};