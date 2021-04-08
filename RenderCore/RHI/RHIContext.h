#pragma once

class IRHIContext
{
public:
	IRHIContext(){}
	~IRHIContext(){}

	virtual void InitializeRHI(int InWidth, int InHeight){}

	virtual void Resize(int InWidth, int InHeight){}
	virtual void BeginDraw(){}
	virtual void SetViewport(){}

	virtual void TransilationResource(){}
	virtual void SetRenderTarget(){}

	virtual void SetConstantBuffer(){}
	virtual void SetShaderResourceView(){}

	virtual void SetVertexBuffer(){}
	virtual void SetIndexBuffer(){}
	virtual void SetPrimitiveTopology(){}

	virtual void DrawIndexedInstanced(){}

	virtual void EndDraw(){}

protected:

};

