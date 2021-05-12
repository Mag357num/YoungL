#pragma once

enum EINPUT_CLASSIFICATION
{
	INPUT_CLASSIFICATION_PER_VERTEX_DATA = 0,
	INPUT_CLASSIFICATION_PER_INSTANCE_DATA = 1
};

class FRHIShaderInputElement
{
public:
	FRHIShaderInputElement(std::string InSemanticName,
		UINT InSemanticIndex,
		EPixelBufferFormat InFormat,
		UINT InInputSlot,
		UINT InAlignedByteOffset,
		EINPUT_CLASSIFICATION InInputSlotClass,
		UINT InInstanceDataStepRate)
		:SemanticName(InSemanticName),
		SemanticIndex(InSemanticIndex),
		Format(InFormat),
		InputSlot(InInputSlot),
		AlignedByteOffset(InAlignedByteOffset),
		InputSlotClass(InInputSlotClass),
		InstanceDataStepRate(InInstanceDataStepRate)

	{}


	~FRHIShaderInputElement(){}

	std::string GetSemanticName(){return SemanticName;}
	UINT GetSemanticIndex(){return SemanticIndex;}
	EPixelBufferFormat GetFormat(){return Format;}
	UINT GetInputSlot(){return InputSlot;}
	UINT GetAlignedByteOffset() {
		return AlignedByteOffset;
		}

	EINPUT_CLASSIFICATION GetInputSlotClass() {
		return InputSlotClass;
		}

	UINT GetInstanceDataStepRate() {
		return InstanceDataStepRate;
	}

	std::string SemanticName;
private:
	UINT SemanticIndex;
	EPixelBufferFormat Format;
	UINT InputSlot;
	UINT AlignedByteOffset;
	EINPUT_CLASSIFICATION InputSlotClass;
	UINT InstanceDataStepRate;
};

