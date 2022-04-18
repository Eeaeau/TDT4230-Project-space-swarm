#pragma once

#include <Actor.hpp>

class Ship :
    public Actor
{
public:
    Ship(float gunCooldownTime = 2, int health = 10);
    void fireGun(float dt);
private:
    float gunCooldownTime;
    float gunCooldown = 0;
    bool gunActive = false;
    Actor target;
};

