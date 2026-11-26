#include <Novice.h>
#include "Player.h"
#include <cmath>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

namespace {
	constexpr float kHalfPi = 1.57f;
	void DrawPart(
		int texture,
		int width, int height,
		Vector2 pivot, Vector2 anchor,
		float scaleX, float scaleY,
		float angle,
		unsigned int color = WHITE
	)
	{
		const float c = std::cos(angle);
		const float s = std::sin(angle);


		auto toScreen = [&](float x, float y)->Vector2
			{
				x = (x - pivot.x) * scaleX;
				y = (y - pivot.y) * scaleY;
				return{
					anchor.x + x * c - y * s,
					anchor.y + x * s + y * c,
				};
			};
		const float w = static_cast<float>(width);
		const float h = static_cast<float>(height);

		const Vector2 lt = toScreen(0.0f, 0.0f);
		const Vector2 rt = toScreen(w, 0.0f);
		const Vector2 lb = toScreen(0.0f, h);
		const Vector2 rb = toScreen(w, h);

		Novice::DrawQuad(
			static_cast<int>(lt.x), static_cast<int>(lt.y),
			static_cast<int>(rt.x), static_cast<int>(rt.y),
			static_cast<int>(lb.x), static_cast<int>(lb.y),
			static_cast<int>(rb.x), static_cast<int>(rb.y),
			0, 0, width, height, texture, color
		);

	}
}

void Player::Initialize() {
	state_ = PlayerState::Normal;

	position_ = { 200.0f, 500.0f };

	velocity_ = { 0.0f, 0.0f };

	size_ = { 40.0f, 60.0f };

	moveSpeed_ = 5.0f;

	facing_ = 1;

	hp_ = maxHp_;

	isGrounded_ = false;

	idleBreathFrame_ = 0;

	attackTimer_ = 0;

	isHammerActive_ = false;

	wasLeftMouseDown_ = false;

	isAiming_ = false;

	recoilTimer_ = 0;

	hurtTime_ = 0;

	dodgeFrame_ = 0;

	perfectDodgeTriggered_ = false;

	perfectDodgeCount_ = 0;

	dodgeDirection_ = 1;

	perfectDodgePending_ = false;

	for (int i = 0; i < kMaxBullets_; i++)
	{
		bullets_[i] = {};
	}

	airVelocity_ = 0.0f;

	hasHammerHit_ = false;

	hammerHitStopTimer_ = 0;

}
void Player::LoadTextures()
{
	bodyTexture_ = Novice::LoadTexture("./Resources/Player/actionPlayer0.png");
	feetTexture_ = Novice::LoadTexture("./Resources/Player/foot2.png");
	gunTexture_ = Novice::LoadTexture("./Resources/Player/actionGun.png");
	hammerTexture_ = Novice::LoadTexture("./Resources/Player/actionHummer.png");
	bulletTexture_ = Novice::LoadTexture("./Resources/Player/actionBullet.png");
}


