#pragma once
#include "Vector2.h"
#include "Cooldown.h"

enum class EnemyAtteck
{
	Normal,
	Rush,
	Tellslam,
	ForelegsSlam,
	bite
};

enum class EnemyState {
	Idle,
	Chase,
	Attack
};


struct BossParts
{
	int armorTexture; // 防護のテクスチャ
	int bodyTexture;  // 生身のテクスチャ

	bool isAlive;       // 部位が生きているか
	bool isArmorNotBroken; // 装甲が壊れていないか

	Vector2 position;      // 	最終的な描画位置
	Vector2 localPosition; // 親からの相対位置
	Vector2 baseLocalPosition; //基準となる位置(動きの最初の位置の基準)

	float angle;           // 最終的な描画角度（ワールド角度）
	float localAngle;      // 親からの相対角度（関節を曲げる角度）
	float length;


	int parentIndex;

	//ボスの状態
	int hp ;
	int maxHp;
	int armorHp ;
	int maxArmorHp ;
	bool isWeakPoint ; // 頭などの急所フラグ（必要なら）

};
const int kbossparts = 16;//ボスの部位
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

	void Attack(EnemyAtteck attack);

private:
	Vector2 worldPosition_ = { 0.0f, 0.0f };//ワールド座標

	Vector2 velocity_ = { 0.0f, 0.0f };//速度

	Vector2 localPosition_ = { 0.0f, 0.0f };//親からの相対座標

	float localAngle = 0.0f; // 親からの相対角度

	BossParts parts_[kbossparts] = {};

	float animTimer_ = 0.0f;

	// 行動状態
	EnemyState state_ = EnemyState::Idle;

	// 攻撃種類
	EnemyAtteck currentAttack_ = EnemyAtteck::Normal;

	// 攻撃クールダウン
	Cooldown attackCooldown_{ 120 };

	// 攻撃アニメーションタイマー
	float attackTimer_ = 0.0f;

	float speed = 2.5f;
};

