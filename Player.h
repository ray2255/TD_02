#pragma once
#include "Vector2.h"
enum class PlayerState
{
	Normal,
	HammerCharge,
	HammerAttack,
	GunRecoil,
	Dodge,
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
	void Update(const char* keys, const char* prevKeys,float worldSpeed,bool updateWorld);

	// 描画
	void Draw()const;

	Vector2 GetCenter()const{
		return{
		position_.x + size_.x * 0.5f,
		position_.y + size_.y * 0.5f
		};
	}

	bool IsHammerActive() const { return isHammerActive_; }

	Vector2 GetHammerPosition() const { return HammerPostion_; }

	Vector2 GetHammerSize() const { return HammerSize_; }

	void DebugGui();

	void Damage(int damage, float knockbackx, float knockbacky);

	void HealFromHammerHit();

	bool CheckHammerHit(Vector2 targetPos, Vector2 targetSize);

	int GetHammerDamage(int baseDamage) const
	{
		return baseDamage + (baseDamage * hammerTurns_ + 1) / 2;
	}

	bool CheckBulletHit(Vector2 targePos, Vector2 targetSize);

	bool ConsumePerfectDodge();

	void LoadTextures();

	bool IsDead()const {
		return state_ == PlayerState::Dead;
	}

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

	//ハンマー㏋吸収
	static constexpr float kMagnumHpCost_ = 5.0f;
	static constexpr float kHammerBaseHeal_ = 3.0f;
	static constexpr float kHammerHealPerTurn_ = 2.0f;
	static constexpr int kHealEffectFrames_ = 36;
	int healEffectTime_ = 0;
	

	bool isGrounded_ = false;//地面に接地しているかどうか	

	int idleBreathFrame_ = 0;//呼吸待機動作

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
	int hurtTime_ = 0;// 操作できるまでの残り時間
	int hurtDuration_ = 12;// 被弾時の硬直時間


	int dodgeFrame_ = 0;// 回避開始からの経過フレーム
	int dodgeDirection_ = 1;// 回避方向（左 -1、右 1）
	static constexpr int kDodgeFrame_ = 20;// 回避の長さ
	static constexpr int kPerfectDodgeFrames_ = 8;// ジャスト回避の受付時間
	bool perfectDodgeTriggered_ = false;// 今回の回避で成功したか
	bool perfectDodgePending_ = false; // mainにまだ伝えていない成功通知
	int perfectDodgeCount_ = 0;// 確認用の成功回数
	static constexpr float kDodgeSpeed_ = 9.0f;// 1フレームの横移動量


	
	float recoilPowerX_ = 15.0f;// 射撃反動X
	float recoilPowerY_ = 5.0f;// 射撃反動Y
	int recoilDuration_ = 20;// 射撃反動強さ


	Vector2 HammerPostion_ = {};
	Vector2 HammerSize_ = { 70.0f, 60.0f };//ハンマーの幅と高さ


	static constexpr int kMaxBullets_ = 10;//最大弾丸

	PlayerBullet bullets_[kMaxBullets_]{};

	float bulletSpeed_ = 30.0f;//弾丸速度
	int bulletLife_ = 200;//弾丸の存在時間

	float airVelocity_ = 0.0f;// 空中の横移動速度

	bool hasHammerHit_ = false;// 1回の攻撃ですでに命中したか


	int hammerHitStopTimer_ = 0;
	int hammerHitStopFrames_ = 5;

	static constexpr float kHammerTwoPi_ = 6.28f;
	static constexpr int kMaxHammerTurns_ = 3;

	static constexpr float kHammerReach_ = 72.0f;// 手元からヘッドまで
	static constexpr float kHeadRadius_ = 16.0f;//ハンマーの頭の大きさ
	static constexpr float kHeadSpacing_ = 16.0f;//柄の長さ

	int hammerTurns_ = 0;
	float hammerTurnAngle_ = 0.0f;
	int prevHammerMouseX_ = 0;
	int prevHammerMouseY_ = 0;
	float hammerAngle_ = 0.0f;
	float releaseAngle_ = 0.0f;

	Vector2 GetHammerHeadPoint(int index)const;// ハンマーヘッドを構成する3つの円の中心


	int invincibleTimer_ = 0;//無敵状態
	int invincibleDuration_ = 60;


	int bodyTexture_ = -1;
	int feetTexture_ = -1;
	int gunTexture_ = -1;
	int hammerTexture_ = -1;
	int bulletTexture_ = -1;

	Vector2 GetGunShoulder()const;
	Vector2 GetHammerShoulder()const;
	Vector2 GetGunDirection()const;
	Vector2 GetMuzzlePosition()const;

};