void Player::Update(const char* keys, const char* preKeys, float worldSpeed, bool updateWorld)
{
	(void)preKeys;
	if (invincibleTimer_ > 0)
	{
		invincibleTimer_--;
	}

	//マウス入力
	Novice::GetMousePosition(&mouseX_, &mouseY_);
	bool isLeftMouseDown = Novice::IsPressMouse(0) != 0;
	bool isRightMouseDown = Novice::IsPressMouse(1) != 0;

	// 左ボタンを押した瞬間
	bool isLeftMouseTriggered = isLeftMouseDown && !wasLeftMouseDown_;
	wasLeftMouseDown_ = isLeftMouseDown;

#ifdef USE_IMGUI
	// ImGui操作中はゲーム側のマウス攻撃を無効にする
	if (ImGui::GetIO().WantCaptureMouse)
	{
		isLeftMouseTriggered = false;
		isRightMouseDown = false;
	}
#endif
	// 弾の更新
	for (int i = 0; i < kMaxBullets_; i++)
	{
		if (!bullets_[i].isActive)
		{
			continue;
		}
		bullets_[i].position.x += bullets_[i].velocity.x*worldSpeed;
		bullets_[i].position.y += bullets_[i].velocity.y*worldSpeed;
		if (updateWorld)
		{
			bullets_[i].life--;
			if (bullets_[i].life <= 0) 
			{
				bullets_[i].isActive = false;
			}
		}
		

	}


	// 通常状態で右ボタンを押している間だけ照準
	isAiming_ = (state_ == PlayerState::Normal && isRightMouseDown);

	if (state_ == PlayerState::Normal)
	{
		// プレイヤーの中心とマウス位置を比較する
		float playerCenterX = position_.x + size_.x * 0.5f;
		if (mouseX_ > playerCenterX)
		{
			facing_ = 1;
		}
		else if (mouseX_ < playerCenterX)
		{
			facing_ = -1;
		}

		velocity_.x = 0.0f;
		// 移動入力（向きとは別に扱う）
		float moveInput = 0.0f;

		if (keys[DIK_A])
		{
			moveInput -= 1.0f;
		}

		if (keys[DIK_D])
		{
			moveInput += 1.0f;
		}

		if (isGrounded_)
		{
			// 地上ではキーを離すと停止
			velocity_.x = moveInput * moveSpeed_;
		}
		else
		{
			// 空中では入力があれば移動、なければ慣性を維持
			if (moveInput != 0.0f)
			{
				velocity_.x = moveInput * moveSpeed_;
			}
			else
			{
				velocity_.x = airVelocity_ * 0.99f;
			}
		}
		//jump
		if (isGrounded_ && keys[DIK_SPACE] && !preKeys[DIK_SPACE])
		{
			velocity_.y = -jumpPower_;
			isGrounded_ = false;
		}
		if (keys[DIK_LSHIFT] && !preKeys[DIK_LSHIFT])
		{
			state_ = PlayerState::Dodge;
			dodgeFrame_ = 0;
			perfectDodgeTriggered_ = false;
			isAiming_ = false;

			if (moveInput < 0.0f)
			{
				dodgeDirection_ = -1;
			}
			else if (moveInput > 0.0f)
			{
				dodgeDirection_ = 1;
			}
			else
			{
				dodgeDirection_ = facing_;
			}
			velocity_.x = static_cast<float>(dodgeDirection_ * kDodgeSpeed_);
		}
		else if (isLeftMouseTriggered)
		{
			if (isAiming_)
			{
				// マグナム発射
				// 反動で射撃方向と反対に吹き飛ぶ
				const Vector2 direction = GetGunDirection();
				const Vector2 muzzle = GetMuzzlePosition();
				if (hp_ > kMagnumHpCost_)
				{
					for (int i = 0; i < kMaxBullets_; i++)
					{
						if (bullets_[i].isActive)
						{
							continue;
						}
						bullets_[i].position = muzzle;

						bullets_[i].velocity = {
							direction.x * bulletSpeed_,
							direction.y * bulletSpeed_
						};
						bullets_[i].life = bulletLife_;
						bullets_[i].isActive = true;

						// 発射時にHPを消費する
						hp_ -= kMagnumHpCost_;

						velocity_.x = -direction.x * recoilPowerX_;
						velocity_.y = -direction.y * recoilPowerX_;
						isGrounded_ = false;
						recoilTimer_ = recoilDuration_;
						isAiming_ = false;
						state_ = PlayerState::GunRecoil;
						break;
					}
				}
			}
			else
			{
				state_ = PlayerState::HammerCharge;
				hammerTurns_ = 0;
				hammerTurnAngle_ = 0.0f;
				prevHammerMouseX_ = mouseX_;
				prevHammerMouseY_ = mouseY_;

				hammerAngle_ = (facing_ > 0) ? 0.0f : 3.14f;
				attackTimer_ = 0;
				hammerHitStopTimer_ = 0;
				isHammerActive_ = false;
				hasHammerHit_ = false;

				if (isGrounded_)
				{
					velocity_.x *= 0.35f;
				}
			}
		}
	}
	else if (state_==PlayerState::Dodge)
	{
		dodgeFrame_++;
		if (dodgeFrame_>=kDodgeFrame_)
		{
			velocity_.x = 0.0f;
			state_ = PlayerState::Normal;
		}
		else
		{
			velocity_.x = static_cast<float>(dodgeDirection_) * kDodgeSpeed_;
		}
	}

	else if (state_ == PlayerState::HammerCharge)
	{
		float moveInput = 0.0f;
		if (keys[DIK_A]) moveInput -= 1.0f;
		if (keys[DIK_D]) moveInput += 1.0f;

		if (isGrounded_ || moveInput != 0.0f)
		{
			velocity_.x = moveInput * moveSpeed_ * 0.35f;
		}
		else
		{
			velocity_.x *= 0.99f;
		}
	}

	else if (state_ == PlayerState::HammerAttack)
	{
		if (isGrounded_)
		{
			velocity_.x = 0.0f;
		}
		else
		{
			velocity_.x *= 0.99f;
		}

		if (hammerHitStopTimer_ > 0)
		{
			hammerHitStopTimer_--;  // 命中時は振りだけ止める
		}
		else
		{
			attackTimer_++;

			const float swingSide = static_cast<float>(facing_);

			if (attackTimer_ <= 6)
			{
				// 振りかぶる
				const float t = static_cast<float>(attackTimer_) / 6.0f;
				hammerAngle_ = releaseAngle_ - swingSide * 1.1f * t;
			}
			else if (attackTimer_ <= 18)
			{
				// 7～18フレームで振り抜く。12フレーム目に狙った方向を通る
				const float t = static_cast<float>(attackTimer_ - 6) / 12.0f;
				hammerAngle_ = releaseAngle_ + swingSide * (-1.1f + 2.2f * t);
			}

			isHammerActive_ = (attackTimer_ >= 7 && attackTimer_ <= 18);

			if (attackTimer_ >= 26)
			{
				state_ = PlayerState::Normal;
				isHammerActive_ = false;
				attackTimer_ = 0;
			}
		}
	}
	else if (state_ == PlayerState::GunRecoil)
	{
		// 反動時間を減らす
		recoilTimer_--;
		velocity_.x *= 0.9f;

		// 反動終了
		if (recoilTimer_ <= 0)
		{
			velocity_.x = 0.0f;
			state_ = PlayerState::Normal;
		}
	}
	else if (state_ == PlayerState::Hurt)
	{
		velocity_.x *= 0.9f;
		hurtTime_--;
		if (hurtTime_ <= 0)
		{
			state_ = PlayerState::Normal;
		}
	}

	else if (state_ == PlayerState::Dead)
	{
		return;
	}


	velocity_.y += gravity_;// 重力を加える

	// 座標を更新する
	position_.x += velocity_.x;
	position_.y += velocity_.y;

	// 地面との当たり判定
	if (position_.y + size_.y >= groundY_)
	{
		// 足元を地面に合わせる
		position_.y = groundY_ - size_.y;
		// 落下を止める
		velocity_.y = 0.0f;
		isGrounded_ = true;
	}
	else
	{
		isGrounded_ = false;
	}

	// ハンマーの位置

	if (state_ == PlayerState::HammerCharge)
	{
		const Vector2 pivot = GetHammerShoulder();
		const float cx = pivot.x;
		const float cy = pivot.y;

		const float dx = static_cast<float>(mouseX_) - cx;
		const float dy = static_cast<float>(mouseY_) - cy;
		const float prevDx = static_cast<float>(prevHammerMouseX_) - cx;
		const float prevDy = static_cast<float>(prevHammerMouseY_) - cy;

		// 中心付近では角度が急変するため、回転を数えない
		constexpr float kDeadZone = 16.0f;
		const bool valid = dx * dx + dy * dy >= kDeadZone * kDeadZone;
		const bool prevValid = prevDx * prevDx + prevDy * prevDy >= kDeadZone * kDeadZone;

		if (valid)
		{
			hammerAngle_ = std::atan2(dy, dx);

			if (dx > 0.0f) facing_ = 1;
			if (dx < 0.0f) facing_ = -1;

			if (prevValid && hammerTurns_ < kMaxHammerTurns_)
			{
				const float prevAngle = std::atan2(prevDy, prevDx);
				const float delta =
					std::remainder(hammerAngle_ - prevAngle, kHammerTwoPi_);

				// 中心を横切るような急な反転は除外する
				if (std::fabs(delta) < kHammerTwoPi_ * 0.25f)
				{
					hammerTurnAngle_ += delta;

					if (std::fabs(hammerTurnAngle_) >= kHammerTwoPi_ - 0.0001f)
					{
						hammerTurns_++;
						hammerTurnAngle_ -=
							std::copysign(kHammerTwoPi_, hammerTurnAngle_);
					}
				}
			}
		}

		prevHammerMouseX_ = mouseX_;
		prevHammerMouseY_ = mouseY_;

		if (!isLeftMouseDown)
		{
			releaseAngle_ = hammerAngle_;
			attackTimer_ = 0;
			state_ = PlayerState::HammerAttack;
		}
	}

	airVelocity_ = velocity_.x;


	const bool isIdle =
		state_ == PlayerState::Normal &&
		isGrounded_ &&
		!isAiming_ &&
		std::fabs(velocity_.x) < 0.01f;

	if (isIdle)
	{
		idleBreathFrame_ = (idleBreathFrame_ + 1) % 150;
	}
	else
	{
		idleBreathFrame_ = 0;
	}

}

