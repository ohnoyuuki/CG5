#include <Windows.h>
#include"KamataEngine.h"

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//エンジンの初期化
	KamataEngine::Initialize(L"LE3D_05_オオノ_ユウキ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width:{} height:{}", w, h).c_str());
	
	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();


	// RootSignatureの作成
	//構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{}; // RootSignatureの設定
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // InputLayoutを使う場合は必須
	ID3DBlob* singnedBlob = nullptr;                                                               // 署名済みバイナリ格納用

	// RootSignatureの作成-------------------------------------------------------------------------------
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlog = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlog);

	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;

	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	assert(SUCCEEDED(hr));













	//メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break; // Updateがtrueを返したらループを抜ける
		}


		// 描画前処理
		dxCommon->PreDraw();
		// ここに描画処理を記述する



		// 描画終了
		dxCommon->PostDraw();


	}

	// エンジンの終了処理
	KamataEngine::Finalize();



	return 0;
}
