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
}


void Player::Update(const char* keys, const char* preKeys) {
	(void)preKeys;
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
		// 左へ移動
		if (keys[DIK_A])
		{
			velocity_.x -= moveSpeed_;
		}
		// 右へ移動
		if (keys[DIK_D])
		{
			velocity_.x += moveSpeed_;
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
				// 弾の生成処理は後で追加する
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
				velocity_.x = -direction.x * recoilPowerX_;
				velocity_.y = -direction.y * recoilPowerY_;


				isGrounded_ = false;

				recoilTimer_ = recoilDuration_;
				isAiming_ = false;

				// 反動状態へ
				state_ = PlayerState::GunRecoil;
			}
			else
			{
				// ハンマー攻撃
				state_ = PlayerState::HammerAttack;
				attackTimer_ = 0;
				isHammerActive_ = false;
				velocity_.x = 0.0f;
			}
		}
	}
	else if (state_ == PlayerState::HammerAttack)
	{
		velocity_.x = 0.0f; // 攻撃中は横移動を止める
		attackTimer_++; // 攻撃時間を進める
		isHammerActive_ = (attackTimer_ >= 10 && attackTimer_ < 10); // 10～17フレーム目に攻撃判定を出す
		if (attackTimer_ >= 30)// 攻撃終了
		{
			state_ = PlayerState::Normal;
			attackTimer_ = 0;
			isHammerActive_ = false;
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
}

void Player::Draw()const {
	Novice::DrawBox(
		static_cast<int>(position_.x),
		static_cast<int>(position_.y),
		static_cast<int>(size_.x), 
		static_cast<int>(size_.y), 
		0.0f, WHITE,kFillModeSolid);

}






//DeBug用
void Player::DebugGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("Player Debug");

	// 現在の情報
	ImGui::Text("Position: %.1f, %.1f", position_.x, position_.y);
	ImGui::Text("Velocity: %.1f, %.1f", velocity_.x, velocity_.y);
	ImGui::Text("HP: %d / %d", hp_, maxHp_);
	ImGui::Text("Facing: %s", facing_ == 1 ? "Right" : "Left");
	ImGui::Text("Grounded: %s", isGrounded_ ? "Yes" : "No");
	ImGui::Text("Aiming: %s", isAiming_ ? "Yes" : "No");

	const char* stateName = "Unknown";

	switch (state_)
	{
	case PlayerState::Normal:
		stateName = "Normal";
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

	ImGui::End();
#endif
}





	