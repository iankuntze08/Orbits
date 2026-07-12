#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/perpendicular.hpp>

class RotVec
{
    public:
    glm::vec3 axis;
    float angle;

    RotVec(glm::vec3 axis, float angle)
    {
        this->axis = axis;
        this->angle = angle;
    }

    glm::vec3 apply(glm::vec3& vec)
    {
        glm::vec3 p = glm::cross(axis, vec);
        vec = vec + (sin(angle) * (p)) + ((1.0f - cos(angle)) * glm::cross(axis, p));
        return vec;
    }
};