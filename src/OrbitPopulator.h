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
#include "RandomNumberGen.h"

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

    void insertBodiesAtLoc(float dist, float incl, float angle, int count)
    {
        angle = glm::radians(angle);
        incl = glm::radians(incl);
        glm::vec3 normalPlane = glm::vec3(0.0, 1.0, 0.0);
        RotVec r = RotVec(glm::vec3(0.0, 0.0, 1.0), incl);

        for (int i = 0; i < count; i++)
        {
            glm::vec3 pos = glm::vec3(dist * cos(angle + (i / 100.0)), 0.0, dist * sin(angle));
            glm::vec3 vel = -glm::normalize(glm::cross(normalPlane, pos)) * (orbitalVelocity(dist, bodies[0].mass));

            vel = r.apply(vel);
            // pos = r.apply(pos);
            bodies.push_back(Body4{glm::vec4(pos, 0.0), glm::vec4(vel, 0.0), 0.1});
        }
    }

    void insertBodiesAtLocRandom(glm::vec3 pos, glm::vec3 vel, int count)
    {
        RandomNumberGenerator rng = RandomNumberGenerator();

        for (int i = 0; i < count; i++)
            bodies.push_back(Body4{
                glm::vec4(pos.x + rng.Generate(-0.01, 0.01), pos.y + rng.Generate(-0.01, 0.01), pos.z + rng.Generate(-0.01, 0.01), 0.0), 
                glm::vec4(vel.x + rng.Generate(-0.01, 0.01), vel.y + rng.Generate(-0.01, 0.01), vel.z + rng.Generate(-0.01, 0.01), 0.0), 
                0.1}
            );
    }
    void insertBodiesAtLocRandom(glm::vec3 pos, int count)
    {
        RandomNumberGenerator rng = RandomNumberGenerator();

        for (int i = 0; i < count; i++)
            bodies.push_back(Body4{
                glm::vec4(pos.x + rng.Generate(-0.01, 0.01), pos.y + rng.Generate(-0.01, 0.01), pos.z + rng.Generate(-0.01, 0.01), 0.0), 
                glm::vec4(rng.Generate(-0.01, 0.01), rng.Generate(-0.01, 0.01), orbitalVelocity(glm::length(pos), bodies[0].mass), 0.0),
                0.1});
    }

    std::vector<Body4> getBodies()
    {
        return this->bodies;
    }
};