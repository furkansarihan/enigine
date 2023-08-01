#ifndef transform_link_hpp
#define transform_link_hpp

#include <glm/glm.hpp>

class TransformLink
{
public:
    virtual ~TransformLink() {}

    virtual glm::mat4 getModelMatrix() = 0;
};

#endif /* transform_hpp */
