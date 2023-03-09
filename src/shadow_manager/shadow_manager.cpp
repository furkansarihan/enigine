#include "shadow_manager.h"

ShadowManager::ShadowManager()
{
    m_camera = new Camera(glm::vec3(0, 0, -16), glm::vec3(0, 1, 0), 90, 0, 1, 200);
}

ShadowManager::~ShadowManager()
{
    m_frustums.clear();
    m_depthPMatrices.clear();
}

// TODO: update cam values

// updateSplitDist computes the near and far distances for every frustum slice
// in camera eye space - that is, at what distance does a slice start and end
void ShadowManager::updateSplitDist(float nd, float fd)
{
    float lambda = m_splitWeight;
    float ratio = fd / nd;
    m_frustums.at(0).near = nd;

    for (int i = 1; i < m_splitCount; i++)
    {
        float si = i / (float)m_splitCount;

        m_frustums.at(i).near = lambda * (nd * powf(ratio, si)) + (1 - lambda) * (nd + (fd - nd) * si);
        m_frustums.at(i - 1).far = m_frustums.at(i).near * 1.005f;
    }
    m_frustums.at(m_splitCount - 1).far = fd;
}

// Compute the 8 corner points of the current view frustum
void ShadowManager::updateFrustumPoints(frustum &f, glm::vec3 &center, glm::vec3 &view_dir)
{
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 right = normalize(cross(view_dir, up));

    up = normalize(cross(right, view_dir));

    // view_dir must be normalized
    glm::vec3 fc = center + view_dir * f.far;
    glm::vec3 nc = center + view_dir * f.near;

    // TODO: why tan(fov/2) is not represents width?
    float near_height = tan(f.fov / 2.0f) * f.near;
    float near_width = near_height * f.ratio;
    float far_height = tan(f.fov / 2.0f) * f.far;
    float far_width = far_height * f.ratio;

    f.points[0] = nc - up * near_height - right * near_width;
    f.points[1] = nc + up * near_height - right * near_width;
    f.points[2] = nc + up * near_height + right * near_width;
    f.points[3] = nc - up * near_height + right * near_width;

    f.points[4] = fc - up * far_height - right * far_width;
    f.points[5] = fc + up * far_height - right * far_width;
    f.points[6] = fc + up * far_height + right * far_width;
    f.points[7] = fc - up * far_height + right * far_width;
}

// this function builds a projection matrix for rendering from the shadow's POV.
// First, it computes the appropriate z-range and sets an orthogonal projection.
// Then, it translates and scales it, so that it exactly captures the bounding box
// of the current frustum slice
glm::mat4 ShadowManager::applyCropMatrix(frustum &f, glm::mat4 lightView)
{
    float maxX, maxY, maxZ, minX, minY, minZ;

    glm::mat4 nv_mvp = lightView;
    glm::vec4 transf;

    // found min-max X, Y, Z
    transf = nv_mvp * glm::vec4(f.points[0], 1.0f);
    minX = transf.x;
    maxX = transf.x;
    minY = transf.y;
    maxY = transf.y;
    minZ = transf.z;
    maxZ = transf.z;
    for (int i = 1; i < 8; i++)
    {
        transf = nv_mvp * glm::vec4(f.points[i], 1.0f);

        if (transf.z > maxZ)
            maxZ = transf.z;
        if (transf.z < minZ)
            minZ = transf.z;

        // eliminate the perspective - for point lights
        transf.x /= transf.w;
        transf.y /= transf.w;

        if (transf.x > maxX)
            maxX = transf.x;
        if (transf.x < minX)
            minX = transf.x;
        if (transf.y > maxY)
            maxY = transf.y;
        if (transf.y < minY)
            minY = transf.y;
    }

    // TODO: extend borders with scene elements

    f.lightAABB[0] = glm::vec3(minX, minY, maxZ);
    f.lightAABB[1] = glm::vec3(maxX, minY, maxZ);
    f.lightAABB[2] = glm::vec3(maxX, maxY, maxZ);
    f.lightAABB[3] = glm::vec3(minX, maxY, maxZ);

    f.lightAABB[4] = glm::vec3(minX, minY, minZ);
    f.lightAABB[5] = glm::vec3(maxX, minY, minZ);
    f.lightAABB[6] = glm::vec3(maxX, maxY, minZ);
    f.lightAABB[7] = glm::vec3(minX, maxY, minZ);

    // TODO: why (-maxZ, -minZ)? direction is negative-z?
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -maxZ, -minZ);

    nv_mvp = projection * nv_mvp;

    float scaleX = 2.0f / (maxX - minX);
    float scaleY = 2.0f / (maxY - minY);
    float offsetX = -0.5f * (maxX + minX) * scaleX;
    float offsetY = -0.5f * (maxY + minY) * scaleY;

    // TODO: convert to these
    // projection = glm::scale(projection, glm::vec3(scaleX, scaleY, 1));
    // projection = glm::translate(projection, glm::vec3(offsetX, offsetY, 1));

    glm::mat4 crop = glm::mat4(1.0f);
    crop[0][0] = scaleX;
    crop[1][1] = scaleY;
    crop[0][3] = offsetX;
    crop[1][3] = offsetY;
    // TODO: why transpose?
    crop = glm::transpose(crop);

    projection = projection * crop;

    return projection;
}

void ShadowManager::setup(float screenWidth, float screenHeight)
{
    m_frustums.clear();
    m_depthPMatrices.clear();

    for (int i = 0; i < m_splitCount; i++)
    {
        // note that fov is in radians here and in OpenGL it is in degrees.
        // the 0.2f factor is important because we might get artifacts at
        // the screen borders.
        frustum frustum;
        frustum.fov = m_camera->fov + 0.2f;
        frustum.ratio = (double)screenWidth / (double)screenHeight;

        m_frustums.push_back(frustum);
    }

    updateSplitDist(m_near, m_far);

    for (int i = 0; i < m_splitCount; i++)
    {
        updateFrustumPoints(m_frustums.at(i), m_camera->position, m_camera->front);
    }

    glm::mat4 depthViewMatrix = getDepthViewMatrix();
    for (int i = 0; i < m_splitCount; i++)
    {
        glm::mat4 projection = applyCropMatrix(m_frustums.at(i), depthViewMatrix);
        m_depthPMatrices.push_back(projection);
    }
}

glm::mat4 ShadowManager::getDepthViewMatrix()
{
    return glm::lookAt(m_lightPos, m_lightLookAt, glm::vec3(0, 1, 0));
}
