#include "stdafx.h"
#include "GeometryShaderApp.h"
#include "../Common/DDSTextureLoader.h"
#include "../Common/GeometryGenerator.h"

GeometryShaderApp::GeometryShaderApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

GeometryShaderApp::~GeometryShaderApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

bool GeometryShaderApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), nullptr));

	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mWaves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersInputAndLayout();

	BuildLandGeometry();
	BuildWaveGeometry();
	BuildBoxGeometry();
	BuildBillTreeGeometry();
	
	BuildMaterials();
	BuildRenderItems();

	BuildFrameResources();
	BuildPSOs();


	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
	FlushCommandQueue();

	return true;
}

void GeometryShaderApp::LoadTextures()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->FileName = L"Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), grassTex->FileName.c_str(), grassTex->Resource, grassTex->UploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->Name = "waterTex";
	waterTex->FileName = L"Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), waterTex->FileName.c_str(),
		waterTex->Resource, waterTex->UploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->Name = "fenceTex";
	fenceTex->FileName = L"Textures/WireFence.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), fenceTex->FileName.c_str(),
		fenceTex->Resource, fenceTex->UploadHeap));

	auto treeArrayTex = std::make_unique<Texture>();
	treeArrayTex->Name = "treeArrayTex";
	treeArrayTex->FileName = L"Textures/treeArray2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), treeArrayTex->FileName.c_str(), treeArrayTex->Resource, treeArrayTex->UploadHeap));

	mTextures[grassTex->Name] = std::move(grassTex);
	mTextures[waterTex->Name] = std::move(waterTex);
	mTextures[fenceTex->Name] = std::move(fenceTex);
	mTextures[treeArrayTex->Name] = std::move(treeArrayTex);
}

void GeometryShaderApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE descriptorTable;
	descriptorTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slotRootParamaters[4];
	slotRootParamaters[0].InitAsDescriptorTable(1, &descriptorTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParamaters[1].InitAsConstantBufferView(0);
	slotRootParamaters[2].InitAsConstantBufferView(1);
	slotRootParamaters[3].InitAsConstantBufferView(2);

	auto StaticSamples = GetStaticSamples();
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParamaters, (UINT)StaticSamples.size(), StaticSamples.data(), 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> serializedBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}

void GeometryShaderApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC desHeapDesc;
	desHeapDesc.NumDescriptors = 4;
	desHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	md3dDevice->CreateDescriptorHeap(&desHeapDesc, IID_PPV_ARGS(mSrvDescriptorHeap.GetAddressOf()));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = mTextures["grassTex"]->Resource;
	auto waterTex = mTextures["waterTex"]->Resource;
	auto fenceTex = mTextures["fenceTex"]->Resource;
	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1);
	srvDesc.Format = waterTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1);
	srvDesc.Format = fenceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(fenceTex.Get, &srvDesc, hDescriptor);

	hDescriptor.Offset(1);
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);
}

void GeometryShaderApp::BuildShadersInputAndLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["StandardVS"] = d3dUtil::CompileShader(L"shaders\\Blend_Default.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"shaders\\Blend_default.hlsl", defines, "PS", "ps_5_0");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"shaders\\Blend_default.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_0");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mStdInputLayout =
	{
		{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	mTreeSpriteInputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}

void GeometryShaderApp::BuildLandGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex_Blend> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUpload);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUpload);

	geo->VertextByteStride = sizeof(Vertex_Blend);
	geo->VertextBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);
}

void GeometryShaderApp::BuildWaveGeometry()
{
	std::vector<std::uint16_t> indices(3 * mWaves->TriangleCount()); // 3 indices per face
	assert(mWaves->VertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = mWaves->RowCount();
	int n = mWaves->ColumnCount();
	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
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

	UINT vbByteSize = mWaves->VertexCount() * sizeof(Vertex_Blend);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUpload);

	geo->VertextByteStride = sizeof(Vertex_Blend);
	geo->VertextBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["waterGeo"] = std::move(geo);
}

