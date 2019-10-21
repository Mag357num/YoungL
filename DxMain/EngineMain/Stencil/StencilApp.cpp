#include "stdafx.h"
#include "StencilApp.h"
#include "../Common/d3dUtil.h"
#include "../Common/d3dx12.h"
#include "../Common/DDSTextureLoader.h"

StencilApp::StencilApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

StencilApp::~StencilApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

bool StencilApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	ThrowIfFailed(mCommandList->Reset(mCommandAllocate.Get(), nullptr));
	mCbvSrvHeapDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputlayout();
	BuildRoomGeometry();
	BuildSkullGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResource();
	BuildPSOs();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void StencilApp::LoadTextures()
{
	auto bricksTexture = std::make_unique<Texture>();
	bricksTexture->Name = "bricksTex";
	bricksTexture->FileName = L"Textures/bricks3.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), bricksTexture->FileName.c_str(),
		bricksTexture->Resource, bricksTexture->UploadHeap));

	auto checkBoardTexture = std::make_unique<Texture>();
	checkBoardTexture->Name = "checkboardTex";
	checkBoardTexture->FileName = L"Textures/checkboard.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), checkBoardTexture->FileName.c_str(),
		checkBoardTexture->Resource, checkBoardTexture->UploadHeap));

	auto iceTexture = std::make_unique<Texture>();
	iceTexture->Name = "iceTex";
	iceTexture->FileName = L"Textures/ice.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), iceTexture->FileName.c_str(),
		iceTexture->Resource, iceTexture->UploadHeap));

	auto white1x1Texture = std::make_unique<Texture>();
	white1x1Texture->Name = "white1x1Tex";
	white1x1Texture->FileName = L"Textures/white1x1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), white1x1Texture->FileName.c_str(),
		white1x1Texture->Resource, white1x1Texture->UploadHeap));

	mTextures[bricksTexture->Name] = std::move(bricksTexture);
	mTextures[checkBoardTexture->Name] = std::move(checkBoardTexture);
	mTextures[iceTexture->Name] = std::move(iceTexture);
	mTextures[white1x1Texture->Name] = std::move(white1x1Texture);
	
}

void StencilApp::BuildRootSignature()
{
	
	CD3DX12_DESCRIPTOR_RANGE descriptorTable;
	descriptorTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER slootRootSigPara[4];
	slootRootSigPara[0].InitAsDescriptorTable(1, &descriptorTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slootRootSigPara[1].InitAsConstantBufferView(0);
	slootRootSigPara[2].InitAsConstantBufferView(1);
	slootRootSigPara[3].InitAsConstantBufferView(2);

	auto staticSamples = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC sigDesc(4, slootRootSigPara, (UINT)staticSamples.size(), staticSamples.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> serializedBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedBlob.GetAddressOf(), errorBlob.GetAddressOf());
	if (errorBlob!=nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedBlob->GetBufferPointer(), serializedBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

}

void StencilApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 4;
	heapDesc.NodeMask = 0;

	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mSrvDesctiptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mSrvDesctiptorHeap->GetCPUDescriptorHandleForHeapStart());
	
	auto bricksTex = mTextures["bricksTex"]->Resource;
	auto checkboardTex = mTextures["checkboardTex"]->Resource;
	auto iceTex = mTextures["iceTex"]->Resource;
	auto white1x1Tex = mTextures["white1x1Tex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvViewDesc = {};
	srvViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvViewDesc.Texture2D.MostDetailedMip = 0;
	srvViewDesc.Texture2D.MipLevels = -1;
	srvViewDesc.Format = bricksTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvViewDesc, handle);

	handle.Offset(1, mCbvSrvHeapDescriptorSize);

	srvViewDesc.Format = checkboardTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(checkboardTex.Get(), &srvViewDesc, handle);

	handle.Offset(1, mCbvSrvHeapDescriptorSize);

	srvViewDesc.Format = iceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(iceTex.Get(), &srvViewDesc, handle);

	handle.Offset(1, mCbvSrvHeapDescriptorSize);

	srvViewDesc.Format = white1x1Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(white1x1Tex.Get(), &srvViewDesc, handle);
}

void StencilApp::BuildShadersAndInputlayout()
{
	const D3D_SHADER_MACRO defines[]=
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[]=
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"shaders\\Stencil_Default.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"shaders\\Stencil_Default.hlsl", defines, "PS", "ps_5_0");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"shaders\\Stencil_Default.hlsl", alphaTestDefines, "PS", "ps_5_0");

	mInputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}

