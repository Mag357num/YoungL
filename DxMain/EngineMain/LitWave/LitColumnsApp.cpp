#include "stdafx.h"
#include "LitColumnsApp.h"

#include "../Common/D3DApp.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"
#include "../Common/GeometryGenerator.h"

using Microsoft::WRL::ComPtr;

LitColumnsApp::LitColumnsApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
{

}

LitColumnsApp::~LitColumnsApp()
{
	if (md3dDevice.Get())
	{
		FlushCommandQueue();
	}
}

bool LitColumnsApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	return true;
}

void LitColumnsApp::BuildroogSignature()
{
	//CD3DX12_ROOT_PARAMETER rootParamater[3];
	//rootParamater[0].InitAsConstantBufferView(0);
	//rootParamater[1].InitAsConstantBufferView(1);
	//rootParamater[2].InitAsConstantBufferView(2);

	//CD3DX12_ROOT_SIGNATURE_DESC signatureDesc(3, rootParamater, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//ComPtr<ID3DBlob> serialzedBlob = nullptr;
	//ComPtr<ID3DBlob> errorBlob = nullptr;

	//HRESULT hr = D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialzedBlob.GetAddressOf(), errorBlob.GetAddressOf());

	//if (errorBlob != nullptr)
	//{
	//	::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	//}

	//ThrowIfFailed(hr);

	//ThrowIfFailed(md3dDevice->CreateRootSignature(0,
	//	serialzedBlob->GetBufferPointer(),
	//	serialzedBlob->GetBufferSize(),
	//	IID_PPV_ARGS(&mRootSignature)));

}

void LitColumnsApp::BuildShadersAndInputlayout()
{
	//const D3D_SHADER_MACRO alphaTestDefines[]=
	//{
	//	"ALPHA_TEST", "1"
	//	NULL, NULL
	//};

	//mShaders["standardPS"] = d3dUtil::CompileShader(L"shaders\\LitWav_Default.hlsl", nullptr, "VS", "vs_5_1");
	//mShaders["opaquePS"] = d3dUtil::CompileShader(L"shaders\\LitWav_Default.hlsl", nullptr, "PS", "ps_5_1");
	//
	//mInputLayout = {
	//	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	//	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	//	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	//};
}

void LitColumnsApp::BuildShapeGeometry()
{
	/*GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = boxVertexOffset + (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = boxIndexOffset + (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	SubmeshGeometry boxMesh;
	boxMesh.IndexCount = (UINT)box.Indices32.size();
	boxMesh.BaseVertexLocation = boxVertexOffset;
	boxMesh.StartIndexLocation = boxIndexOffset;

	SubmeshGeometry gridMesh;
	gridMesh.IndexCount = (UINT)grid.Indices32.size();
	gridMesh.BaseVertexLocation = gridVertexOffset;
	gridMesh.StartIndexLocation = gridIndexOffset;

	SubmeshGeometry sphereMesh;
	sphereMesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereMesh.BaseVertexLocation = sphereVertexOffset;
	sphereMesh.StartIndexLocation = sphereIndexOffset;

	SubmeshGeometry cylinderMesh;
	cylinderMesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderMesh.BaseVertexLocation = cylinderVertexOffset;
	cylinderMesh.StartIndexLocation = cylinderIndexOffset;

	auto totalVertexCount = box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	std::vector<Vertex_LitWave> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); i++, ++k)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = vertices.size() * sizeof(Vertex_LitWave);
	const UINT ibByteSize = indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	geo->VertextByteStride = sizeof(Vertex_LitWave);
	geo->VertextBufferByteSize = vbByteSize;
	geo->IndexBufferByteSize = ibByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_FLOAT;

	geo->DrawArgs["box"] = boxMesh;
	geo->DrawArgs["grid"] = gridMesh;
	geo->DrawArgs["sphere"] = sphereMesh;
	geo->DrawArgs["cylinder"] = cylinderMesh;

	mGeometries[geo->Name] = std::move(geo);*/
}

void LitColumnsApp::BuildSkullGeometry()
{

}