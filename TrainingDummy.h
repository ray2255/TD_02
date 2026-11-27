#pragma once
#include "Vector2.h"

class TrainingDummy
{
public:
	void Initialize(Vector2 position);
	void Update();
	void Draw() const;
	
	void DamageArmor(int damage);
	bool DamageBody(int damage);



	Vector2 GetPosition() const
	{
		return position_;
	}

	Vector2 GetSize() const
	{
		return size_;
	}

	bool IsArmorBreak()	const {
		return armorHp_ <= 0;
	}

	bool IsDefeated()const {
		return bodyHp_ <= 0;
	}

private:
	Vector2 position_{};
	Vector2 size_{ 80.0f, 140.0f };

	int armorHp_ = 50;
	int bodyHp_ = 100;

	int hitFlashTimer_ = 0;
};