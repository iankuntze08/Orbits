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
#include "Constants.h"

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

    std::vector<Body4> bodies;
    float distance;
    float inclination;
    int numSats;

    OrbitPopulator(int numSatellites, float distanceFromCenter, float inclination)
    {
        this->distance = distanceFromCenter;
        // bodies.reserve(numSatellites);
        this->inclination = glm::radians(inclination);
        this->numSats = numSatellites;
    }

    void insertBody(int index, Body4 body)
    {
        bodies.emplace(bodies.begin() + index, body);
    }

    void generate(int rings, float ringSpacing)
    {

        glm::vec3 normalPlane = glm::vec3(0.0, 1.0, 0.0);
        RotVec r = RotVec(glm::vec3(1.0, 0.0, 0.0), inclination);

        for (int i = 0; i < rings; i++)
        {
            float dtheta = rings * M_TAU / numSats;
            for (int q = 0; q < numSats / rings; q++)
            {
                bodies.emplace_back(Body4{glm::vec4(0.0), glm::vec4(0.0), 0.1});
                Body4& body = bodies.back();
                float angle = dtheta * q;

                glm::vec3 v3pos = glm::vec3(distance * cos(angle), 0.0, distance * sin(angle));

                glm::vec3 tangent = glm::normalize(glm::cross(normalPlane, v3pos));
                glm::vec3 v3vel = tangent * orbitalVelocity(distance, bodies[0].mass);

                body.vel = glm::vec4(r.apply(v3vel), 0.0);
                body.pos = glm::vec4(r.apply(v3pos), 0.0);
            }
            distance -= ringSpacing;
        }
    }

    std::vector<Body4> getBodies()
    {
        return this->bodies;
    }
};