class Actor {
public:
	Actor(bool damageable = false, int health = 10);
	void takeDamage(int damange = 1);
	bool isDestroyed();
private:
	bool damageable;
	int health;
	bool destroyed = false;
};