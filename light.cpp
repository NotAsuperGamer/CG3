#include "light.h"

Light::Light(const Vec3f& direction)
    : direction_(direction.normalized()),
      color_(1.0f, 1.0f, 1.0f),
      intensity_(1.0f)
{
}

Light::Light(const Vec3f& direction, const Vec3f& color, float intensity)
    : direction_(direction.normalized()),
      color_(color),
      intensity_(intensity)
{
}