void StencilApp::BuildRoomGeometry()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/

	std::array<Vertex_Stencil, 20> vertices =
	{
		// Floor: Observe we tile texture coordinates.
		Vertex_Stencil(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0 
		Vertex_Stencil(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
		Vertex_Stencil(7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
		Vertex_Stencil(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		Vertex_Stencil(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
		Vertex_Stencil(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex_Stencil(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
		Vertex_Stencil(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

		Vertex_Stencil(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8 
		Vertex_Stencil(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex_Stencil(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
		Vertex_Stencil(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

		Vertex_Stencil(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
		Vertex_Stencil(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex_Stencil(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
		Vertex_Stencil(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

		// Mirror
		Vertex_Stencil(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 16
		Vertex_Stencil(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex_Stencil(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		Vertex_Stencil(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)
	};

	std::array<std::int16_t, 30> indices =
	{
		// Floor
		0, 1, 2,
		0, 2, 3,

		// Walls
		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		// Mirror
		16, 17, 18,
		16, 18, 19
	};

	SubmeshGeometry floorSubmesh;
	floorSubmesh.IndexCount = 6;
	floorSubmesh.StartIndexLocation = 0;
	floorSubmesh.BaseVertexLocation = 0;

	SubmeshGeometry wallSubmesh;
	wallSubmesh.IndexCount = 18;
	wallSubmesh.StartIndexLocation = 6;
	wallSubmesh.BaseVertexLocation = 0;

	SubmeshGeometry mirrorSubmesh;
	mirrorSubmesh.IndexCount = 6;
	mirrorSubmesh.StartIndexLocation = 24;
	mirrorSubmesh.BaseVertexLocation = 0;

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex_Stencil);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "roomGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUpload);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUpload);

	geo->VertextByteStride = sizeof(Vertex_Stencil);
	geo->VertextBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["floor"] = floorSubmesh;
	geo->DrawArgs["wall"] = wallSubmesh;
	geo->DrawArgs["mirror"] = mirrorSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void StencilApp::BuildSkullGeometry()
{
	std::ifstream fin("Models/skull_stencil.txt");
	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex_Stencil> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		// Model does not have texture coordinates, so just zero them out.
		vertices[i].TexC = { 0.0f, 0.0f };
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	const UINT vbBytesize = (UINT)vertices.size() * sizeof(Vertex_Stencil);
	const UINT ibBytesize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skullGeo";

	ThrowIfFailed(D3DCreateBlob(vbBytesize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbBytesize);

	ThrowIfFailed(D3DCreateBlob(ibBytesize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibBytesize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices.data(), vbBytesize, geo->VertexBufferUpload);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), ibBytesize, geo->IndexBufferUpload);

	geo->VertextByteStride = sizeof(Vertex_Stencil);
	geo->VertextBufferByteSize = ibBytesize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->VertextBufferByteSize = vbBytesize;

	SubmeshGeometry submesh;
	submesh.StartIndexLocation = 0;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["skull"] = submesh;
	mGeometries[geo->Name] = std::move(geo);

}

void StencilApp::BuildMaterials()
{
	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 0;
	bricks->DiffuseSrvHeapIndex = 0;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	bricks->Roughness = 0.25f;

	auto checkertile = std::make_unique<Material>();
	checkertile->Name = "checkertile";
	checkertile->MatCBIndex = 1;
	checkertile->DiffuseSrvHeapIndex = 1;
	checkertile->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	checkertile->FresnelR0 = XMFLOAT3(0.07f, 0.07f, 0.07f);
	checkertile->Roughness = 0.3f;

	auto icemirror = std::make_unique<Material>();
	icemirror->Name = "icemirror";
	icemirror->MatCBIndex = 2;
	icemirror->DiffuseSrvHeapIndex = 2;
	icemirror->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	icemirror->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	icemirror->Roughness = 0.5f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 3;
	skullMat->DiffuseSrvHeapIndex = 3;
	skullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.3f;

	auto shadowMat = std::make_unique<Material>();
	shadowMat->Name = "shadowMat";
	shadowMat->MatCBIndex = 4;
	shadowMat->DiffuseSrvHeapIndex = 3;
	shadowMat->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	shadowMat->FresnelR0 = XMFLOAT3(0.001f, 0.001f, 0.001f);
	shadowMat->Roughness = 0.0f;

	mMaterials["bricks"] = std::move(bricks);
	mMaterials["checkertile"] = std::move(checkertile);
	mMaterials["icemirror"] = std::move(icemirror);
	mMaterials["skullMat"] = std::move(skullMat);
	mMaterials["shadowMat"] = std::move(shadowMat);
}

void StencilApp::BuildRenderItems()
{
	auto FloorRitem = std::make_unique<RenderItem_Stencil>();
	FloorRitem->World = MathHelper::Identity4x4();
	FloorRitem->TexTransform = MathHelper::Identity4x4();
	FloorRitem->ObjCBIndex = 0;
	FloorRitem->Mat = mMaterials["checkertile"].get();
	FloorRitem->Geo = mGeometries["roomGeo"].get();
	FloorRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FloorRitem->IndexCount = FloorRitem->Geo->DrawArgs["floor"].IndexCount;
	FloorRitem->StartIndexLocation = FloorRitem->Geo->DrawArgs["floor"].StartIndexLocation;
	FloorRitem->BaseVertexLocation = FloorRitem->Geo->DrawArgs["floor"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer_Stencil::Opaque].push_back(FloorRitem.get());

	auto WallRitem = std::make_unique<RenderItem_Stencil>();
	WallRitem->World = MathHelper::Identity4x4();
	WallRitem->TexTransform = MathHelper::Identity4x4();
	WallRitem->ObjCBIndex = 1;
	WallRitem->Mat = mMaterials["bricks"].get();
	WallRitem->Geo = mGeometries["roomGeo"].get();
	WallRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	WallRitem->IndexCount = WallRitem->Geo->DrawArgs["wall"].IndexCount;
	WallRitem->StartIndexLocation = WallRitem->Geo->DrawArgs["wall"].StartIndexLocation;
	WallRitem->BaseVertexLocation = WallRitem->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer_Stencil::Opaque].push_back(WallRitem.get());

	auto skullRitem = std::make_unique<RenderItem_Stencil>();
	skullRitem->World = MathHelper::Identity4x4();
	skullRitem->TexTransform = MathHelper::Identity4x4();
	skullRitem->ObjCBIndex = 2;
	skullRitem->Mat = mMaterials["skullMat"].get();
	skullRitem->Geo = mGeometries["skullGeo"].get();
	skullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skullRitem->IndexCount = skullRitem->Geo->DrawArgs["skull"].IndexCount;
	skullRitem->StartIndexLocation = skullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
	skullRitem->BaseVertexLocation = skullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;
	mSkullRitem = skullRitem.get();
	mRitemLayer[(int)RenderLayer_Stencil::Opaque].push_back(skullRitem.get());

	auto reflectedSkullRitem = std::make_unique<RenderItem_Stencil>();
	*reflectedSkullRitem = *skullRitem;
	reflectedSkullRitem->ObjCBIndex = 3;
	mReflectedSkullRitem = reflectedSkullRitem.get();
	mRitemLayer[(int)RenderLayer_Stencil::Reflected].push_back(reflectedSkullRitem.get());

	auto shadowedSkullRitem = std::make_unique<RenderItem_Stencil>();
	*shadowedSkullRitem = *skullRitem;
	shadowedSkullRitem->ObjCBIndex = 4;
	shadowedSkullRitem->Mat = mMaterials["shadowMat"].get();
	mShadowedSkillRitem = shadowedSkullRitem.get();
	mRitemLayer[(UINT)RenderLayer_Stencil::Shadow].push_back(shadowedSkullRitem.get());

	auto mirrorRitem = std::make_unique<RenderItem_Stencil>();
	mirrorRitem->World = MathHelper::Identity4x4();
	mirrorRitem->TexTransform = MathHelper::Identity4x4();
	mirrorRitem->ObjCBIndex = 5;
	mirrorRitem->Geo = mGeometries["roomGeo"].get();
	mirrorRitem->Mat = mMaterials["icemirror"].get();
	mirrorRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mirrorRitem->IndexCount = mirrorRitem->Geo->DrawArgs["mirror"].IndexCount;
	mirrorRitem->BaseVertexLocation = mirrorRitem->Geo->DrawArgs["mirror"].BaseVertexLocation;
	mirrorRitem->StartIndexLocation = mirrorRitem->Geo->DrawArgs["mirror"].StartIndexLocation;

	mRitemLayer[(int)RenderLayer_Stencil::Mirrors].push_back(mirrorRitem.get());
	mRitemLayer[(int)RenderLayer_Stencil::Transparent].push_back(mirrorRitem.get());

	mAllRitems.push_back(std::move(FloorRitem));
	mAllRitems.push_back(std::move(WallRitem));
	mAllRitems.push_back(std::move(skullRitem));
	mAllRitems.push_back(std::move(reflectedSkullRitem));
	mAllRitems.push_back(std::move(shadowedSkullRitem));
	mAllRitems.push_back(std::move(mirrorRitem));
}

void StencilApp::BuildFrameResource()
{
	for (int i = 0; i < gNumFrameResources; i++)
	{
		mFrameResources.push_back(std::make_unique<FrameResource_Stencil>(md3dDevice.Get(), 2, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
	}
}

void StencilApp::BuildPSOs()
{
	//pso for opaque
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = {mInputLayout.data(), (UINT)mInputLayout.size()};
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsasQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"]));

	//pso for transparent
	D3D12_GRAPHICS_PIPELINE_STATE_DESC TransparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC BlendRTStateDesc;
	BlendRTStateDesc.BlendEnable = true;
	BlendRTStateDesc.LogicOpEnable = false;
	BlendRTStateDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	BlendRTStateDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	BlendRTStateDesc.BlendOp = D3D12_BLEND_OP_ADD;

	BlendRTStateDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendRTStateDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	BlendRTStateDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	BlendRTStateDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	BlendRTStateDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	TransparentPsoDesc.BlendState.RenderTarget[0] = BlendRTStateDesc;
	md3dDevice->CreateGraphicsPipelineState(&TransparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"]));

	//pso for mirror
	D3D12_GRAPHICS_PIPELINE_STATE_DESC mirrorPsoDesc;
	mirrorPsoDesc = opaquePsoDesc;

	CD3DX12_BLEND_DESC mirrorBlendDesc(D3D12_DEFAULT);
	mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	CD3DX12_DEPTH_STENCIL_DESC mirrorStencilDesc(D3D12_DEFAULT);
	mirrorStencilDesc.DepthEnable = true;
	mirrorStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorStencilDesc.StencilEnable = true;
	mirrorStencilDesc.StencilReadMask = 0xff;
	mirrorStencilDesc.StencilWriteMask = 0xff;

	mirrorStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mirrorStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;

	mirrorStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mirrorStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;

	mirrorPsoDesc.BlendState = mirrorBlendDesc;
	mirrorPsoDesc.DepthStencilState = mirrorStencilDesc;
	md3dDevice->CreateGraphicsPipelineState(&mirrorPsoDesc, IID_PPV_ARGS(&mPSOs["markStencilMirrors"]));

	//pso for reflection
	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectionPsoDesc;
	reflectionPsoDesc = opaquePsoDesc;

	CD3DX12_DEPTH_STENCIL_DESC reflectionStencilDesc(D3D12_DEFAULT);
	reflectionStencilDesc.DepthEnable = true;
	reflectionStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionStencilDesc.StencilEnable = true;
	reflectionStencilDesc.StencilReadMask = 0xff;
	reflectionStencilDesc.StencilWriteMask = 0xff;

	reflectionStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	reflectionStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

	reflectionStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	reflectionStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;

	reflectionPsoDesc.DepthStencilState = reflectionStencilDesc;
	reflectionPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectionPsoDesc.RasterizerState.FrontCounterClockwise = true;

	md3dDevice->CreateGraphicsPipelineState(&reflectionPsoDesc, IID_PPV_ARGS(&mPSOs["drawStencilReflections"]));

	//pso for shadow
	// We are going to draw shadows with transparency, so base it off the transparency description.
	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = TransparentPsoDesc;
	shadowPsoDesc.DepthStencilState = shadowDSS;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&mPSOs["shadow"])));

	
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> StencilApp::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
	0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

void StencilApp::Draw(const GameTimer& gt)
{
	auto cmdAllocator = mCurrentFrameResource->CmdListAlloc;
	ThrowIfFailed(cmdAllocator->Reset());
	mCommandList->Reset(cmdAllocator.Get(), mPSOs["opaque"].Get());

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&mMainPassCB.FogColor, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDesctiptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants_Stencil));

	// Draw opaque items--floors, walls, skull.
	auto passCB = mCurrentFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer_Stencil::Opaque]);

	// Mark the visible mirror pixels in the stencil buffer with the value 1
	//mCommandList->OMSetStencilRef(1);
	//mCommandList->SetPipelineState(mPSOs["markStencilMirrors"].Get());
	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer_Stencil::Mirrors]);

	//// Draw the reflection into the mirror only (only for pixels where the stencil buffer is 1).
	//// Note that we must supply a different per-pass constant buffer--one with the lights reflected.
	//mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress() + 1 * passCBByteSize);
	//mCommandList->SetPipelineState(mPSOs["drawStencilReflections"].Get());
	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer_Stencil::Reflected]);

	//// Restore main pass constants and stencil ref.
	//mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	//mCommandList->OMSetStencilRef(0);

	//// Draw mirror with transparency so reflection blends through.
	//mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer_Stencil::Transparent]);

	//// Draw shadows
	//mCommandList->SetPipelineState(mPSOs["shadow"].Get());
	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer_Stencil::Shadow]);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrentFrameResource->Fence = ++mCurrentFence;

	// Notify the fence when the GPU completes commands up to this fence point.
	mCommandQueue->Signal(md3d12Fence.Get(), mCurrentFence);
}

void StencilApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::vector<RenderItem_Stencil *>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstans_Stencil));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrentFrameResource->ObjectCB->Resource();
	auto matCB = mCurrentFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDesctiptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}


void StencilApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::PI, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void StencilApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	UpdateCamera(gt);

	// Cycle through the circular frame resource array.
	mCurrentFrameResourceIndex = (mCurrentFrameResourceIndex + 1) % gNumFrameResources;
	mCurrentFrameResource = mFrameResources[mCurrentFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrentFrameResource->Fence != 0 && md3d12Fence->GetCompletedValue() < mCurrentFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(md3d12Fence->SetEventOnCompletion(mCurrentFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
	UpdateReflectedPassCB(gt);
}

void StencilApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void StencilApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void StencilApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadiu += dx - dy;

		// Restrict the radius.
		mRadiu = MathHelper::Clamp(mRadiu, 5.0f, 150.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void StencilApp::OnKeyboardInput(const GameTimer& gt)
{
	//
	// Allow user to move skull.
	//

	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('A') & 0x8000)
		mSkullTranslation.x -= 1.0f*dt;

	if (GetAsyncKeyState('D') & 0x8000)
		mSkullTranslation.x += 1.0f*dt;

	if (GetAsyncKeyState('W') & 0x8000)
		mSkullTranslation.y += 1.0f*dt;

	if (GetAsyncKeyState('S') & 0x8000)
		mSkullTranslation.y -= 1.0f*dt;

	// Don't let user move below ground plane.
	mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, 0.0f);

	// Update the new world matrix.
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::PI);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
	XMMATRIX skullWorld = skullRotate * skullScale*skullOffset;
	XMStoreFloat4x4(&mSkullRitem->World, skullWorld);

	// Update reflection world matrix.
	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&mReflectedSkullRitem->World, skullWorld * R);

	// Update shadow world matrix.
	XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
	XMVECTOR toMainLight = -XMLoadFloat3(&mMainPassCB.Lights[0].Direction);
	XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
	XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);
	XMStoreFloat4x4(&mShadowedSkillRitem->World, skullWorld * S * shadowOffsetY);

	mSkullRitem->NumFramesDirty = gNumFrameResources;
	mReflectedSkullRitem->NumFramesDirty = gNumFrameResources;
	mShadowedSkillRitem->NumFramesDirty = gNumFrameResources;
}

void StencilApp::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	mEyePos.x = mRadiu * sinf(mPhi)*cosf(mTheta);
	mEyePos.z = mRadiu * sinf(mPhi)*sinf(mTheta);
	mEyePos.y = mRadiu * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);
}

void StencilApp::AnimateMaterials(const GameTimer& gt)
{

}

void StencilApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrentFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstans_Stencil objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void StencilApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrentFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void StencilApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	// Main pass stored in index 2
	auto currPassCB = mCurrentFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void StencilApp::UpdateReflectedPassCB(const GameTimer& gt)
{
	mReflectedPassCB = mMainPassCB;

	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);

	// Reflect the lighting.
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&mMainPassCB.Lights[i].Direction);
		XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mReflectedPassCB.Lights[i].Direction, reflectedLightDir);
	}

	// Reflected pass stored in index 1
	auto currPassCB = mCurrentFrameResource->PassCB.get();
	currPassCB->CopyData(1, mReflectedPassCB);
}