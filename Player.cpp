#include <Novice.h>
#include "Player.h"
#include <cmath>
#ifdef USE_IMGUI
#include <imgui.h>
#endif


void Player::Initialize() {
	state_ = PlayerState::Normal;

	position_ = { 200.0f, 500.0f };

	velocity_ = { 0.0f, 0.0f };

	size_ = { 40.0f, 60.0f };

	moveSpeed_ = 5.0f;

	facing_ = 1;

	hp_ = maxHp_;

	isGrounded_ = false;

	attackTimer_ = 0;

	isHammerActive_ = false;

	wasLeftMouseDown_ = false;

	isAiming_ = false;

	recoilTimer_ = 0;

	hurtTime_ = 0;

	for (int i = 0; i < kMaxBullets_; i++)
	{
		bullets_[i] = {};
	}

	airVelocity_ = 0.0f;

	hasHammerHit_ = false;

	hammerHitStopTimer_ = 0;
}


void Player::Update(const char* keys, const char* preKeys) {
	(void)preKeys;
	if (invincibleTimer_>0)
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
		bullets_[i].position.x += bullets_[i].velocity.x;
		bullets_[i].position.y += bullets_[i].velocity.y;

		bullets_[i].life--;
		if (bullets_[i].life <= 0) {
			bullets_[i].isActive = false;
		}

	}


	// 通常状態で右ボタンを押している間だけ照準
	isAiming_ = (state_== PlayerState::Normal && isRightMouseDown);

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
		if (isLeftMouseTriggered)
		{
			if (isAiming_)
			{
				// マグナム発射
				// 反動で射撃方向と反対に吹き飛ぶ
				Vector2 center = {position_.x + size_.x * 0.5f,position_.y + size_.y * 0.5f};
				Vector2 direction = { static_cast<float>(mouseX_) - center.x,static_cast<float>(mouseY_) - center.y };
				float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
				if (length>0.001f)
				{
					direction.x /= length;
					direction.y /= length;

				}
				else
				{
					direction = { static_cast<float>(facing_),0.0f };
				}
				for (int i = 0; i < kMaxBullets_; i++)
				{
					if (!bullets_[i].isActive)
					{
						bullets_[i].position = { center.x + direction.x * 30,center.y + direction.y * 30 };
						bullets_[i].isActive = true;
						bullets_[i].velocity = { direction.x * bulletSpeed_,direction.y * bulletSpeed_ };
						bullets_[i].life = bulletLife_;
						break;
					}
				}

				velocity_.x = -direction.x * recoilPowerX_;
				velocity_.y = -direction.y * recoilPowerX_;


				isGrounded_ = false;

				recoilTimer_ = recoilDuration_;
				isAiming_ = false;

				// 反動状態へ
				state_ = PlayerState::GunRecoil;
			}
			else
			{
				state_ = PlayerState::HammerCharge;
				chargeFrames_ = 0;
				hammerAngle_ = -2.0f;
				attackTimer_ = 0;
				hammerHitStopFrames_ = 0;
				isHammerActive_ = false;
				hasHammerHit_ = false;
				if (isGrounded_)
				{
					velocity_.x *= 0.35f;
				}
			}
		}
	}
	else if (state_ == PlayerState::HammerCharge)
	{
		const float centerX = position_.x + size_.x * 0.5f;
		if (mouseX_ > centerX)
		{
			facing_ = 1;
		}
		if (mouseX_ < centerX)
		{
			facing_ = -1;
		}

		float moveInput = 0.0f;
		if (keys[DIK_A]) moveInput -= 1.0f;
		if (keys[DIK_D]) moveInput += 1.0f;

		if (isGrounded_ || moveInput != 0.0f)
		{
			velocity_.x = moveInput * moveSpeed_ * 0.35f;
		}
		else
		{
			velocity_.x *= 0.99f;  // 空中でキーを離したら慣性を残す
		}

		if (isLeftMouseDown)
		{
			if (chargeFrames_ < kMaxChargeFrames_)
			{
				chargeFrames_++;
			}

			hammerAngle_ = std::fmod(hammerAngle_ + 0.12f, 6.2f);
		}
		else
		{
			// 左クリックを離した瞬間の角度を保存
			releaseAngle_ = hammerAngle_;
			attackTimer_ = 0;
			state_ = PlayerState::HammerAttack;
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

			constexpr float kRaiseAngle = -2.2f;
			constexpr float kEndAngle = 0.85f;
			constexpr float kTwoPi = 6.28f;

			if (attackTimer_ <= 6)
			{
				// 現在の角度から近い方向へ振りかぶる
				const float turn = std::remainder(kRaiseAngle - releaseAngle_, kTwoPi);
				hammerAngle_ = releaseAngle_ + turn * (attackTimer_ / 6.0f);
			}
			else if (attackTimer_ <= 18)
			{
				const float t = (attackTimer_ - 6) / 12.0f;
				hammerAngle_ = kRaiseAngle + (kEndAngle - kRaiseAngle) * t;
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
		if (recoilTimer_<=0)
		{
			velocity_.x = 0.0f;
			state_ = PlayerState::Normal;
		}
	}
	else if (state_==PlayerState::Hurt)
	{
		velocity_.x *= 0.9f;
		hurtTime_--;
		if (hurtTime_<=0)
		{
			state_ = PlayerState::Normal;
		}
	}

	else if (state_==PlayerState::Dead)
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
	

	airVelocity_ = velocity_.x;

}

void Player::Damage(int damage, float knockbackx, float knockbacky) {
	if (damage <=0||invincibleTimer_>0||state_==PlayerState::Dead)
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

Vector2 Player::GetHammerHeadPoint(int index) const
{
	const float dirX = std::cos(hammerAngle_) * static_cast<float>(facing_);
	const float dirY = std::sin(hammerAngle_);

	const float handX = position_.x + size_.x * 0.5f;
	const float handY = position_.y + size_.y * 0.5f;

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
		const float nearX = std::fmax(targetPos.x,std::fmin(p.x, targetPos.x + targetSize.x));
		const float nearY = std::fmax(targetPos.y,std::fmin(p.y, targetPos.y + targetSize.y));

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


void Player::Draw()const {
	Novice::DrawBox(
		static_cast<int>(position_.x),
		static_cast<int>(position_.y),
		static_cast<int>(size_.x), 
		static_cast<int>(size_.y), 
		0.0f, WHITE,kFillModeSolid);

	// 照準中だけレーザーを表示する
	if (isAiming_)
	{
		Vector2 center = { position_.x + size_.x * 0.5f,position_.y + size_.y * 0.5f };
	
		Vector2 direction = { static_cast<float>(mouseX_) - center.x,static_cast<float>(mouseY_) - center.y };
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

		if (length > 0.001f)
		{
			direction.x /= length;
			direction.y /= length;
		}
		else
		{
			direction = { static_cast<float>(facing_),0.0f };
		}
		//仮の銃口位置
		Vector2 muzzlePos = { center.x + direction.x * 30.0f,center.y + direction.y * 30.0f };

		// 銃口から伸びるレーザーの終点
		float leserLenght = 2500.0f;

		Vector2 laserEnd = {
			muzzlePos.x + direction.x * leserLenght,
			muzzlePos.y + direction.y * leserLenght
		};
		Novice::DrawLine(
			static_cast<int>(center.x),
			static_cast<int>(center.y),
			static_cast<int>(muzzlePos.x),
			static_cast<int>(muzzlePos.y),
			WHITE
		);
		Novice::DrawLine(
			static_cast<int>(muzzlePos.x),
			static_cast<int>(muzzlePos.y),
			static_cast<int>(laserEnd.x),
			static_cast<int>(laserEnd.y),
			0xFF000080
		);

	}



	if (state_ == PlayerState::HammerCharge || state_ == PlayerState::HammerAttack)
	{
		const int handX = static_cast<int>(position_.x + size_.x * 0.5f);
		const int handY = static_cast<int>(position_.y + size_.y * 0.5f);
		const Vector2 head = GetHammerHeadPoint(0);


		for (int i = 0; i <=3; i++)
		{
			Novice::DrawLine(
				handX + i, handY,
				static_cast<int>(head.x) + i,
				static_cast<int>(head.y),
				0xAF7848FF
			);
		}
		// 金属のヘッド。最大まで溜めると金色になる
		const unsigned int color = chargeFrames_ >= kMaxChargeFrames_ ? 0xFFD050FF : 0xAAB8C8FF;

		for (int i = -1; i <= 1; i++)
		{
			const Vector2 p = GetHammerHeadPoint(i);
			Novice::DrawEllipse(
				static_cast<int>(p.x), static_cast<int>(p.y),
				static_cast<int>(kHeadRadius_), static_cast<int>(kHeadRadius_),
				0.0f, color, kFillModeSolid);
		}

	}

	for (int i = 0; i < kMaxBullets_; i++)
	{
		if (!bullets_[i].isActive)
		{
			continue;
		}
		Novice::DrawBox(
			static_cast<int>(bullets_[i].position.x - 4.0f),
			static_cast<int>(bullets_[i].position.y - 4.0f), 
			8, 8,
			0.0f, 0xFF8800FF, kFillModeSolid);
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

	ImGui::End();
#endif
}





	