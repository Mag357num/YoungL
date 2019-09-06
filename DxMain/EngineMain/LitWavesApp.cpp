#include "stdafx.h"
#include "LitWavesApp.h"
#include "Common/d3dUtil.h"

using namespace DirectX;

LitWaveApp::LitWaveApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

LitWaveApp::~LitWaveApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

bool LitWaveApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), nullptr));
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mWaves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	//build start
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();
	//build end


	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	FlushCommandQueue();

	return true;
}

void LitWaveApp::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParamater[3];
	rootParamater[0].InitAsConstantBufferView(0);
	rootParamater[1].InitAsConstantBufferView(1);
	rootParamater[2].InitAsConstantBufferView(2);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(3, rootParamater, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedSig = nullptr;
	ComPtr<ID3DBlob> errorSerilized = nullptr;


	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedSig.GetAddressOf(), errorSerilized.GetAddressOf());

	if (errorSerilized!=nullptr)
	{
		::OutputDebugStringA((char*)errorSerilized->GetBufferPointer());
	}

	ThrowIfFailed(hr);

	md3dDevice->CreateRootSignature(0, serializedSig->GetBufferPointer(), serializedSig->GetBufferSize(), IID_PPV_ARGS(mRootSignature.GetAddressOf()));
}

void LitWaveApp::BuildShadersAndInputLayout()
{
	mShaders["standardVS"] = d3dUtil::CompileShader(L"shaders\\LitWave_Default.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"shaders\\LitWave_Default.hlsl", nullptr, "PS", "ps_5_0");
	
	mInputLayout = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void LitWaveApp::BuildLandGeometry()
{
	GeometryGenerator geoGenerate;
	GeometryGenerator::MeshData grid = geoGenerate.CreateGrid(160.0f, 160.0f, 50, 50);

	//extract the vertex element we are intrested and apply the height function to
	//each veretx. inaddition color the vertices based on their height so we have sandy looking beaches,
	//grassy low hills, and snow montain hills

	std::vector<Vertex_LitWave> vertices(grid.Vertices.size());
	for (size_t i=0;i<grid.Vertices.size();++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
	}

	UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex_LitWave);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";
	
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	memcpy(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	memcpy(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUpload);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUpload);

	geo->IndexBufferByteSize = ibByteSize;
	geo->VertextBufferByteSize = vbByteSize;
	geo->VertextByteStride = sizeof(Vertex_LitWave);
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;

	SubmeshGeometry subMesh;
	subMesh.StartIndexLocation = 0;
	subMesh.BaseVertexLocation = 0;
	subMesh.IndexCount = (UINT)indices.size();

	geo->DrawArgs["grid"] = subMesh;

	mGeometries["landGeo"] = std::move(geo);

}

float LitWaveApp::GetHillsHeight(float x, float z)const
{
	return 0.3f*(z * sinf(0.1f*x) + x * cosf(0.1f*z));
}

XMFLOAT3 LitWaveApp::GetHillsNormal(float x, float z)const
{
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitVector = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitVector);

	return n;
}

void LitWaveApp::BuildWavesGeometryBuffers()
{
	std::vector<uint16_t> indices(3 * mWaves->TriangleCount());
	assert(mWaves->VertexCount() < 0xffff0000);

	//iterator over each quad
	int m = mWaves->RowCount();
	int n = mWaves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m-1; i++)
	{
		for (int j = 0; j < n-1; j++)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = (UINT)mWaves->VertexCount() * sizeof(Vertex_LitWave);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	//set dynamicly
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	memcpy(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUpload);

	geo->VertextBufferByteSize = vbByteSize;
	geo->VertextByteStride = sizeof(Vertex_LitWave);
	geo->IndexBufferByteSize = ibByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;

	SubmeshGeometry subMesh;
	subMesh.IndexCount = (UINT)indices.size();
	subMesh.StartIndexLocation = 0;
	subMesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = subMesh;
	mGeometries["waterGeo"] = std::move(geo);
}

void LitWaveApp::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseAlbedo = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(0.0f, 0.2f, 0.6f, 1.0f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	mMaterials["grass"] = std::move(grass);
	mMaterials["water"] = std::move(water);
}

void LitWaveApp::BuildRenderItems()
{
	auto waveRitem = std::make_unique<RenderItem_LitWave>();
	waveRitem->World = MathHelper::Identity4x4();
	waveRitem->ObjCBIndex = 0;
	waveRitem->Mat = mMaterials["water"].get();
	waveRitem->Geo = mGeometries["waterGeo"].get();
	waveRitem->Primitivetype = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	waveRitem->IndexCount = waveRitem->Geo->DrawArgs["grid"].IndexCount;
	waveRitem->StartIndexLocation = waveRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	waveRitem->BaseVertexLocation = waveRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mWavesRitem = waveRitem.get();

	mRitemLayer[(int)RenderLayer_LitWav::Opaque].push_back(waveRitem.get());

	auto gridRitem = std::make_unique<RenderItem_LitWave>();
	gridRitem->World = MathHelper::Identity4x4();
	gridRitem->ObjCBIndex = 1;
	gridRitem->Mat = mMaterials["grass"].get();
	gridRitem->Geo = mGeometries["landGeo"].get();
	gridRitem->Primitivetype = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer_LitWav::Opaque].push_back(gridRitem.get());

	mAllRitems.push_back(std::move(waveRitem));
	mAllRitems.push_back(std::move(gridRitem));

}

void LitWaveApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources_LitWav; i++)
	{
		mFrameResources.push_back(std::make_unique<FrameResource_LitWave>(md3dDevice.Get(), 1, (UINT)mAllRitems.size(), (UINT)mMaterials.size(), mWaves->VertexCount()));
	}
}

void LitWaveApp::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc;

	ZeroMemory(&pipelineStateDesc, sizeof(pipelineStateDesc));
	pipelineStateDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size()};
	pipelineStateDesc.pRootSignature = mRootSignature.Get();
	pipelineStateDesc.VS = {
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	pipelineStateDesc.PS = {
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()

	};

	pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = mBackBufferFormat;
	pipelineStateDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	pipelineStateDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsasQuality - 1) : 0;
	pipelineStateDesc.DSVFormat = mDepthStencilFormat;


	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&mPSOs["opaque"])));
}

void LitWaveApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX p = XMMatrixPerspectiveFovLH(0.25f*MathHelper::PI, AspectRatio(), 1.0f, 1000.0f);
	
	XMStoreFloat4x4(&mProj, p);
}

void LitWaveApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void LitWaveApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) !=0)
	{
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;
		mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void LitWaveApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void LitWaveApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		mSunTheta -= 1.0f*dt;
	}
	if (GetAsyncKeyState(VK_RIGHT) &0x8000)
	{
		mSunTheta += 1.0f*dt;
	}
	if (GetAsyncKeyState(VK_UP)&0x8000)
	{
		mSunPhi -= 1.0f*dt;
	}
	if (GetAsyncKeyState(VK_DOWN)&0x8000)
	{
		mSunPhi += 1.0f*dt;
	}

	mSunPhi = MathHelper::Clamp(mSunPhi, 0.1f, XM_PIDIV2);
}

void LitWaveApp::Draw(const GameTimer& gt)
{
	auto cmdAllocator = mCurrFrameResource->CmdListAlloc;
	ThrowIfFailed(cmdAllocator->Reset());

	ThrowIfFailed(mCommandList->Reset(cmdAllocator.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto PassCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, PassCB->GetGPUVirtualAddress());

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer_LitWav::Opaque]);

	//indicate a state change
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = {mCommandList.Get()};
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	mCurrFrameResource->Fence = ++mCurrentFence;

	mCommandQueue->Signal(md3d12Fence.Get(), mCurrentFence);
}

void LitWaveApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem_LitWave*>& rItems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto ObjCB = mCurrFrameResource->ObjectCB->Resource();
	auto MatCB = mCurrFrameResource->MaterialCB->Resource();

	for (size_t i = 0; i < rItems.size(); i++)
	{
		auto ri = rItems[i];

		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetPrimitiveTopology(ri->Primitivetype);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = ObjCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = MatCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}

}

void LitWaveApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);

	UpdateCamera(gt);

	//cycle through the circular frame resource arry
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources_LitWav;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	if (mCurrFrameResource->Fence !=0 && md3d12Fence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(md3d12Fence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCB(gt);
	UpdateMaterialCB(gt);
	UpdateMainPassCB(gt);
	UpdateWaves(gt);
}

void LitWaveApp::UpdateCamera(const GameTimer& gt)
{
	mEyePos.x = mRadius * sinf(mPhi)*cosf(mTheta);
	mEyePos.z = mRadius * sinf(mPhi)*sinf(mTheta);
	mEyePos.y = mRadius * cosf(mPhi);

	//build view matrix
	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void LitWaveApp::UpdateObjectCB(const GameTimer& gt)
{
	auto objCB = mCurrFrameResource->ObjectCB.get();

	for (auto& e:mAllRitems )
	{
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstant;
			XMStoreFloat4x4(&objConstant.World, XMMatrixTranspose(world));

			objCB->CopyData(e->ObjCBIndex, objConstant);

			e->NumFramesDirty--;
		}
	}
}

void LitWaveApp::UpdateMaterialCB(const GameTimer& gt)
{
	auto matCB = mCurrFrameResource->MaterialCB.get();

	for (auto& e:mMaterials )
	{
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			
			matCB->CopyData(mat->MatCBIndex, matConstants);

			mat->NumFramesDirty--;
		}
	}
}

void LitWaveApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosw = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f/mClientWidth, 1.0f/mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = {0.25f, 0.25f, 0.35f, 1.0f};

	XMVECTOR lightDir = -MathHelper::SphericalToCartesian(1.0f, mSunTheta, mSunPhi);

	XMStoreFloat3(&mMainPassCB.Lights[0].Direction, lightDir);
	mMainPassCB.Lights[0].Strength = {1.0f, 1.0f, 0.9f};

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);

}

void LitWaveApp::UpdateWaves(const GameTimer& gt)
{
	static float t_base = 0.0f;
	if (mTimer.TotalTime() - t_base >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);
		mWaves->Disturb(i, j, r);
	}

	//
	mWaves->Update(gt.DeltaTime());

	auto currWavVB = mCurrFrameResource->WavesVB.get();
	for (int i = 0; i < mWaves->VertexCount(); i++)
	{
		Vertex_LitWave v;
		v.Pos = mWaves->Position(i);
		v.Normal = mWaves->Normal(i);

		currWavVB->CopyData(i, v);
	}

	mWavesRitem->Geo->VertexBufferGPU = currWavVB->Resource();

}