void Player::Damage(int damage, float knockbackx, float knockbacky) 
{
	if (state_==PlayerState::Dodge)
	{
		if (damage > 0 && invincibleTimer_ == 00 && dodgeFrame_ < kPerfectDodgeFrames_ && !perfectDodgeTriggered_)
		{
			perfectDodgeTriggered_ = true;
			perfectDodgeCount_++;
			perfectDodgePending_ = true;
		}
		return;
	}
	if (damage <= 0 || invincibleTimer_ > 0 || state_ == PlayerState::Dead)
	{
		return;
	}
	isHammerActive_ = false;
	isAiming_ = false;
	hammerHitStopTimer_ = 0;

	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		velocity_ = { 0.0f,0.0f };
		state_ = PlayerState::Dead;
		return;
	}
	velocity_ = { knockbackx,knockbacky };
	isGrounded_ = false;
	hurtTime_ = hurtDuration_;
	invincibleTimer_ = invincibleDuration_;
	state_ = PlayerState::Hurt;


}
bool Player::ConsumePerfectDodge() {
	const bool triggereed = perfectDodgePending_;
	perfectDodgePending_ = false;
	return triggereed;
}

void Player::HealFromHammerHit() {
	if (state_ == PlayerState::Dead)
	{
		return;
	}

	const float healAmount = kHammerBaseHeal_ + kHammerHealPerTurn_ * static_cast<float>(hammerTurns_);
	hp_ = std::fmin(hp_ + healAmount, maxHp_);
}

