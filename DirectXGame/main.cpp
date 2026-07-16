#include "IndexBuffer.h"
#include "KamataEngine.h"
#include "PipelineState.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "rootSignature.h"
#include <Windows.h>

// #include <d3dcompiler.h>

using namespace KamataEngine;

// 関数プロトタイプ宣言
void SetupPipelineState(PipelineState* pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {
	// InputLayout --------------------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（班時計回り）をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// PSO(PipelineStateObject)の生成 ---------------------------------------------------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();                                                    // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // InputLayout
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // VertexShader
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                             // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1; // 1つのRTVに書き込む ※2つ同時にしようと思えばできる
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(今は気にしなくていい)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 準備は整った。PSOを生成する
	pipelineState->Create(graphicsPipelineStateDesc);
}

// RenderTextureResourceの生成(プロトタイプ)
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT clearformat, const FLOAT* clearColor) {
	// 1.生成するRenderTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);                             // RenderTextureの幅
	resourceDesc.Height = UINT(height);                           // RenderTextureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数
	resourceDesc.DepthOrArraySize = 1;                            // 奥行or配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;        // Textureのフォーマット
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // Textureの次元数。普段使っているのは2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使うためのフラグ

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 3.ClearValueの用意
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = clearformat;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4. RenderTextureResourceの生成
	ID3D12Resource* resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                            // Heapの設定
	    D3D12_HEAP_FLAG_NONE,                       // Heapの特殊な設定
	    &resourceDesc,                              // Resourceの設定
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // Pixel Shader でアクセスできるようにする
	    &clearValue,                                // Clear最適値
	    IID_PPV_ARGS(&resource)                     // 作成するResourceポインタへのポインタ
	);

	assert(SUCCEEDED(hr));

	return resource;
}

// DepthStencilTextureResourceの生成(プロトタイプ)
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {
	// 1. 生成するDepthStencilTextureの Descの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	resourceDesc.Width = width;                  // Textureの幅
	resourceDesc.Height = height;                // Textureの高さ
	resourceDesc.MipLevels = 1;                  // mipmapの数 DepthStencilなので 1つでいい
	resourceDesc.DepthOrArraySize = 1;           // Textureの配列数 DepthStencilは 1つでいい
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT; // DepthStencilとして利用可能なフォーマット
	                                             // ※KamataEngineと合わせる

	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 2. 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VARM上に作る(GPUが使う領域にする)

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;      // 1.0f(最大値）でクリア（ 深度値の初期値は 1.0f）
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // Zバッファ形式、resourceと合わせる

	// 3. Resourceの生成
	ID3D12Resource* resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                  // Heapの設定
	    D3D12_HEAP_FLAG_NONE,             // Heapの特殊な設定 ★後で変更？
	    &resourceDesc,                    // Resourceの設定
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
	    &depthClearValue,                 // Clear最適値
	    IID_PPV_ARGS(&resource)           // 作成するResourceポインタへのポインタ
	);

	assert(SUCCEEDED(hr));

	return resource;
}

//-----------------------------------------------------------------------------

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE3D_05_オオノ_ユウキ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// Resource生成、Heap生成、View生成で再利用される変数の準備
	// Device取得
	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	// RenderTextureResource作成
	// 画面クリア色＃わかりやすいように赤とする
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);

	// DepthStencilTextureResource作成
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 1. DSV用のDescriptorHeapの作成
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   // HeapType
	dsvDescriptorHeapDesc.NumDescriptors = 1;                      // HeapTypeの個数
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // DSVはShaderで触らないとする

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//2.DSV用のViewの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // DepthStencilResourceのFormatと合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2DTexture



	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);
	//------------------------------------------------------------


	// ====================================================================
	// 1. SRV用の DescriptorHeapの作成
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     // SRV
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // PixelShader から見える
	srvDescriptorHeapDesc.NumDescriptors = 1;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLE、GPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();


	// 1. RTV用の DescriptorHeapを作成する
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // Descriptorの個数は 1

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 2.RTV用のViewを作成する
	device->CreateRenderTargetView(renderTextureResource, nullptr, rtvHandleCPU);

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width:{} height:{}", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// RootSignatureの作成-------------------------------------------------------------------------------
	// 構造体にデータを用意する
	RootSignature rs;
	rs.Create();

	// vertexShaderをコンパイルする------------------------------------------------------------------------------------------
	// 頂点シェーダの読み込みとコンパイル
	Shader vs;
	vs.LoadDxc(L"TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	// PixeShader------------------------------------------------------------------------------------------
	// ピクセルシェーダの読み込みとコンパイル
	Shader ps;
	ps.LoadDxc(L"TestPS.hlsl", L"ps_6_0");

	assert(ps.GetDxcBlob() != nullptr);

	// PapelineStateの生成
	PipelineState pipelineState;
	SetupPipelineState(&pipelineState, rs, vs, ps);

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	// 頂点データの準備
	VertexData vertices[] = {
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 左上
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}, // 右上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 左下
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}}, // 右下
	};

	// 頂点インデックスデータの準備
	uint16_t indices[] = {
	    0, 1, 2, 1, 3, 2,
	};

	// VertexBuffer(VertexResource,VertexResourceView)の生成
	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	// 頂点リソースにデータを書き込む--------------------------------------------------------------
	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	// IndexBuffer(IndexResource, IndexResourceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	// 頂点インデックスリソースにデータを書き込む
	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	// 頂点リソースにデータを書き込む ------------------------------------------------------------------
	// Vector4* vertexData = nullptr;
	// vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f}; // 左下
	// vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};   // 上
	// vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};  // 右下
	// 頂点リソースのマップを解除する
	// vb.Get()->Unmap(0, nullptr);

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break; // Updateがtrueを返したらループを抜ける
		}

		// 描画前処理
		dxCommon->PreDraw();
		// ここに描画処理を記述する

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());     // RootSignatureの設定
		commandList->SetPipelineState(pipelineState.Get());  // PSOの設定をする
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // VBVの設定をする
		commandList->IASetIndexBuffer(ib.GetView());         // IBVの設定をする
		// トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		// 描画終了
		dxCommon->PostDraw();
	}

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
