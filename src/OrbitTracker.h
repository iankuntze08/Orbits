#include <glm.hpp>
#include <iostream>

#include "Body.h"

#ifndef ORBIT_TRACKER_H
#define ORBIT_TRACKER_H

struct OrbitalParameter
{
    glm::vec4& pos;
    glm::vec4& vel;

    glm::vec3 apoapsis;
    glm::vec3 periapsis;
    float eccentricity;
    float trueAnomaly;
    float semiMajorAxis;
};

float calcTrueAnomaly(glm::vec3 apoapsis, glm::vec3 periapsis, glm::vec3 curPos, float eccentricity)
{
    glm::vec3 eccentricityVector = glm::normalize(apoapsis - periapsis) * eccentricity;
    return acos(glm::dot(eccentricityVector, curPos) / (glm::length(curPos) * glm::length(eccentricityVector)));
}

float calcEccentricity(float apoapsis, float periapsis)
{
    return (apoapsis - periapsis) / (apoapsis + periapsis);
}

class OrbitTracker
{
    private:

    public:
    Body3 focusBody;
    Body3 centralBody;
    glm::vec3 apoapsis;
    glm::vec3 periapsis;
    float eccentricity;
    float sma;
    float trueAnomaly;
    float decimalMultiplier;

    OrbitTracker(const Body4& focusBody, const Body4& centralBody, const int decimals)
    {
        this->focusBody = body4toBody3(focusBody);
        this->centralBody = body4toBody3(centralBody);

        apoapsis = focusBody.pos;
        periapsis = focusBody.pos;
        eccentricity = (glm::length(apoapsis) - glm::length(periapsis)) /
            (glm::length(apoapsis) + glm::length(periapsis));
        sma = (glm::length(periapsis) + glm::length(apoapsis)) / 2.0;
        trueAnomaly = 0.0;

        decimalMultiplier = pow(10, decimals);
    }

    void track(const Body4& focusBody, const Body4& centralBody)
    {
        this->focusBody = body4toBody3(focusBody);
        this->centralBody = body4toBody3(centralBody);
        
        float mag = glm::length(focusBody.pos);
        float roundedMagnitude = ((float)((int)(mag * decimalMultiplier))) / decimalMultiplier;
        if (roundedMagnitude > glm::length(apoapsis))
        {
            apoapsis = roundedMagnitude * (focusBody.pos / mag);
            // std::cout << "new apoapsis at " << glm::length(apoapsis) << " u\n";
        }
        if (roundedMagnitude < glm::length(periapsis))
        {
            periapsis = roundedMagnitude * (focusBody.pos / mag);
            // std::cout << "new periapsis at " << glm::length(periapsis) << " u\n"; 
        }

        eccentricity = calcEccentricity(glm::length(apoapsis), glm::length(periapsis));
        trueAnomaly = calcTrueAnomaly(this->apoapsis, this->periapsis, this->focusBody.pos, this->eccentricity);
        // std::cout << "True Anomaly: " << trueAnomaly << "\n";
        sma = (glm::length(periapsis) + glm::length(apoapsis)) / 2.0;
    }
};

#endif