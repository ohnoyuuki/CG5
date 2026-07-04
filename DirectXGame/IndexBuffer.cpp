#include "IndexBuffer.h"
#include"KamataEngine.h"

#include<d3d12.h>
#include<cassert>

using namespace KamataEngine;	

void IndexBuffer::Create(const UINT size, const UINT stride) {
	//strideの値によって、１つのインデックスのフォーマットを決める
	assert(stride == 2 || stride == 4); // 2byte or 4byte　のみ受け付ける
	DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	//クラス内でdxCommonを利用するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ

	// 頂点リソースの設定


	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // バッファ
	vertexResourceDesc.Width = size;                                // リソースのサイズ、引数sizeで受け取った値
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
}

ID3D12Resource* IndexBuffer::Get() { return nullptr; }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return nullptr; }
