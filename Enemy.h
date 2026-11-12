#pragma once
#include "Vector2.h"
enum class EnemyAtteck
{
	Normal,
	Rush,
	Tellslam,
	ForelegsSlam,
	bite
};


class Enemy
{
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw()const;
	//移動
	void Move(const Vector2& delta);

	bool Attack(EnemyAtteck attack);

private:
	Vector2 position_ = { 0.0f, 0.0f };//位置

	Vector2 velocity_ = { 0.0f, 0.0f };//速度

	int bossparts_ = 16;//ボスの部位

	bool partsAlive_[16] = { true };//ボスの部位が生きているかどうか	
	
	bool partsNotBreak_[16] = { true };//ボスの装備がこわれていないかどうか



};

