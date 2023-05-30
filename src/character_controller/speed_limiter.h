#ifndef speed_limiter_hpp
#define speed_limiter_hpp

#include <string>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

struct LimiterPoint
{
    float angle;
    float heightFactor;
    float lengthFactor;

    LimiterPoint(float angle, float heightFactor, float lengthFactor)
        : angle(angle),
          heightFactor(heightFactor),
          lengthFactor(lengthFactor)
    {
    }
};

class SpeedLimiter
{
public:
    std::vector<LimiterPoint> m_points;
    float m_constantValue;

    float getSpeed(float angle);

private:
    float getValue(LimiterPoint &point, float angle);
};

#endif /* speed_limiter_hpp */
