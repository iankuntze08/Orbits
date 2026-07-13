
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

struct State
{
    Body3 bMain;
    Body3 bSecondary;
    Body3 bCurrent;
};

const float dt = 0.01;

vec3 getGravitationalAccel(const State curState)
{
    vec3 dif = curState.bMain.pos - curState.bCurrent.pos;
    float dist2 = dot(dif, dif);
    float invDist = inversesqrt(dist2);
    float invDist3 = invDist * invDist * invDist;
    vec3 accel = dif * (curState.bMain.mass * invDist3);

    dif = curState.bSecondary.pos - curState.bCurrent.pos;
    dist2 = dot(dif, dif);
    invDist = inversesqrt(dist2);
    invDist3 = invDist * invDist * invDist;

    accel += dif * (curState.bSecondary.mass * invDist3);

    return accel;
}

vec3 getSecondaryAccel(State s)
{
    vec3 dif = s.bMain.pos - s.bCurrent.pos;

    float dist2 = dot(dif, dif);
    float invDist = inversesqrt(dist2);
    float invDist3 = invDist * invDist * invDist;

    return dif * (s.bMain.mass * invDist3);
}

Derivative3 evaluateDerivative(State curState, uint index)
{
    Derivative3 d;
    d.dPos = curState.bCurrent.vel;
    if (index != 1) d.dVel = getGravitationalAccel(curState);
    else d.dVel = getSecondaryAccel(curState);
    return d;
}

State rk4(State curState, uint index)
{
    Derivative3 k1 = evaluateDerivative(curState, index);

    Body3 b = curState.bCurrent;
    b.pos += k1.dPos * (dt * 0.5);
    b.vel += k1.dVel * (dt * 0.5);

    State s = curState;
    s.bCurrent = b;


    Derivative3 k2 = evaluateDerivative(s, index);

    b = curState.bCurrent;
    b.pos += k2.dPos * (dt * 0.5f);
    b.vel += k2.dVel * (dt * 0.5f);

    s = curState;
    s.bCurrent = b;

    Derivative3 k3 = evaluateDerivative(s, index);

    b = curState.bCurrent;
    b.pos += k3.dPos * dt;
    b.vel += k3.dVel * dt;

    s = curState;
    s.bCurrent = b;

    Derivative3 k4 = evaluateDerivative(s, index);


    curState.bCurrent.pos += ((dt / 6.0) * (k1.dPos + 2.0 * k2.dPos + 2.0 * k3.dPos + k4.dPos));
    curState.bCurrent.vel += ((dt / 6.0) * (k1.dVel + 2.0 * k2.dVel + 2.0 * k3.dVel + k4.dVel));

    return curState;
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
    uint idx = gl_GlobalInvocationID.x;
    if (idx == 0)
        return;
    if (idx >= count)
        return;

    Body3 mainBody = Body3(bodiesCurrent[0].pos.xyz, bodiesCurrent[0].vel.xyz, bodiesCurrent[0].mass);
    Body3 secondaryBody = Body3(bodiesCurrent[1].pos.xyz, bodiesCurrent[1].vel.xyz, bodiesCurrent[1].mass);

    Body3 currentBody = Body3(bodiesCurrent[idx].pos.xyz, bodiesCurrent[idx].vel.xyz, bodiesCurrent[idx].mass);
    
    State curState = State(mainBody, secondaryBody, currentBody);

    curState = rk4(curState, idx);

    bodiesNext[idx] = Body4(vec4(curState.bCurrent.pos, 0.0), vec4(curState.bCurrent.vel, 0.0), curState.bCurrent.mass);
}