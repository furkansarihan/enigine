#ifndef update_manager_hpp
#define update_manager_hpp

#include <iostream>
#include <vector>

class Updatable
{
public:
    virtual ~Updatable() {}

    virtual void update(float deltaTime) = 0;
};

class UpdateManager
{
public:
    UpdateManager();
    ~UpdateManager();

    // TODO: private?
    void update(float deltaTime);
    void add(Updatable *updatable);
    void remove(Updatable *updatable);

private:
    std::vector<Updatable *> m_updatables;
};

#endif /* update_manager_hpp */
