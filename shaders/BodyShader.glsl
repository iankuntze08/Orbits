
#version 430 core

struct Body3
{
    vec3 pos;
    vec3 vel;
    float mass;
};

struct Body4
{
    vec4 pos;
    vec4 vel;
    float mass;
};

struct Derivative3
{
    vec3 dPos;
    vec3 dVel;
};

const float dt = 0.01;

vec3 getGravitationalAccel(const Body3 b0, const Body3 b1)
{
    vec3 dif = b1.pos - b0.pos;
    float dist2 = dot(dif, dif);
    return normalize(dif) * (b1.mass / dist2);
} 

Derivative3 evaluateDerivative(const Body3 b0, const Body3 b1)
{
    Derivative3 d;
    d.dPos = b0.vel;
    d.dVel = getGravitationalAccel(b0, b1);
    return d;
}

Body3 rk4(Body3 b0, const Body3 b1)
{
    Derivative3 k1 = evaluateDerivative(b0, b1);

    Body3 b = b0;
    b.pos += k1.dPos * (dt * 0.5);
    b.vel += k1.dVel * (dt * 0.5);

    Derivative3 k2 = evaluateDerivative(b, b1);

    b = b0;
    b.pos += k2.dPos * (dt * 0.5f);
    b.vel += k2.dVel * (dt * 0.5f);

    Derivative3 k3 = evaluateDerivative(b, b1);

    b = b0;
    b.pos += k3.dPos * dt;
    b.vel += k3.dVel * dt;

    Derivative3 k4 = evaluateDerivative(b, b1);


    b0.pos += ((dt / 6.0) * (k1.dPos + 2.0 * k2.dPos + 2.0 * k3.dPos + k4.dPos));
    b0.vel += ((dt / 6.0) * (k1.dVel + 2.0 * k2.dVel + 2.0 * k3.dVel + k4.dVel));

    return b0;
}

layout(local_size_x = 32) in;

layout(std430, binding = 0) readonly buffer curPart
{
    Body4 bodiesCurrent[];
};

layout(std430, binding = 1) writeonly buffer nextPart
{
    Body4 bodiesNext[];
};

// built in variables
// in uvec3 gl_NumWorkGroups;
// in uvec3 gl_WorkGroupID;
// in uvec3 gl_LocalInvocationID;
// in uvec3 gl_GlobalInvocationID;
// in uint  gl_LocalInvocationIndex;

uniform uint count;  

void main()
{
    Body3 mainBody = Body3(bodiesCurrent[0].pos.xyz, bodiesCurrent[0].vel.xyz, bodiesCurrent[0].mass);

    uint idx = gl_GlobalInvocationID.x;
    if (idx == 0)
        return;
    if (idx >= count)
        return;
    Body3 b = Body3(bodiesCurrent[idx].pos.xyz, bodiesCurrent[idx].vel.xyz, bodiesCurrent[idx].mass);

    b = rk4(b, mainBody);

    bodiesNext[idx] = Body4(vec4(b.pos, 0.0), vec4(b.vel, 0.0), b.mass);
}