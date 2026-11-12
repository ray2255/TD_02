#pragma once
#include "Vector2.h"
enum class PlayerState
{
	Normal,
	HammerAttack,
	GunRecoil,
	Hurt,
	Dead
};

class Player
{
public:
	// 初期化
	void Initialize();

	// 更新
	void Update(const char* keys, const char* prevKeys);

	// 描画
	void Draw()const;

private:
	PlayerState state_ = PlayerState::Normal;//初期状態

	Vector2 position_ = { 0.0f, 0.0f };//位置

	Vector2 velocity_ = { 0.0f, 0.0f };//速度

	Vector2 size_ = { 40.0f, 60.0f };// キャラクターの幅と高さ

	float moveSpeed_ = 2.0f;//移動速度

	int facing_ = 1;//向き 1:右 -1:左

	//体力
	float hp_ = 100.0f;
	float maxHp_ = 100.0f;

	bool isGrounded_ = false;//地面に接地しているかどうか	

};

