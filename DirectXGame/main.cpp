#include "KamataEngine.h"
#include <Windows.h>
#include <d3dcompiler.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
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
	// 構造体にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};                                          // RootSignatureの設定
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

	// InputLayout----------------------------------------------------------------------------------------------------------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState----------------------------------------------------------------------------------------------------------
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState----------------------------------------------------------------------------------------------------------
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（反時計回り）をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする（ワイヤーフレームならD3D12_FILL_MODE_WIREFRAME）
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// vertexShaderをコンパイルする------------------------------------------------------------------------------------------
	// コンパイル済みのShader、エラー時情報の格納場所の用意
	ID3DBlob* vsBlob = nullptr;    // 頂点シェーダオブジェクト
	ID3DBlob* psBlob = nullptr;    // ピクセルシェーダオブジェクト
	ID3DBlob* errorBlob = nullptr; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	std::wstring vsFile = L"Resources/shaders/TestVS.hlsl";
	hr = D3DCompileFromFile(
	    vsFile.c_str(), // シェーダファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", "vs_5_0",                                // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &vsBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}


	// PixeShader------------------------------------------------------------------------------------------
	// ピクセルシェーダの読み込みとコンパイル
	std::wstring psFile = L"Resources/shaders/TestPS.hlsl";
	hr = D3DCompileFromFile(
	    psFile.c_str(), // シェーダファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", "ps_5_0",                                // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &psBlob, &errorBlob);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(std::system_category().message(hr).c_str());
		if (errorBlob) {
			DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}


	// PSO(PipelineStateObject)の生成 ---------------------------------------------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;// RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;// InputLayout
	graphicsPipelineStateDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};// VertexShader
	graphicsPipelineStateDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};// PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                      // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;            // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1; // 1つのRTVに書き込む ※2つ同時にしようと思えばできる
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(今は気にしなくていい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 準備は整った。PSOを生成する
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));





















	// メインループ
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
