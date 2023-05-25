#ifndef np_character_hpp
#define np_character_hpp

#include "character.h"

class NPCharacter : public Character
{
public:
    NPCharacter(ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~NPCharacter();

    std::vector<Character *> m_avoidAimList;

    void update(GLFWwindow *window, float deltaTime);
    void avoidAim(GLFWwindow *window, float deltaTime);
};

#endif /* np_character_hpp */