Vector2 Player::GetHammerHeadPoint(int index) const
{
	const float dirX = std::cos(hammerAngle_);
	const float dirY = std::sin(hammerAngle_);

	const Vector2 pivot = GetHammerShoulder();
	const float handX = pivot.x;
	const float handY = pivot.y;

	// ヘッドは柄に対して直角に配置する
	const float offset = static_cast<float>(index) * kHeadSpacing_;
	return {
		handX + dirX * kHammerReach_ - dirY * offset,
		handY + dirY * kHammerReach_ + dirX * offset
	};
}
//ハンマー　Hit
bool Player::CheckHammerHit(Vector2 targetPos, Vector2 targetSize)
{
	if (!isHammerActive_ || hasHammerHit_)
	{
		return false;
	}

	for (int i = -1; i <= 1; i++)
	{
		const Vector2 p = GetHammerHeadPoint(i);

		// 円の中心に最も近い、木人の矩形上または内部の点
		const float nearX = std::fmax(targetPos.x, std::fmin(p.x, targetPos.x + targetSize.x));
		const float nearY = std::fmax(targetPos.y, std::fmin(p.y, targetPos.y + targetSize.y));

		const float dx = p.x - nearX;
		const float dy = p.y - nearY;

		if (dx * dx + dy * dy <= kHeadRadius_ * kHeadRadius_)
		{
			hasHammerHit_ = true;
			hammerHitStopTimer_ = hammerHitStopFrames_;
			return true;
		}
	}

	return false;
}
//Bullet Hit
bool Player::CheckBulletHit(Vector2 targetPos, Vector2 targetSize) {
	for (int i = 0; i < kMaxBullets_; i++)
	{
		if (!bullets_[i].isActive)
		{
			continue;
		}
		float bulletLeft = bullets_[i].position.x - 4.0f;
		float bulletRight = bullets_[i].position.x + 4.0f;
		float bulletTop = bullets_[i].position.y - 4.0f;
		float bulletBottom = bullets_[i].position.y + 4.0f;

		bool isHit =
			bulletLeft<targetPos.x + targetSize.x &&
			bulletRight>targetPos.x &&
			bulletTop<targetPos.y + targetSize.y &&
			bulletBottom>targetPos.y;
		if (isHit)
		{
			bullets_[i].isActive = false;
			return true;
		}

	}
	return false;
}

