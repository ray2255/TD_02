#include <Novice.h>
#include "TrainingDummy.h"

void TrainingDummy::Initialize(Vector2 position)
{
	position_ = position;
	armorHp_ = 50;
	bodyHp_ = 100;
	hitFlashTimer_ = 0;
}

void TrainingDummy::Update()
{
	if (hitFlashTimer_ > 0)
	{
		hitFlashTimer_--;
	}
}

void TrainingDummy::DamageArmor(int damage)
{
	if (armorHp_ <= 0)
	{
		return;
	}

	armorHp_ -= damage;

	if (armorHp_ < 0)
	{
		armorHp_ = 0;
	}

	hitFlashTimer_ = 5;
}

void TrainingDummy::DamageBody(int damage)
{
	// 装甲が残っている時はダメージを受けない
	if (armorHp_ > 0)
	{
		return;
	}

	bodyHp_ -= damage;

	if (bodyHp_ < 0)
	{
		bodyHp_ = 0;
	}

	hitFlashTimer_ = 5;
}

void TrainingDummy::Draw() const
{
	unsigned int color;

	if (hitFlashTimer_ > 0)
	{
		color = WHITE;
	}
	else if (armorHp_ > 0)
	{
		// 装甲あり
		color = 0x808080FF;
	}
	else
	{
		// 装甲破壊後
		color = 0x8B4513FF;
	}

	Novice::DrawBox(
		static_cast<int>(position_.x),
		static_cast<int>(position_.y),
		static_cast<int>(size_.x),
		static_cast<int>(size_.y),
		0.0f,
		color,
		kFillModeSolid
	);

	Novice::ScreenPrintf(
		static_cast<int>(position_.x),
		static_cast<int>(position_.y - 45.0f),
		"Armor : %d",
		armorHp_
	);

	Novice::ScreenPrintf(
		static_cast<int>(position_.x),
		static_cast<int>(position_.y - 25.0f),
		"Body : %d",
		bodyHp_
	);
}