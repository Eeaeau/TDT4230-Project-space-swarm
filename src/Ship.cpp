#include "Ship.hpp"


Ship::Ship(float gunCooldownTime , int health) : Actor(true, health) { 
	this->gunCooldownTime = gunCooldownTime;
}
void Ship::fireGun(float dt)
{
	if (!gunActive) {
		gunActive == true;
		gunCooldown = gunCooldownTime;
	}
	gunCooldown -= dt;
	
}

;