Vector2 Player::GetGunShoulder()const {
	const Vector2 center = GetCenter();
	return{
		center.x + 10.0f * static_cast<float>(facing_),
		position_.y + size_.y - 78.0f

	};
}
Vector2 Player::GetHammerShoulder() const {
	const float centerX = GetCenter().x;
	const float shoulderY = position_.y + size_.y - 78.0f;

	if (state_ == PlayerState::HammerCharge ||
		state_ == PlayerState::HammerAttack)
	{
		return { centerX, shoulderY };
	}

	return {
		centerX - 10.0f * static_cast<float>(facing_),
		shoulderY
	};
}
Vector2 Player::GetGunDirection() const
{
	const Vector2 shoulder = GetGunShoulder();
	const float dx = static_cast<float>(mouseX_) - shoulder.x;
	const float dy = static_cast<float>(mouseY_) - shoulder.y;
	const float length = std::sqrt(dx * dx + dy * dy);

	if (length < 0.01f)
	{
		return { static_cast<float>(facing_), 0.0f };
	}

	return { dx / length, dy / length };
}

Vector2 Player::GetMuzzlePosition() const
{
	const Vector2 shoulder = GetGunShoulder();
	const Vector2 dir = GetGunDirection();

	// 画像上の肩から銃口まで約210px、描画倍率は0.22
	constexpr float kGunLength = 210.0f * 0.22f;
	return {
		shoulder.x + dir.x * kGunLength,
		shoulder.y + dir.y * kGunLength
	};
}

