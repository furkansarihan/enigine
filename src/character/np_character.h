#ifndef np_character_hpp
#define np_character_hpp

#include "character.h"

class NPCharacter : public Character
{
public:
    NPCharacter(RenderManager *renderManager, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~NPCharacter();

    std::vector<Character *> m_avoidAimList;

    void update(float deltaTime);
    void avoidAim(float deltaTime);
};

#endif /* np_character_hpp */