void GeometryShaderApp::BuildBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(8.0f, 8.0f, 8.0f, 3);

	std::vector<Vertex_Blend> Vetices(box.Vertices.size());
	for (int vIndex = 0; vIndex < box.Vertices.size(); vIndex++)
	{
		Vetices[vIndex].Pos = box.Vertices[vIndex].Position;
		Vetices[vIndex].Normal = box.Vertices[vIndex].Normal;
		Vetices[vIndex].TexC = box.Vertices[vIndex].TexC;
	}

	std::vector<std::uint16_t> Indices = box.GetIndices16();

	UINT VertexBufferSize = Vetices.size()* sizeof(Vertex_Blend);
	UINT IndexBufferSize = Indices.size()*sizeof(uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(VertexBufferSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), Vetices.data(), VertexBufferSize);
	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), Vetices.data(), VertexBufferSize, geo->VertexBufferUpload);

	ThrowIfFailed(D3DCreateBlob(IndexBufferSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), Indices.data(), IndexBufferSize);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), Indices.data(), IndexBufferSize, geo->IndexBufferUpload);

	geo->VertextByteStride = sizeof(Vertex_Blend);
	geo->VertextBufferByteSize = VertexBufferSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = IndexBufferSize;

	SubmeshGeometry subMesh;
	subMesh.BaseVertexLocation = 0;
	subMesh.StartIndexLocation = 0;
	subMesh.IndexCount = (UINT)Indices.size();

	geo->DrawArgs["box"] = subMesh;

	mGeometries["boxGeo"] = std::move(geo);
}

void GeometryShaderApp::BuildBillTreeGeometry()
{
	struct BillboardTreeVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int treeCount = 16;
	std::array<BillboardTreeVertex, 16> vertices;
	for (UINT i = 0; i < treeCount; i++)
	{
		float x = MathHelper::RandF(-45.0f, 45.0f);
		float z = MathHelper::RandF(-45.0f, 45.0f);
		float y = GetHillsHeight(x, z);

		y += 8.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);
	}

	std::array<std::uint16_t, 16> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(BillboardTreeVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUpload);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUpload);

	geo->VertextByteStride = sizeof(BillboardTreeVertex);
	geo->VertextBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	mGeometries["treeSpritesGeo"] = std::move(geo);
}

void GeometryShaderApp::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->MatCBIndex = 0;
	grass->DiffuseSrvHeapIndex = 0;
	grass->Roughness = 0.125f;

	// This is not a good water material definition, but we do not have all the rendering
	// tools we need (transparency, environment reflection), so we fake it for now.
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseSrvHeapIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->MatCBIndex = 2;
	wirefence->DiffuseSrvHeapIndex = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wirefence->Roughness = 0.25f;

	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 3;
	treeSprites->DiffuseSrvHeapIndex = 3;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;

	mMaterials["grass"] = std::move(grass);
	mMaterials["water"] = std::move(water);
	mMaterials["wirefence"] = std::move(wirefence);
	mMaterials["treeSprites"] = std::move(treeSprites);
}

void GeometryShaderApp::BuildPSOs()
{

	//pso for opaque
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueDesc;
	ZeroMemory(&opaqueDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaqueDesc.InputLayout = { mStdInputLayout.data(), mStdInputLayout.size() };
	opaqueDesc.pRootSignature = mRootSignature.Get();
	opaqueDesc.PS = {
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaqueDesc.VS = {
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaqueDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaqueDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaqueDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	opaqueDesc.RTVFormats[0] = mBackBufferFormat;
	opaqueDesc.DSVFormat = mDepthStencilFormat;

	opaqueDesc.SampleMask = UINT_MAX;
	opaqueDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaqueDesc.NumRenderTargets = 1;

	opaqueDesc.SampleDesc.Count = m4xMsaaState ? 4 : 0;
	opaqueDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsasQuality -1) : 0;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueDesc, IID_PPV_ARGS(mPSOs["opaque"].GetAddressOf())));

	//pso for transparent
	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentDesc = opaqueDesc;
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc;

	blendDesc.BlendEnable = true;
	
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;

	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

	blendDesc.LogicOpEnable = false;
	blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	
	transparentDesc.BlendState.RenderTarget[0] = blendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentDesc, IID_PPV_ARGS(mShaders["transparent"].GetAddressOf())));
	
	//pso for alphatest
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestDesc = opaqueDesc;
	alphaTestDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestPS"]->GetBufferSize()
	};
	alphaTestDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	md3dDevice->CreateGraphicsPipelineState(&alphaTestDesc, IID_PPV_ARGS(mPSOs["alphaTested"].GetAddressOf()));

	//pso for sprite tree
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpriteDesc = opaqueDesc;
	treeSpriteDesc.VS = 
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpriteDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpriteDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	treeSpriteDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	transparentDesc.InputLayout = { mTreeSpriteInputLayout.data(), mTreeSpriteInputLayout.size() };
	treeSpriteDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpriteDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));
}

