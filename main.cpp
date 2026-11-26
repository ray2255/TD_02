#include <Novice.h>
#include "Player.h"
#include "TrainingDummy.h"
#include <cmath>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

const char kWindowTitle[] = "LC1C_07_クギヤマ_ソラ_タイトル";

//スローモーションの視覚効果
void DrawSlowMotionCircle(Vector2 center, int framesLeft, int totalFrames)
{
	if (framesLeft <= 0)
	{
		return;
	}

	constexpr int kExpandFrames = 15;
	constexpr int kShrinkFrames = 15;

	const int elapsed = totalFrames - framesLeft;
	float radiusRate = 1.0f;

	if (elapsed < kExpandFrames)
	{
		// 広がる：0 → 1
		radiusRate = static_cast<float>(elapsed) / (kExpandFrames - 1);
	}
	else if (framesLeft <= kShrinkFrames)
	{
		// プレイヤーの位置に戻る：1 → 0
		radiusRate = static_cast<float>(framesLeft - 1) / (kShrinkFrames - 1);
	}

	// 円の広がり始めと止まり際を滑らかにする
	radiusRate = radiusRate * radiusRate * (3.0f - 2.0f * radiusRate);

	// 1280 × 720 の画面で、プレイヤーから最も遠い角までの距離
	const float farX = std::fmax(std::fabs(center.x), std::fabs(1280.0f - center.x));
	const float farY = std::fmax(std::fabs(center.y), std::fabs(720.0f - center.y));
	const float maxRadius = std::sqrt(farX * farX + farY * farY) + 2.0f;

	const int radius = static_cast<int>(maxRadius * radiusRate);
	if (radius > 0)
	{
		Novice::DrawEllipse(
			static_cast<int>(center.x),
			static_cast<int>(center.y),
			radius, radius,
			0.0f, 0x00000066, kFillModeSolid
		);
	}
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Player player;
	TrainingDummy trainingDummy;
	bool isTrainingDummyActive = false;
	constexpr int kSlowMotionFrame = 300;
	constexpr int kWorldStep = 4;
	int slowMotiomLeft = 0;


	player.Initialize();
	trainingDummy.Initialize({ 800.0f, 440.0f });


	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		
		player.DebugGui();

#ifdef USE_IMGUI
		ImGui::Begin("Training Dummy Debug");

		if (!isTrainingDummyActive)
		{
			if (ImGui::Button("Spawn Dummy"))
			{
				trainingDummy.Initialize({ 800.0f, 440.0f });
				isTrainingDummyActive = true;
			}
		}
		else
		{
			if (ImGui::Button("Reset Dummy"))
			{
				trainingDummy.Initialize({ 800.0f, 440.0f });
			}

			if (ImGui::Button("Delete Dummy"))
			{
				isTrainingDummyActive = false;
			}
		}

		ImGui::Text(
			"Active: %s",
			isTrainingDummyActive ? "Yes" : "No"
		);

		ImGui::End();
#endif


		if (player.ConsumePerfectDodge())
		{
			slowMotiomLeft = kSlowMotionFrame;
		}
		const bool isSlowMotion = slowMotiomLeft > 0;
		const float worldSpeed = isSlowMotion ? 1.0f / static_cast<float>(kWorldStep) : 1.0f;
		const bool updateWorld = !isSlowMotion || ((kSlowMotionFrame - slowMotiomLeft) % kWorldStep == 0);

		player.Update(keys, preKeys, worldSpeed, updateWorld);
		
		if (isTrainingDummyActive)
		{
			if (updateWorld)
			{
				trainingDummy.Update();
			}
			if (player.CheckHammerHit(trainingDummy.GetPosition(), trainingDummy.GetSize()))
			{
				if (trainingDummy.IsArmorBreak())
				{
					if (trainingDummy.DamageBody(player.GetHammerDamage(3)))
					{
						player.HealFromHammerHit();
					}
				}
				else
				{
					trainingDummy.DamageArmor(player.GetHammerDamage(10));
				}
			}
			if (player.CheckBulletHit(trainingDummy.GetPosition(), trainingDummy.GetSize()))
			{
				if (trainingDummy.IsArmorBreak())
				{
					trainingDummy.DamageBody(25);
				}
				else
				{
					trainingDummy.DamageArmor(3);
				}
			}
		}
	




		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		
		if (isTrainingDummyActive)
		{
			trainingDummy.Draw();
		}

		player.Draw();

		Novice::DrawLine(0, 600, 1280, 600, WHITE	);

		DrawSlowMotionCircle(player.GetCenter(), slowMotiomLeft, kSlowMotionFrame);


		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		if (slowMotiomLeft > 0)
		{
			Novice::ScreenPrintf(20, 80, "slow motton: %d", slowMotiomLeft);
			slowMotiomLeft--;
		}
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
