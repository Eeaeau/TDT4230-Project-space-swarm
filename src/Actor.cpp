#include "Actor.hpp"

Actor::Actor(bool damageable, int health) {
	this->damageable= damageable;
	this->health = health;
}

void Actor::takeDamage(int damange)
{
	this->health -= damange;
	if (this->health < 0) {
		this->destroyed = true;
	}
}

bool Actor::isDestroyed()
{
	return destroyed;
}
