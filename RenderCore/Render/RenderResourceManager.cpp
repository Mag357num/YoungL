#include "pch.h"
#include "RenderResourceManager.h"

FRenderResourceManager::FRenderResourceManager()
{

}

FRenderResourceManager::~FRenderResourceManager()
{
	if (MeshRenderResources.size() > 0)
	{
		for (auto It = MeshRenderResources.begin(); It != MeshRenderResources.end(); ++It)
		{
			delete It->second;
		}

		if (!MeshRenderResources.empty())
		{
		}
	}

	if (TextureRenderResource.size() > 0)
	{
		for (auto It = TextureRenderResource.begin(); It != TextureRenderResource.end(); ++It)
		{
			It->second.reset();
		}

		if (!TextureRenderResource.empty())
		{
		}
	}
}


FMeshRenderResource* FRenderResourceManager::CheckHasValidRenderResource(std::string InObjName)
{
	std::map<std::string, FMeshRenderResource*>::iterator Iter;
	Iter = MeshRenderResources.find(InObjName);
	if (Iter != MeshRenderResources.end())
	{
		return Iter->second;
	}


	return nullptr;
}

void FRenderResourceManager::CacheMeshRenderResource(std::string InObjName, FMeshRenderResource* InResource)
{
	MeshRenderResources.insert(std::make_pair(InObjName, InResource));
}

std::shared_ptr<FRHIColorResource> FRenderResourceManager::CheckHasValidTextureResource(std::string InObjName)
{
	std::map<std::string, std::shared_ptr<FRHIColorResource>>::iterator Iter;
	Iter = TextureRenderResource.find(InObjName);
	if (Iter != TextureRenderResource.end())
	{
		return Iter->second;
	}


	return nullptr;
}

void FRenderResourceManager::CacheTextureRenderResource(std::string InObjName, std::shared_ptr<FRHIColorResource> InResource)
{
	TextureRenderResource.insert(std::make_pair(InObjName, InResource));
}