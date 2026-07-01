#include "IndexBuffer.h"
#include"KamataEngine.h"

#include<d3d12.h>
#include<cassert>

void IndexBuffer::Create(const UINT size, const UINT stride) {
	//strideの値によって、１つのインデックスのフォーマットを決める
	assert(stride == 2 || stride == 4); //2

}

ID3D12Resource* IndexBuffer::Get() { return nullptr; }

D3D12_INDEX_BUFFER_VIEW* IndexBuffer::GetView() { return nullptr; }
