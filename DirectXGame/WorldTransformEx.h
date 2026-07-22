#pragma once
#include "KamataEngine.h" // WorldTransformクラス利用
class WorldTransformEx : public KamataEngine::WorldTransform {
public:
	//Affine変換行列の生成と定数バッファへの転送を行う
	void UpdateMatrix();

	//Affine変換行列の生成
	KamataEngine::Matrix4x4 MakeAffineMatrix();

};
