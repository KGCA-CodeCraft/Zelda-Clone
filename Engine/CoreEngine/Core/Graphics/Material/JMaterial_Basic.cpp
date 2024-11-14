﻿#include "JMaterial_Basic.h"

#include "Core/Graphics/ShaderStructs.h"
#include "Core/Graphics/Shader/JShader.h"
#include "Core/Interface/MManagerInterface.h"

JMaterial_Basic::JMaterial_Basic(JTextView InName)
	: JMaterial(InName)
{
	JMaterial_Basic::InitializeParams();
}

void JMaterial_Basic::BindMaterialPipeline(ID3D11DeviceContext*          InDeviceContext,
										   const JArray<FMaterialParam>& InInstanceParams)
{
	JMaterial::BindMaterialPipeline(InDeviceContext, InInstanceParams);

	Renderer.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	int32_t slots[2] = {0, 1};
	Renderer.SetSamplerState(ESamplerState::LinearWrap, slots, 2);
	Renderer.SetRasterState(IManager.GUIManager->IsRenderWireFrame()
											   ? ERasterState::WireFrame
											   : ERasterState::CCW);
	Renderer.SetBlendState(EBlendState::AlphaBlend);
	Renderer.SetDepthStencilState(EDepthStencilState::DepthDefault);

	// 텍스처맵 슬롯 바인딩
	for (int32_t i = 0; i < InInstanceParams.size(); ++i)
	{
		const FMaterialParam& param = InInstanceParams[i];

		mShader->UpdateConstantData(InDeviceContext,
									CBuffer::NAME_CONSTANT_BUFFER_MATERIAL,
									param.Name,
									&param.Float4Value);

		param.BindMaterialParam(InDeviceContext, i);
	}
}

void JMaterial_Basic::InitializeParams()
{
	if (mMaterialParams.empty())
	{
		mMaterialParams.push_back(FMaterialParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_DIFFUSE,
												 EMaterialParamValue::Float4,
												 &FVector::ZeroVector,
												 true));
		mMaterialParams.push_back(FMaterialParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_NORMAL,
												 EMaterialParamValue::Float4,
												 &FVector::ZeroVector,
												 true));

		mMaterialParams.push_back(FMaterialParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_AO,
												 EMaterialParamValue::Float2,
												 &FVector::ZeroVector,
												 true));
		mMaterialParams.push_back(FMaterialParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_ROUGHNESS,
												 EMaterialParamValue::Float2,
												 &FVector::ZeroVector,
												 true));
		mMaterialParams.push_back(FMaterialParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_METALLIC,
												 EMaterialParamValue::Float2,
												 &FVector::ZeroVector,
												 true));
		mMaterialParams.push_back(FMaterialParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_USAGE_FLAG,
												 EMaterialParamValue::Integer,
												 &FVector::ZeroVector,
												 true));
	}

}