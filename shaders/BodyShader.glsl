
#version 430 core

struct Body
{
    vec3 pos;
    vec3 vel;
    float mass;
};

struct Derivative
{
    vec3 dPos;
    vec3 dVel;
};

const float dt = 0.01;

vec3 getGravitationalAccel(Body b0, Body b1)
{
    vec3 dif = b1.pos - b0.pos;
    float dist2 = dot(dif, dif);
    return normalize(dif) * (b1.mass / dist2);
} 

Derivative evaluateDerivative(Body b0, Body b1)
{
    Derivative d;
    d.dPos = b0.vel;
    d.dVel = getGravitationalAccel(b0, b1);
    return d;
}

Body rk4(Body b0, Body b1)
{
    Derivative k1 = evaluateDerivative(b0, b1);

    Body b = b0;
    b.pos += k1.dPos * (dt * 0.5);
    b.vel += k1.dVel * (dt * 0.5);

    Derivative k2 = evaluateDerivative(b, b1);

    b = b0;
    b.pos += k2.dPos * (dt * 0.5f);
    b.vel += k2.dVel * (dt * 0.5f);

    Derivative k3 = evaluateDerivative(b, b1);

    b = b0;
    b.pos += k3.dPos * dt;
    b.vel += k3.dVel * dt;

    Derivative k4 = evaluateDerivative(b, b1);


    b0.pos += ((dt / 6.0) * (k1.dPos + 2.0 * k2.dPos + 2.0 * k3.dPos + k4.dPos));
    b0.vel += ((dt / 6.0) * (k1.dVel + 2.0 * k2.dVel + 2.0 * k3.dVel + k4.dVel));

    return b0;
}

layout(local_size_x = 32) in;

layout(std430, binding = 0) buffer curPart
{
    Body bodies[];
};

// built in variables
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint  gl_LocalInvocationIndex;

uniform int count;

void main()
{
    uint idx = gl_GlobalInvocationID.x;
    if (idx == 0)
        return;
    if (idx >= count)
        return;
    Body b = bodies[idx];

    b = rk4(b, bodies[0]);

    bodies[idx] = b;
}