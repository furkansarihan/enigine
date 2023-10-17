#include "shadowmap_ui.h"

void ShadowmapUI::render()
{
    if (!ImGui::CollapsingHeader("Shadowmap", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("drawShadowmap", &m_drawShadowmap);
    ImGui::Checkbox("drawFrustum", &m_drawFrustum);
    ImGui::Checkbox("drawFrustumAABB", &m_drawFrustumAABB);
    ImGui::Checkbox("drawAABB", &m_drawAABB);
    ImGui::DragFloat("quadScale", &m_quadScale, 0.1f);
    ImGui::DragFloat("splitWeight", &m_shadowManager->m_splitWeight, 0.01f);
    ImGui::Separator();
    ImGui::Text("Light");
    ImGui::DragFloat("X", &m_shadowManager->m_lightPos.x, 0.01f);
    ImGui::DragFloat("Y", &m_shadowManager->m_lightPos.y, 0.01f);
    ImGui::DragFloat("Z", &m_shadowManager->m_lightPos.z, 0.01f);
    m_shadowManager->m_lightPos = glm::normalize(m_shadowManager->m_lightPos);
    ImGui::Text("Light - look at");
    ImGui::DragFloat("llaX", &m_shadowManager->m_lightLookAt.x, 0.01f);
    ImGui::DragFloat("llaY", &m_shadowManager->m_lightLookAt.y, 0.01);
    ImGui::DragFloat("llaZ", &m_shadowManager->m_lightLookAt.z, 0.01);
    ImGui::Text("camPos");
    ImGui::DragFloat("camPosX", &m_shadowManager->m_camera->position.x, 0.5f);
    ImGui::DragFloat("camPosY", &m_shadowManager->m_camera->position.y, 0.5f);
    ImGui::DragFloat("camPosZ", &m_shadowManager->m_camera->position.z, 0.5f);
    ImGui::Text("camView");
    ImGui::DragFloat("camViewX", &m_shadowManager->m_camera->front.x, 0.01f);
    ImGui::DragFloat("camViewY", &m_shadowManager->m_camera->front.y, 0.01f);
    ImGui::DragFloat("camViewZ", &m_shadowManager->m_camera->front.z, 0.01f);
    // m_shadowManager->m_camera->front = glm::normalize(m_shadowManager->m_camera->front);
    ImGui::DragFloat("camNear", &m_shadowManager->m_near, 1);
    ImGui::DragFloat("camFar", &m_shadowManager->m_far, 1, 26, 10000);
}

void ShadowmapUI::drawFrustum(Shader &simpleShader, glm::mat4 mvp, unsigned int c_vbo, unsigned int c_vao, unsigned int c_ebo)
{
    if (!m_drawFrustum)
        return;

    for (int i = 0; i < m_shadowManager->m_splitCount; i++)
    {
        GLuint indices[] = {
            // Near plane
            0, 1, 1, 2, 2, 3, 3, 0,
            // Far plane
            4, 5, 5, 6, 6, 7, 7, 4,
            // Connections between planes
            0, 4, 1, 5, 2, 6, 3, 7};

        glBindVertexArray(c_vao);

        glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), m_shadowManager->m_frustums[i].points, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(float), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        simpleShader.use();
        simpleShader.setMat4("MVP", mvp);
        simpleShader.setMat4("u_meshOffset", glm::mat4(1.0));
        simpleShader.setVec4("DiffuseColor", glm::vec4(0.0, 1.0, 1.0, 1.0f));

        glBindVertexArray(c_vao);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void ShadowmapUI::drawFrustumAABB(Shader &simpleShader, glm::mat4 mvp, unsigned int c_vbo, unsigned int c_vao, unsigned int c_ebo)
{
    if (!m_drawFrustumAABB)
        return;

    for (int i = 0; i < m_shadowManager->m_splitCount; i++)
    {
        glm::vec3 minPoint = m_shadowManager->m_aabb.min;
        glm::vec3 maxPoint = m_shadowManager->m_aabb.max;

        glm::vec3 vertices[] = {
            glm::vec3(minPoint.x, minPoint.y, minPoint.z),
            glm::vec3(maxPoint.x, minPoint.y, minPoint.z),
            glm::vec3(maxPoint.x, maxPoint.y, minPoint.z),
            glm::vec3(minPoint.x, maxPoint.y, minPoint.z),
            glm::vec3(minPoint.x, minPoint.y, maxPoint.z),
            glm::vec3(maxPoint.x, minPoint.y, maxPoint.z),
            glm::vec3(maxPoint.x, maxPoint.y, maxPoint.z),
            glm::vec3(minPoint.x, maxPoint.y, maxPoint.z)};

        int indices[] = {
            0, 1, 2, 2, 3, 0, // Front face
            1, 5, 6, 6, 2, 1, // Right face
            5, 4, 7, 7, 6, 5, // Back face
            4, 0, 3, 3, 7, 4, // Left face
            3, 2, 6, 6, 7, 3, // Top face
            4, 5, 1, 1, 0, 4, // Bottom face
        };

        glBindVertexArray(c_vao);

        glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(float), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        simpleShader.use();
        simpleShader.setMat4("MVP", mvp);
        simpleShader.setMat4("u_meshOffset", glm::mat4(1.0));
        simpleShader.setVec4("DiffuseColor", glm::vec4(1.0, 0.0, 1.0, 0.2f));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(c_vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);

        simpleShader.setVec4("DiffuseColor", glm::vec4(0.0, 0.0, 0.0, 1.0f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ShadowmapUI::drawLightAABB(Shader &simpleShader, glm::mat4 mvp, glm::mat4 inverseDepthViewMatrix, unsigned int c_vbo, unsigned int c_vao, unsigned int c_ebo)
{
    if (!m_drawAABB)
        return;

    for (int i = 0; i < m_shadowManager->m_splitCount; i++)
    {
        // Define indices
        GLuint indices[] = {
            0, 1, 2, 2, 3, 0, // Front face
            1, 5, 6, 6, 2, 1, // Right face
            5, 4, 7, 7, 6, 5, // Back face
            4, 0, 3, 3, 7, 4, // Left face
            3, 2, 6, 6, 7, 3, // Top face
            0, 1, 5, 5, 4, 0, // Bottom face
        };

        glBindVertexArray(c_vao);

        glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), m_shadowManager->m_frustums[i].lightAABB, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(float), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        simpleShader.use();
        simpleShader.setMat4("MVP", mvp * inverseDepthViewMatrix);
        simpleShader.setMat4("u_meshOffset", glm::mat4(1.0));
        glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 0.2f);
        color[i] *= 0.7;
        simpleShader.setVec4("DiffuseColor", color);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(c_vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glDisable(GL_BLEND);

        simpleShader.setVec4("DiffuseColor", glm::vec4(0.0, 0.0, 0.0, 1.0f));
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ShadowmapUI::drawShadowmap(Shader &textureArrayShader, float screenWidth, float screenHeight, unsigned int q_vao)
{
    if (!m_drawShadowmap)
        return;

    textureArrayShader.use();

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(textureArrayShader.id, "renderedTexture"), 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    for (int i = 0; i < m_shadowManager->m_splitCount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);

        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);    // Camera position
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Point camera is looking at
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     // Up direction

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        float aspectRatio = (float)screenWidth / screenHeight;
        glm::vec3 scale = glm::vec3(m_quadScale / aspectRatio, m_quadScale, 1.0f);

        float posX = 1.0f - scale.x * ((i + 0.5) * 2);
        float posY = -1.0f + scale.y;
        glm::vec3 position = glm::vec3(posX, posY, 0);

        glm::mat4 projection = glm::ortho(-1, 1, -1, 1, 0, 2);
        projection = glm::scale(glm::translate(projection, position), scale);

        textureArrayShader.setMat4("MVP", projection * view * model);
        textureArrayShader.setInt("layer", i);

        glBindVertexArray(q_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
