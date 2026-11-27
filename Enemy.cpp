#include "Enemy.h"
#include "Cooldown.h"
#include <Novice.h>
#define _USE_MATH_DEFINES
#include <math.h>

void Enemy::Initialize() {
	// 初期化処理
	worldPosition_ = { 440.0f, 260.0f };
	localPosition_ = { 0.0f, 0.0f };
	velocity_ = { 0.0f, 0.0f };
	localAngle = 0.0f;
	animTimer_ = 0.0f;

	
	// 状態とタイマーの初期化を追加
	state_ = EnemyState::Idle;
	currentAttack_ = EnemyAtteck::Normal;
	attackTimer_ = 0.0f;
	// ボスの部位の初期化
	for (int i = 0; i < kbossparts; ++i) {
		parts_[i].isAlive = true;
		parts_[i].isArmorNotBroken = true;
		parts_[i].position = { 0.0f, 0.0f };
		parts_[i].localPosition = { 0.0f, 0.0f };
		parts_[i].angle = 0.0f;
		parts_[i].localAngle = 0.0f;
		worldPosition_.x = 800;
		//体力の初期化
		parts_[i].maxHp = 1000;
		parts_[i].hp = parts_[i].maxHp;
		parts_[i].maxArmorHp = 300;
		parts_[i].armorHp = parts_[i].maxArmorHp;
	}
	int activeParts[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12 };
	for (int idx : activeParts) {
		parts_[idx].isAlive = true;
	}
	// テクスチャ読み込み
	parts_[0].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/Body.png");
	parts_[1].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/neck.png");
	parts_[2].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/neck.png");
	parts_[3].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/head1.png");
	parts_[4].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/head2.png");
	parts_[5].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexTail0.png");
	parts_[6].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexTail2.png");
	parts_[7].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexTail1.png");
	parts_[8].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexArm1.png");
	parts_[9].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/enemyArm2.png");
	parts_[10].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexArm1.png");
	parts_[11].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/enemyArm2.png");
	parts_[12].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexFoot.png");
	parts_[13].bodyTexture = Novice::LoadTexture("./Resources/images/enemy/rexFoot.png");

	//アーマー
	parts_[0].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexTorso.png");
	parts_[1].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexNeckarmer.png");
	parts_[2].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexNeckarmer.png");
	parts_[3].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexhelm.png");
	parts_[4].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexagithelm.png");
	parts_[5].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexTailarmer2.png");
	parts_[6].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexTailarmer4.png");
	parts_[7].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexTailarmer3.png");
	parts_[8].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexGauntret1.png");
	parts_[9].armorTexture = Novice::LoadTexture("./Resources/images/enemy/Gauntret.png");
	parts_[10].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexGauntret1.png");
	parts_[11].armorTexture = Novice::LoadTexture("./Resources/images/enemy/Gauntret.png");
	parts_[12].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexregins.png");
	parts_[13].armorTexture = Novice::LoadTexture("./Resources/images/enemy/rexregins.png");


	parts_[0].parentIndex = -1;
	parts_[0].baseLocalPosition = { 0.0f, 0.0f };
	parts_[0].localPosition = parts_[0].localPosition;

	// 首・頭・アゴ
	parts_[1].parentIndex = 0;
	parts_[1].baseLocalPosition = { 50.0f, 20.0f };
	parts_[1].localPosition = parts_[1].baseLocalPosition;


	parts_[2].parentIndex = 1;
	parts_[2].baseLocalPosition = { 45.0f, 20.0f };
	parts_[2].localPosition = parts_[2].baseLocalPosition;

	parts_[3].parentIndex = 2;
	parts_[3].baseLocalPosition = { 160.0f,-80.0f };
	parts_[3].localPosition = parts_[3].baseLocalPosition;

	parts_[4].parentIndex = 3;
	parts_[4].baseLocalPosition = { 0.0f, 0.0f };
	parts_[4].localPosition = parts_[4].baseLocalPosition;

	// 尻尾
	parts_[5].parentIndex = 0;
	parts_[5].baseLocalPosition = { -350.0f, 40.0f };
	parts_[5].localPosition = parts_[5].baseLocalPosition;

	parts_[6].parentIndex = 5;
	parts_[6].baseLocalPosition = { -140.0f, 0.0f };
	parts_[6].localPosition = parts_[6].baseLocalPosition;

	parts_[7].parentIndex = 6;
	parts_[7].baseLocalPosition = { -160.0f, 0.0f };
	parts_[7].localPosition = parts_[7].baseLocalPosition;

	// 右前脚
	parts_[8].parentIndex = 0;
	parts_[8].baseLocalPosition = { -50.0f, 50.0f };
	parts_[8].localPosition = parts_[8].baseLocalPosition;

	parts_[9].parentIndex = 8;
	parts_[9].baseLocalPosition = { 0.0f, -55.0f };
	parts_[9].localPosition = parts_[9].baseLocalPosition;

	// 左前脚
	parts_[10].parentIndex = 0;
	parts_[10].baseLocalPosition = { -50.0f, 50.0f };
	parts_[10].localPosition = parts_[8].baseLocalPosition;

	parts_[11].parentIndex = 10;
	parts_[11].baseLocalPosition = { 0.0f, -55.0f };
	parts_[11].localPosition = parts_[9].baseLocalPosition;

	// 後脚
	parts_[12].parentIndex = 0;
	parts_[12].baseLocalPosition = { -170.0f, 80.0f };
	parts_[12].localPosition = parts_[12].baseLocalPosition;

	parts_[13].parentIndex = 0;
	parts_[13].baseLocalPosition = { -220.0f, 80.0f };
	parts_[13].localPosition = parts_[13].baseLocalPosition;
}

