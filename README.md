# YoungL Learn Project of DX12

## 多线程的部分是暂时的方案, 还需要整改(待整理)

## Hierarchy of RenderCore

- RenderCore
  - WinApp
  - GameCore
  - Render
  - RenderThread
  - RHI
    - RHIContext
    - D3D12
      - d3dx12.h
      - RHIResource_D3D12
      - RHIContext_D3D12
      - RHIVertextBuffer_D3D12
      - RHIIndexBuffer_D3D12
      - RHIShaderResource_D3D12
      - RHIConstantBuffer_D3D12
      - RHIResourceHandle_D3D12
      - RHIUploadBuffer_D3D12
      - RHIRenderingItem_D3D12
      - RHIGraphicsPipelineState_D3D12
  - Shaders
    - RenderCoreRS.hlsli
    - Lighting.hlsli
    - TestVS.hlsl
    - TestPS.hlsl
  - Models
    - ModelSave.Bin

## RenderCore: DX12 Course work
- Has a class named D3DExample
- version 0: basic process for drawing objects
- version 1: basic blinn-phong
- version 2: basic render core rhi framework


## EngineCore:Expermental framework for rhi
- version 0: application interface(WINAPI relative)
- version 1: GPU Resrouce && Buffer
- version 2: command context(not completed yet)
