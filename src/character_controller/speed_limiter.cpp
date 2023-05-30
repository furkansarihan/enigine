#include "speed_limiter.h"

bool isAngleInRange(float angle, float start, float end);

// TODO: cache values - interpolate close hit
float SpeedLimiter::getSpeed(float angle)
{
    float pointsSum = 0.f;
    for (int i = 0; i < m_points.size(); i++)
        pointsSum += getValue(m_points[i], angle);

    return m_constantValue + pointsSum;
}

float SpeedLimiter::getValue(LimiterPoint &point, float a)
{
    // translate negative angle
    float angle = a;
    if (angle < 0.f)
        angle = 2.f * M_PI + angle;

    float rangeMin = point.angle - (M_PI_2 * point.lengthFactor);
    float rangeMax = point.angle + (M_PI_2 * point.lengthFactor);
    if (!isAngleInRange(angle, rangeMin, rangeMax))
        return 0.f;

    // ref look-up angle
    float refAngle = angle - point.angle;
    if (rangeMin < 0.f)
        rangeMin = 2.f * M_PI + rangeMin;
    if (rangeMax < 0.f)
        rangeMax = 2.f * M_PI + rangeMax;
    if (rangeMin > rangeMax)
    {
        if (a < 0.f)
            refAngle = point.angle + glm::abs(a);
    }

    float result = -glm::cos(refAngle / point.lengthFactor) * point.heightFactor;

    return result;
}

bool isAngleInRange(float angle, float start, float end)
{
    // // Normalize the angles to be within the range [0, 2π)
    // angle = fmod(angle, 2 * M_PI);
    // start = fmod(start, 2 * M_PI);
    // end = fmod(end, 2 * M_PI);

    if (start < 0.f)
        start = 2.f * M_PI + start;
    if (end < 0.f)
        end = 2.f * M_PI + end;

    if (start < end)
    {
        // Case 1: Range does not wrap around 2π
        return (angle >= start) && (angle <= end);
    }
    else
    {
        // Case 2: Range wraps around 2π
        return (angle >= start) || (angle <= end);
    }
}