void Enemy::Move(const Vector2& delta) {
	worldPosition_.x += delta.x;
	worldPosition_.y += delta.y;
}

void Enemy::Attack(EnemyAtteck attack) {
	if (!attackCooldown_.IsReady()) {
		return;
	}

	// 攻撃種類を設定
	currentAttack_ = attack;

	// 攻撃状態にする
	state_ = EnemyState::Attack;

	// 攻撃タイマーをリセット
	attackTimer_ = 0.0f;

	// クールダウン開始
	attackCooldown_.Start();

}

void Enemy::Draw() const {
	// 描画したい順番に番号を並べる（奥にしたい10〜13を先に書く）
	const int drawOrder[kbossparts] = {
		10, 11, 13,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,12,14,15 // ※kbosspartsの数に合わせて残りを並べる
	};

	for (int i = 0; i < kbossparts; ++i) {
		int index = drawOrder[i];

		if (!parts_[index].isAlive) {
			continue;
		}

		// 本体
		Novice::DrawSprite(
			(int)parts_[index].position.x,
			(int)parts_[index].position.y,
			parts_[index].bodyTexture,
			1.0f, 1.0f,
			parts_[index].angle,
			0xFFFFFFFF
		);

		// 装甲（耐久値が残っているときだけ描画）
		if (parts_[index].armorHp > 0) {
			Novice::DrawSprite(
				(int)parts_[index].position.x,
				(int)parts_[index].position.y,
				parts_[index].armorTexture,
				1.0f, 1.0f,
				parts_[index].angle,
				0xFFFFFFFF
			);
		}
	}
}

