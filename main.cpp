#include <Novice.h>
#include "Player.h"
#include "TrainingDummy.h"
#include <cmath>
#include <cstring>
#ifdef USE_IMGUI
#include <imgui.h>
#endif

const char kWindowTitle[] = "LC1C_07_クギヤマ_ソラ_タイトル";

enum class GameScene
{
	Title,
	Training,
	BossBattle,
	Victory,
	Defeat
};



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
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    Novice::Initialize(kWindowTitle, 1280, 720);

    Player player;
    TrainingDummy trainingDummy;
    GameScene scene = GameScene::Title;

    const Vector2 dummyStart = { 800.0f, 440.0f };
    bool isTrainingDummyActive = false;
    bool isRunning = true;

    constexpr int kSlowMotionFrame = 300;
    constexpr int kWorldStep = 4;
    int slowMotionLeft = 0;

    char keys[256] = {};
    char preKeys[256] = {};

    // 画像はゲーム起動時に1回だけ読み込む
    player.LoadTextures();

    // タイトルから開始するときと、リトライするときに使う
    auto startStage = [&](GameScene nextScene)
        {
            player.Initialize();
            trainingDummy.Initialize(dummyStart);
            slowMotionLeft = 0;
            isTrainingDummyActive = (nextScene == GameScene::BossBattle);
            scene = nextScene;
        };

    while (isRunning && Novice::ProcessMessage() == 0)
    {
        Novice::BeginFrame();

        std::memcpy(preKeys, keys, sizeof(keys));
        Novice::GetHitKeyStateAll(keys);

        const bool enterPressed = keys[DIK_RETURN] && !preKeys[DIK_RETURN];
        const bool escapePressed = keys[DIK_ESCAPE] && !preKeys[DIK_ESCAPE];
        const bool bossPressed = keys[DIK_B] && !preKeys[DIK_B];
        const bool retryPressed = keys[DIK_R] && !preKeys[DIK_R];

        if (scene == GameScene::Title)
        {
            Novice::ScreenPrintf(550, 250, "BOSS BATTLE");
            Novice::ScreenPrintf(500, 330, "ENTER : Training Room");
            Novice::ScreenPrintf(500, 360, "B     : Boss Battle");
            Novice::ScreenPrintf(500, 390, "ESC   : Quit");

            if (enterPressed)
            {
                startStage(GameScene::Training);
            }
            else if (bossPressed)
            {
                startStage(GameScene::BossBattle);
            }
            else if (escapePressed)
            {
                isRunning = false;
            }
        }
        else if (scene == GameScene::Victory || scene == GameScene::Defeat)
        {
            const bool won = (scene == GameScene::Victory);
            const unsigned int background = won ? 0x16382CFF : 0x381B24FF;

            Novice::DrawBox(0, 0, 1280, 720, 0.0f, background, kFillModeSolid);
            Novice::ScreenPrintf(570, 270, won ? "VICTORY!" : "GAME OVER");
            Novice::ScreenPrintf(510, 350, "R     : Retry");
            Novice::ScreenPrintf(510, 380, "ENTER : Back to Title");

            if (retryPressed)
            {
                startStage(GameScene::BossBattle);
            }
            else if (enterPressed || escapePressed)
            {
                scene = GameScene::Title;
            }
        }
        else
        {
            // Training と BossBattle で共通のプレイヤー処理
            player.DebugGui();

#ifdef USE_IMGUI
            if (scene == GameScene::Training)
            {
                ImGui::Begin("Training Dummy Debug");

                if (!isTrainingDummyActive)
                {
                    if (ImGui::Button("Spawn Dummy"))
                    {
                        trainingDummy.Initialize(dummyStart);
                        isTrainingDummyActive = true;
                    }
                }
                else
                {
                    if (ImGui::Button("Reset Dummy"))
                    {
                        trainingDummy.Initialize(dummyStart);
                    }

                    if (ImGui::Button("Delete Dummy"))
                    {
                        isTrainingDummyActive = false;
                    }
                }

                ImGui::Text("Active: %s", isTrainingDummyActive ? "Yes" : "No");
                ImGui::End();
            }
#endif

            // ジャスト回避とスローモーション
            if (player.ConsumePerfectDodge())
            {
                slowMotionLeft = kSlowMotionFrame;
            }

            const bool isSlowMotion = slowMotionLeft > 0;
            const float worldSpeed =
                isSlowMotion ? 1.0f / static_cast<float>(kWorldStep) : 1.0f;
            const bool updateWorld =
                !isSlowMotion || (kSlowMotionFrame - slowMotionLeft) % kWorldStep == 0;

            player.Update(keys, preKeys, worldSpeed, updateWorld);

            // 当たり判定
            if (isTrainingDummyActive)
            {
                if (updateWorld)
                {
                    trainingDummy.Update();
                }

                const Vector2 targetPos = trainingDummy.GetPosition();
                const Vector2 targetSize = trainingDummy.GetSize();

                if (player.CheckHammerHit(targetPos, targetSize))
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

                if (player.CheckBulletHit(targetPos, targetSize))
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

                trainingDummy.Draw();
            }

            player.Draw();
            Novice::DrawLine(0, 600, 1280, 600, WHITE);
            DrawSlowMotionCircle(player.GetCenter(), slowMotionLeft, kSlowMotionFrame);

            if (slowMotionLeft > 0)
            {
                Novice::ScreenPrintf(20, 80, "Slow motion: %d", slowMotionLeft);
                --slowMotionLeft;
            }

            // 勝敗判定はボス戦だけ
            if (scene == GameScene::BossBattle && player.IsDead())
            {
                scene = GameScene::Defeat;
            }
            else if (scene == GameScene::BossBattle && trainingDummy.IsDefeated())
            {
                scene = GameScene::Victory;
            }
            else if (escapePressed)
            {
                scene = GameScene::Title;
            }
        }

        Novice::EndFrame();
    }

    Novice::Finalize();
    return 0;
}