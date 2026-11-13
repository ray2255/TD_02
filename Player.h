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

struct PlayerBullet
{
	Vector2 position{};
	Vector2 velocity{};
	int life = 0;
	bool isActive = false;
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

	bool IsHammerActive() const { return isHammerActive_; }

	Vector2 GetHammerPosition() const { return HammerPostion_; }

	Vector2 GetHammerSize() const { return HammerSize_; }

	void DebugGui();
		

private:
	PlayerState state_ = PlayerState::Normal;//初期状態

	Vector2 position_ = { 0.0f, 0.0f };//位置

	Vector2 velocity_ = { 0.0f, 0.0f };//速度

	Vector2 size_ = { 40.0f, 60.0f };// キャラクターの幅と高さ

	float moveSpeed_ = 5.0f;//移動速度

	int facing_ = 1;//向き 1:右 -1:左

	//体力
	float hp_ = 100.0f;
	float maxHp_ = 100.0f;

	bool isGrounded_ = false;//地面に接地しているかどうか	

	float gravity_ = 0.5f;//重力加速度

	float jumpPower_ = 10.0f;//ジャンプ力

	float groundY_ = 600.0f;//地面のY座標(仮)

	int attackTimer_ = 0;// ハンマー攻撃の経過フレーム

	bool isHammerActive_ = false;// ハンマーの攻撃判定が有効か

	int mouseX_ = 0;//マウスのX座標
	int mouseY_ = 0;//マウスのY座標

	bool wasLeftMouseDown_ = false;// 左クリックの前フレームの状態

	bool isAiming_ = false;//照準中かどうか

	int recoilTimer_ = 0;//反動の経過フレーム

	
	float recoilPowerX_ = 12.0f;// 射撃反動X
	float recoilPowerY_ = 5.0f;// 射撃反動Y
	int recoilDuration_ = 20;// 射撃反動強さ


	Vector2 HammerPostion_ = {};
	Vector2 HammerSize_ = { 70.0f, 60.0f };//ハンマーの幅と高さ


	static constexpr int kMaxBullets_ = 10;//最大弾丸

	PlayerBullet bullets_[kMaxBullets_]{};

	float bulletSpeed_ = 30.0f;//弾丸速度
	int bulletLife_ = 200;//弾丸の存在時間

	float airVelocity_ = 0.0f;//// 空中の横移動速度



};