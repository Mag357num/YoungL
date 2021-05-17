#pragma once
#include <map>

class FMeshRenderResource
{
public:
	FMeshRenderResource(){}
	~FMeshRenderResource(){
		if (VertexBuffer != nullptr)
		{
			VertexBuffer.reset();
		}

		if (IndexBuffer != nullptr)
		{
			IndexBuffer.reset();
		}
	}

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

	size_t GetVertexStrideSize() { return VertexStrideSize; }
	size_t GetVertexBufferSize() { return VertexBufferSize; }

	size_t GetIndexBufferSize() { return IndexBufferSize; }
	size_t GetIndexCount() { return IndexCount; }

	void SetVertexStrideSize(size_t InStride){ VertexStrideSize = InStride;}
	void SetVertexBufferSize(size_t InVBSize) { VertexBufferSize = InVBSize; }

	void SetIndexBufferSize(size_t InIBSize) { IndexBufferSize = InIBSize; }
	void SetIndexCount(size_t InCount) { IndexCount = InCount; }

private:
	std::shared_ptr<IRHIVertexBuffer> VertexBuffer;
	std::shared_ptr<IRHIIndexBuffer> IndexBuffer;

	size_t VertexStrideSize = 0;
	size_t VertexBufferSize = 0;

	size_t IndexBufferSize = 0;
	size_t IndexCount = 0;
};


class FRenderResourceManager
{
public:
	FRenderResourceManager();
	~FRenderResourceManager();

	FMeshRenderResource* CheckHasValidRenderResource(std::string InObjName);
	void CacheMeshRenderResource(std::string InObjName, FMeshRenderResource* InResource);


	std::shared_ptr<FRHIColorResource> CheckHasValidTextureResource(std::string InObjName);
	void CacheTextureRenderResource(std::string InObjName, std::shared_ptr<FRHIColorResource> InResource);
private:

	//cache mesh render resource
	std::map<std::string, FMeshRenderResource*> MeshRenderResources;

	//cache texture render resource
	std::map<std::string, std::shared_ptr<FRHIColorResource>> TextureRenderResource;
};