void Player::Draw()const {
	constexpr float kBodyScale = 0.5f;//Player大きさ
	const float face = static_cast<float>(facing_);

	// 靴底から足の画像1枚分だけ上がった位置が腰
	const Vector2 waist = {
		position_.x + size_.x * 0.5f,
		position_.y + size_.y - 90.0f * kBodyScale
	};

	// 足：画像の上端を腰に合わせる → 靴は腰より下
	DrawPart(feetTexture_, 90, 90,
		{ 45.0f, 0.0f }, waist,
		face * kBodyScale, kBodyScale, 0.0f);

	// 上半身：服の裾を腰に合わせる → 頭は腰より上
	const float phase = static_cast<float>(idleBreathFrame_) * 6.2831853f / 150.0f;
	const float breath = (1.0f - std::cos(phase)) * 0.5f;
	const float bodyStretch = 1.0f + breath * 0.025f;

	DrawPart(bodyTexture_, 250, 250,
		{ 125.0f, 174.0f }, waist,
		face * kBodyScale, kBodyScale * bodyStretch, 0.0f);

	// 左手：通常は下向き、照準中はマウスの方向へ
	Vector2 gunShoulder = GetGunShoulder();
	gunShoulder.y += (gunShoulder.y - waist.y) * (bodyStretch - 1.0f);
	float gunAngle = 0.0f;

	if (isAiming_)
	{
		const Vector2 dir = GetGunDirection();
		gunAngle = std::atan2(dir.y, dir.x) - kHalfPi;
	}

	DrawPart(gunTexture_, 150, 250,
		{ 75.0f, 20.0f }, gunShoulder,
		face * 0.22f, 0.22f, gunAngle);

	// 右手：通常は下向き、溜め・攻撃中は実際の判定位置に合わせる
	Vector2 hammerShoulder = GetHammerShoulder();
	hammerShoulder.y += (hammerShoulder.y - waist.y) * (bodyStretch - 1.0f);
	const bool usingHammer =
		state_ == PlayerState::HammerCharge ||
		state_ == PlayerState::HammerAttack;

	float hammerDrawAngle = 0.0f;
	float hammerScaleY = 0.22f;

	if (usingHammer)
	{
		const Vector2 head = GetHammerHeadPoint(0);
		const float dx = head.x - hammerShoulder.x;
		const float dy = head.y - hammerShoulder.y;

		hammerDrawAngle = std::atan2(dy, dx) - kHalfPi;

		// 素材の肩支点 y=20 からヘッド中心 y≈215 まで
		hammerScaleY = std::sqrt(dx * dx + dy * dy) / 195.0f;
	}


	//ハンマーを光に付ける
	const bool isFullCharge = usingHammer && hammerTurns_ >= kMaxHammerTurns_;
	const Vector2 head = GetHammerHeadPoint(0);
	const int headX = static_cast<int>(head.x);
	const int headY = static_cast<int>(head.y);

	
	if (isFullCharge)
	{
		Novice::DrawEllipse(headX, headY, 29, 29, 0.0f, 0xFFB82028, kFillModeSolid);
		Novice::DrawEllipse(headX, headY, 22, 22, 0.0f, 0xFFD04050, kFillModeSolid);
	}

	DrawPart(hammerTexture_, 150, 250,
		{ 75.0f, 20.0f }, hammerShoulder,
		face * (usingHammer ? 0.40f : 0.22f),
		hammerScaleY, hammerDrawAngle, WHITE);


	if (isFullCharge)
	{
		Novice::DrawEllipse(headX, headY, 11, 11, 0.0f, 0xFFCC4055, kFillModeSolid);
		Novice::DrawEllipse(headX, headY, 3, 3, 0.0f, 0xFFF4B0DD, kFillModeSolid);
	}

	// 照準中だけレーザーを表示する
	if (isAiming_)
	{
		const Vector2 muzzle = GetMuzzlePosition();
		const Vector2 direction = GetGunDirection();
		constexpr float kLaserLength = 2500.0f;

		Novice::DrawLine(
			static_cast<int>(muzzle.x),
			static_cast<int>(muzzle.y),
			static_cast<int>(muzzle.x + direction.x * kLaserLength),
			static_cast<int>(muzzle.y + direction.y * kLaserLength),
			0xFF000080
		);
	}



	//if (state_ == PlayerState::HammerCharge || state_ == PlayerState::HammerAttack)
	//{
	//	const int handX = static_cast<int>(position_.x + size_.x * 0.5f);
	//	const int handY = static_cast<int>(position_.y + size_.y * 0.5f);
	//	const Vector2 head = GetHammerHeadPoint(0);


	//	for (int i = 0; i <= 3; i++)
	//	{
	//		Novice::DrawLine(
	//			handX + i, handY,
	//			static_cast<int>(head.x) + i,
	//			static_cast<int>(head.y),
	//			0xAF7848FF
	//		);
	//	}
	//	// 金属のヘッド。最大まで溜めると金色になる
	//	const unsigned int color = hammerTurns_ >= kMaxHammerTurns_ ? 0xFFD050FF : 0xAAB8C8FF;

	//	for (int i = -1; i <= 1; i++)
	//	{
	//		const Vector2 p = GetHammerHeadPoint(i);
	//		Novice::DrawEllipse(
	//			static_cast<int>(p.x), static_cast<int>(p.y),
	//			static_cast<int>(kHeadRadius_), static_cast<int>(kHeadRadius_),
	//			0.0f, color, kFillModeSolid);
	//	}

	//}

	for (int i = 0; i < kMaxBullets_; i++)
	{
		if (!bullets_[i].isActive)
		{
			continue;
		}
		const Vector2 v = bullets_[i].velocity;
		const float bulletAngle = std::atan2(v.y, v.x);

		//Bulletの大きさ
		DrawPart(bulletTexture_, 50, 50,
			{ 25.0f, 25.0f }, bullets_[i].position,
			0.50f, 0.50f, bulletAngle);
	}


}