void Enemy::Update() {
	attackCooldown_.Update();

	//////////////////////////////////////////////////////////////
	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };
	Novice::GetHitKeyStateAll(keys);
	// デバッグ用：Aキーを押した瞬間に 0番の装甲耐久値を 0 にする
	if (keys[DIK_A] && preKeys[DIK_A] == 0) {
		parts_[0].armorHp = 0;
	}
	if (keys[DIK_B] && preKeys[DIK_B] == 0) {
		Attack(EnemyAtteck::bite);
	}
	if (keys[DIK_V] && preKeys[DIK_V] == 0) {
		Attack(EnemyAtteck::Tellslam);
	}
	if (keys[DIK_N] && preKeys[DIK_N] == 0) {
		Attack(EnemyAtteck::Rush);
	}



	////////////////////////////////////////////////



	// 1. 先に「ローカル座標・角度」を更新する
	if (state_ != EnemyState::Attack) {
		static float timer = 0.0f;
		timer += 0.03f;

		
		// 基準の位置 + sinf(タイマー * 速さ) * 動きの大きさ
		parts_[0].localPosition.y = -sinf(timer * 4.0f) * 5.0f;
		parts_[0].localAngle = sinf(timer * 2.0f) * 0.03f;

		parts_[1].localAngle = sinf(timer * 2.0f) * 0.01f;
		parts_[2].localAngle = sinf(timer * 2.0f) * 0.02f;
		parts_[3].localAngle = sinf(timer * 2.0f) * 0.01f;

		parts_[5].localAngle = (15.0f * (float)M_PI / 180.0f) + sinf(timer * 1.5f) * 0.08f;
		parts_[6].localAngle = sinf(timer * 1.5f - 0.4f) * 0.12f;
		parts_[7].localAngle = sinf(timer * 1.5f - 0.8f) * 0.16f;


		parts_[8].localAngle = -sinf(timer * 2.0f) * 0.8f;
		parts_[9].localAngle = -sinf(timer * 2.0f) * 0.3f;

		parts_[10].localAngle = sinf(timer * 2.0f) * 0.8f;
		parts_[11].localAngle = sinf(timer * 2.0f) * 0.3f;
		//足
		parts_[12].localAngle = sinf(timer * 2.0f) * 0.2f;
		parts_[13].localAngle = -sinf(timer * 2.0f) * 0.2f;
	}
	else {
		// 攻撃中のローカル座標・角度の更新
		attackTimer_ += 0.02f;

		switch (currentAttack_) {
		////////////////////////////////
		//☆噛みつき
		////////////////////////////////
		case EnemyAtteck::bite:
			
			// 1. 首の根元を少し左下に伸ばす
			parts_[1].localPosition.x = parts_[1].baseLocalPosition.x + sinf(attackTimer_ * (float)M_PI) * 40.0f;
			parts_[1].localPosition.y = parts_[1].baseLocalPosition.y + sinf(attackTimer_ * (float)M_PI) * 25.0f;

			// 2. 首の先もさらに左下に伸ばす
			parts_[2].localPosition.x = parts_[2].baseLocalPosition.x + sinf(attackTimer_ * (float)M_PI) * 45.0f;
			parts_[2].localPosition.y = parts_[2].baseLocalPosition.y + sinf(attackTimer_ * (float)M_PI) * 25.0f;

			// 3. 頭も左下に伸ばす
			parts_[3].localPosition.x = parts_[3].baseLocalPosition.x + sinf(attackTimer_ * (float)M_PI) * 55.0f;
			parts_[3].localPosition.y = parts_[3].baseLocalPosition.y + sinf(attackTimer_ * (float)M_PI) * 30.0f;

			// 4. 下アゴを開閉する
			parts_[4].localAngle = sinf(attackTimer_ * (float)M_PI) * 0.6f;

			// 攻撃終了
			if (attackTimer_ >= 1.0f) {
				state_ = EnemyState::Idle;
				currentAttack_ = EnemyAtteck::Normal;
				attackTimer_ = 0.0f;

				parts_[1].localPosition.x = parts_[1].baseLocalPosition.x;
				parts_[1].localPosition.y = parts_[1].baseLocalPosition.y;

				parts_[2].localPosition.x = parts_[2].baseLocalPosition.x;
				parts_[2].localPosition.y = parts_[2].baseLocalPosition.y;

				parts_[3].localPosition.x = parts_[3].baseLocalPosition.x;
				parts_[3].localPosition.y = parts_[3].baseLocalPosition.y;

				parts_[4].localAngle = 0.0f;
			}
			break;


		////////////////////////////////
		//尻尾攻撃
		////////////////////////////////
		case EnemyAtteck::Tellslam: 
		
			parts_[5].localAngle = sinf(attackTimer_ * speed * (float)M_PI) * -0.30f;
			parts_[6].localAngle = sinf(attackTimer_ * speed * (float)M_PI - 0.4f) * -0.34f;
			parts_[7].localAngle = sinf(attackTimer_ * speed * (float)M_PI - 0.8f) * -0.38f;



			if (attackTimer_ >= 1.26f) {
				state_ = EnemyState::Idle;
				currentAttack_ = EnemyAtteck::Normal;
				attackTimer_ = 0.0f;



			}

			break;
			////////////////////////////////
			//☆突撃
			////////////////////////////////
		case EnemyAtteck::Rush: 

			float rushTime = attackTimer_ - 0.4f;
			float runSpeed = 9.0f; // 腕・脚の回転速度（お好みで調整）
			float cycle = rushTime * runSpeed;

			parts_[0].localPosition.y = -sinf(attackTimer_ * (float)M_PI) * 5.0f;
			parts_[0].localAngle = sinf(attackTimer_ * (float)M_PI) * 0.03f;

			parts_[8].localAngle = -sinf(attackTimer_ * speed * (float)M_PI) * 0.8f;
			parts_[9].localAngle = -sinf(attackTimer_ * speed * (float)M_PI) * 0.3f;

			parts_[10].localAngle = sinf(attackTimer_ * speed * (float)M_PI) * 0.8f;
			parts_[11].localAngle = sinf(attackTimer_ * speed * (float)M_PI) * 0.3f;
			//足
			parts_[12].localAngle = sinf(attackTimer_ * speed * (float)M_PI) * 0.25f;
			parts_[13].localAngle = -sinf(attackTimer_ * speed * (float)M_PI) * 0.25f;

			

			parts_[1].localAngle = 0.1f + sinf(cycle * 2.0f) * 0.03f;
			parts_[2].localAngle = 0.1f + sinf(cycle * 2.0f - 0.3f) * 0.04f;
			parts_[3].localAngle = sinf(cycle * 2.0f - 0.6f) * 0.05f;

			parts_[5].localAngle = (15.0f * (float)M_PI / 180.0f) + sinf(attackTimer_ *1.2f * (float)M_PI) * 0.12f;
			parts_[6].localAngle = sinf(attackTimer_ * 1.2f * (float)M_PI - 0.4f) * 0.16f;
			parts_[7].localAngle = sinf(attackTimer_ * 1.2f * (float)M_PI - 0.8f) * 0.20f;

			parts_[4].localAngle = 0.5f;


			
			if (attackTimer_ >= 3.00f) {
				state_ = EnemyState::Idle;
				currentAttack_ = EnemyAtteck::Normal;
				attackTimer_ = 0.0f;
				parts_[4].localAngle = 0.0f;
			}

			break;
		}
	}

	// 2. 最後に親子関係を計算して描画座標（position / angle）を確定させる
	for (int i = 0; i < kbossparts; ++i) {
		if (!parts_[i].isAlive) {
			continue;
		}

		int parent = parts_[i].parentIndex;
		//胴体
		if (parent == -1) {
			parts_[i].angle = -parts_[i].localAngle;
			parts_[i].position.x = worldPosition_.x - parts_[i].localPosition.x;
			parts_[i].position.y = worldPosition_.y + parts_[i].localPosition.y;
		}
		//それ以外
		else {
			// 親の傾きに、自分の曲げ角度を足し合わせる
			parts_[i].angle = parts_[parent].angle - parts_[i].localAngle;

			// 親が回転した分だけ関節の位置も一緒に回すためのサイン・コサイン
			float c = cosf(parts_[parent].angle);
			float s = sinf(parts_[parent].angle);

			// 親の現在地を基準に、回転させたズレ（localPosition）を足し引きして座標を確定させる
			// （※X軸は左向きをプラスとする座標系のため引き算になっている）
			parts_[i].position.x = parts_[parent].position.x - (parts_[i].localPosition.x * c + parts_[i].localPosition.y * s);
			parts_[i].position.y = parts_[parent].position.y + (-parts_[i].localPosition.x * s + parts_[i].localPosition.y * c);
		}
	}
	//あごのずれ直し
	// 下アゴ(parts_[4])の回転軸を「画像の左上」から「アゴの付け根」へずらす
	if (parts_[4].isAlive) {
		// 画像の左上(0,0)から見た「アゴの関節（回転させたい中心）」のピクセル座標
		const Vector2 jawPivot = { 180.0f, 120.0f };

		// 親（頭：parts_[3]）の回転に合わせた「関節のワールド座標」を計算
		float c3 = cosf(parts_[3].angle);
		float s3 = sinf(parts_[3].angle);
		float jointWorldX = parts_[3].position.x + (jawPivot.x * c3 - jawPivot.y * s3);
		float jointWorldY = parts_[3].position.y + (jawPivot.x * s3 + jawPivot.y * c3);

		// 下アゴ（parts_[4]）を回転させたとき、関節位置がズレないように左上座標を逆算
		float c4 = cosf(parts_[4].angle);
		float s4 = sinf(parts_[4].angle);
		parts_[4].position.x = jointWorldX - (jawPivot.x * c4 - jawPivot.y * s4);
		parts_[4].position.y = jointWorldY - (jawPivot.x * s4 + jawPivot.y * c4);
	}
}