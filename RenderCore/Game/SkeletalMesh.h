#pragma once

#include "../Render/RHI/RHIVertexBuffer.h"
#include "../Render/RHI/RHIIndexBuffer.h"

class USkeletalMesh
{
public:
	USkeletalMesh() {
		bHasValidRenderResource = false;
	}
	~USkeletalMesh() {
		if (VertexBuffer != nullptr)
		{
			VertexBuffer.reset();
		}

		if (IndexBuffer != nullptr)
		{
			IndexBuffer.reset();
		}
	}

	void SetAssetPath(std::wstring Inpath) { AssetPath = Inpath; }
	void SetGeometry(std::unique_ptr<FGeometry<FSkinVertex>> InGeometry)
	{
		Geometry = std::move(InGeometry);
	}

	FGeometry<FSkinVertex>* GetGeometry() {
		return Geometry.get();
	}

	std::wstring GetAssetPath() { return AssetPath; }

private:
	std::wstring AssetPath;
	std::unique_ptr<FGeometry<FSkinVertex>> Geometry;


	//render resource; only accessed by render thread
public:
	void SetVertexBuffer(std::shared_ptr<IRHIVertexBuffer> InVertexBuffer)
	{
		VertexBuffer = InVertexBuffer;
	}
	std::shared_ptr<IRHIVertexBuffer> GetVertexBuffer()
	{
		return VertexBuffer;
	}

	void SetIndexBuffer(std::shared_ptr<IRHIIndexBuffer> InIndexBuffer)
	{
		IndexBuffer = InIndexBuffer;
	}
	std::shared_ptr<IRHIIndexBuffer> GetIndexBuffer()
	{
		return IndexBuffer;
	}

	void SetHasValidRenderResource()
	{
		bHasValidRenderResource = true;
	}

	bool GetHasValidRenderResource()
	{
		return bHasValidRenderResource;
	}

private:
	bool bHasValidRenderResource;
	std::shared_ptr<IRHIVertexBuffer> VertexBuffer;
	std::shared_ptr<IRHIIndexBuffer> IndexBuffer;
};