//DeBug用
void Player::DebugGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("Player Debug");

	// 現在の情報
	ImGui::Text("Position: %.1f, %.1f", position_.x, position_.y);
	ImGui::Text("Velocity: %.1f, %.1f", velocity_.x, velocity_.y);
	ImGui::Text("HP: %.0f / %.0f", hp_, maxHp_);
	ImGui::Text("Facing: %s", facing_ == 1 ? "Right" : "Left");
	ImGui::Text("Grounded: %s", isGrounded_ ? "Yes" : "No");
	ImGui::Text("Aiming: %s", isAiming_ ? "Yes" : "No");
	ImGui::Text("Dodge Frame: %d", dodgeFrame_);
	ImGui::Text("Perfect Dodge Count: %d", perfectDodgeCount_);

	const char* stateName = "Unknown";

	switch (state_)
	{
	case PlayerState::Normal:
		stateName = "Normal";
		break;
	case PlayerState::HammerCharge:
		stateName = "hammerCharge";
		break;
	case PlayerState::HammerAttack:
		stateName = "HammerAttack";
		break;
	case PlayerState::GunRecoil:
		stateName = "GunRecoil";
		break;
	case PlayerState::Hurt:
		stateName = "Hurt";
		break;
	case PlayerState::Dodge:
		stateName = "Dodge";
		break;
	case PlayerState::Dead:
		stateName = "Dead";
		break;
	}

	ImGui::Text("State: %s", stateName);
	ImGui::Text("Attack Timer: %d", attackTimer_);
	ImGui::Text("Hammer Active: %s", isHammerActive_ ? "Yes" : "No");
	ImGui::Text("Recoil Timer: %d", recoilTimer_);

	ImGui::Separator();

	// 調整用パラメータ
	ImGui::SliderFloat("Move Speed", &moveSpeed_, 0.0f, 15.0f);
	ImGui::SliderFloat("Jump Power", &jumpPower_, 1.0f, 25.0f);
	ImGui::SliderFloat("Gravity", &gravity_, 0.1f, 2.0f);

	ImGui::SliderFloat("Recoil Power", &recoilPowerX_, 0.0f, 30.0f);
	ImGui::SliderFloat("Recoil Y", &recoilPowerY_, 0.0f, 15.0f);
	ImGui::SliderInt("Recoil Frames", &recoilDuration_, 1, 60);

	ImGui::SliderFloat("Bullet Speed", &bulletSpeed_, 1.0f, 50.0f);
	ImGui::SliderInt("Bullet Life", &bulletLife_, 1, 500);

	ImGui::Text("Hurt: %d  Invincible: %d", hurtTime_, invincibleTimer_);
	if (ImGui::Button("Test Damage"))
	{
		Damage(10, -6.0f, -4.0f);
	}

	if (ImGui::Button("Reset Player"))
	{
		Initialize();  // 初期位置・HP・状態をリセット
	}
	ImGui::Text("Hammer turns: %d / %d", hammerTurns_, kMaxHammerTurns_);

	ImGui::End();
#endif
}