void GeometryShaderApp::BuildRenderItems()
{
	auto wavesRitem = std::make_unique<RenderItem_Blend>();
	wavesRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&wavesRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem->ObjCBIndex = 0;
	wavesRitem->Mat = mMaterials["water"].get();
	wavesRitem->Geo = mGeometries["waterGeo"].get();
	wavesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mWavesRitem = wavesRitem.get();

	mRitemLayer[(int)RenderLayer_GeometryShader::Transparent].push_back(wavesRitem.get());

	auto gridRitem = std::make_unique<RenderItem_Blend>();
	gridRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRitem->ObjCBIndex = 1;
	gridRitem->Mat = mMaterials["grass"].get();
	gridRitem->Geo = mGeometries["landGeo"].get();
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer_GeometryShader::Opaque].push_back(gridRitem.get());

	auto boxRitem = std::make_unique<RenderItem_Blend>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	boxRitem->ObjCBIndex = 2;
	boxRitem->Mat = mMaterials["wirefence"].get();
	boxRitem->Geo = mGeometries["boxGeo"].get();
	boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer_GeometryShader::AlphaTested].push_back(boxRitem.get());

	auto treeSpritesRitem = std::make_unique<RenderItem_Blend>();
	treeSpritesRitem->World = MathHelper::Identity4x4();
	treeSpritesRitem->ObjCBIndex = 3;
	treeSpritesRitem->Mat = mMaterials["treeSprites"].get();
	treeSpritesRitem->Geo = mGeometries["treeSpritesGeo"].get();
	treeSpritesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	treeSpritesRitem->IndexCount = treeSpritesRitem->Geo->DrawArgs["points"].IndexCount;
	treeSpritesRitem->StartIndexLocation = treeSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
	treeSpritesRitem->BaseVertexLocation = treeSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer_GeometryShader::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());

	mAllRitems.push_back(std::move(wavesRitem));
	mAllRitems.push_back(std::move(gridRitem));
	mAllRitems.push_back(std::move(boxRitem));
	mAllRitems.push_back(std::move(treeSpritesRitem));
}

void GeometryShaderApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources_Blend; i++)
	{
		mFrameResources.push_back(std::make_unique<FrameResource_Blend>(md3dDevice.Get(), 
			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size(), mWaves->VertexCount()));
	}
}

float GeometryShaderApp::GetHillsHeight(float x, float z)const
{
	return 0.3f*(z*sinf(0.1f*x) + x * cosf(0.1f*z));
}

XMFLOAT3 GeometryShaderApp::GetHillsNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> GeometryShaderApp::GetStaticSamples()
{
	const CD3DX12_STATIC_SAMPLER_DESC PointWrap(
	0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	const CD3DX12_STATIC_SAMPLER_DESC PointClamp(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);

	const CD3DX12_STATIC_SAMPLER_DESC LinearWrap(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	const CD3DX12_STATIC_SAMPLER_DESC LinearClamp(
		0,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);

	const CD3DX12_STATIC_SAMPLER_DESC AnisotropicWrap(
		0,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	const CD3DX12_STATIC_SAMPLER_DESC AnisotropicClamp(
		0,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);

	return {
	PointWrap, PointClamp,
	LinearWrap, LinearClamp,
	AnisotropicWrap, AnisotropicClamp
	};
}

void GeometryShaderApp::Draw(const GameTimer& gt)
{
	auto cmdAlloc = mCurrFrameResource->CmdListAlloc;
	ThrowIfFailed(cmdAlloc->Reset());

	ThrowIfFailed(mCommandList->Reset(cmdAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), 
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&mMainPassCB.FogColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
}

void GeometryShaderApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderLayer_Blend *>& rItems)
{

}