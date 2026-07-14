#include <glm.hpp>
#include <iostream>

#include "Body.h"

#ifndef ORBIT_TRACKER_H
#define ORBIT_TRACKER_H

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
            std::cout << "new apoapsis at " << glm::length(apoapsis) << " u\n";
        }
        if (roundedMagnitude < glm::length(periapsis))
        {
            periapsis = roundedMagnitude * (focusBody.pos / mag);
            std::cout << "new periapsis at " << glm::length(periapsis) << " u\n"; 
        }

        eccentricity = (glm::length(apoapsis) - glm::length(periapsis)) /
            (glm::length(apoapsis) + glm::length(periapsis));
        sma = (glm::length(periapsis) + glm::length(apoapsis)) / 2.0;
    }
};

#endif