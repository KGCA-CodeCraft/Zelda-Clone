﻿#include "GUI_Editor_Material.h"

#include "Core/Entity/Camera/JCameraComponent.h"
#include "Core/Graphics/Material/Instance/JMaterialInstance.h"
#include "Core/Graphics/Mesh/JMeshObject.h"
#include "Core/Graphics/Mesh/MMeshManager.h"
#include "Core/Graphics/Texture/MTextureManager.h"
#include "Core/Interface/JWorld.h"
#include "Core/Utils/Logger.h"
#include "GUI/Popup/GUI_Popup_FileBrowser.h"


GUI_Editor_Material::GUI_Editor_Material(const JText& InTitle)
	: GUI_Editor_Base(InTitle),
	  bOpenFileBrowser(false),
	  bOpenFolder(false)
{
	mMaterialToEdit = GetWorld.MaterialInstanceManager->Load(InTitle);

	SetMeshObject(Path_Mesh_Sphere);
}

void GUI_Editor_Material::Render()
{
	using Super = GUI_Editor_Base;

	Super::Render();

	// 변경사항 적용 Draw
	mPreviewMeshObject->UpdateInstance_Transform();
	mPreviewMeshObject->Draw();
}

void GUI_Editor_Material::SetMeshObject(JTextView InMeshPath)
{
	// 3. 구체 메시를 생성 (굳이 구체가 아니어도 상관없음, 대부분의 엔진에서는 구체를 사용)
	mPreviewMeshObject = GetWorld.MeshManager->Clone(InMeshPath.data());
	mPreviewMeshObject->SetMaterialInstance(mMaterialToEdit);
}

void GUI_Editor_Material::HandleIntegerType(FMaterialParam& MaterialParam)
{
	// if (MaterialParam.Key == CBuffer::HASH_CONSTANT_VARIABLE_MATERIAL_USAGE_FLAG)
	// {
	// 	uint32_t flag = static_cast<uint32_t>(MaterialParam.IntegerValue);
	// 	
	// 	for (int32_t i = 1; i < ARRAYSIZE(CBuffer::TextureUsageString); ++i)
	// 	{
	// 		bool bSelected = flag & (1 << (i - 1));
	// 		ImGui::Selectable(CBuffer::TextureUsageString[i], &bSelected);
	// 	}
	// }
}

void GUI_Editor_Material::HandleFloatType(FMaterialParam& Param)
{
	ImGui::DragFloat(Param.Name.c_str(), &Param.FloatValue, 0.01f, 0.f, 100.f);
}

void GUI_Editor_Material::HandleFloat2Type(FMaterialParam& Param)
{
	ImGui::DragFloat(Param.Name.c_str(), &Param.FloatValue, 0.01f, 0.f, 1.f);
}

void GUI_Editor_Material::HandleFloat4Type(FMaterialParam& Param, uint32_t Index) const
{
	const FVector4 cachedColor = Param.Float4Value;
	float          color[4];

	bool bUseTexture = Param.ParamValue == EMaterialParamValue::Texture2D;
	ImGui::Text("Use Texture");
	ImGui::SameLine();
	ImGui::Checkbox(Param.Name.c_str(), &bUseTexture);

	if (bUseTexture)
	{
		Param.ParamValue = EMaterialParamValue::Texture2D;
		if (FMaterialParam* flagParam = mMaterialToEdit->
				GetInstanceParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_USAGE_FLAG))
		{
			uint32_t flag = static_cast<uint32_t>(flagParam->IntegerValue);
			flag |= 1 << (Index);
			flagParam->IntegerValue = flag;
			mMaterialToEdit->EditInstanceParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_USAGE_FLAG, *flagParam);
		}
	}

	ImGui::DragFloat4(Param.Name.c_str(), &Param.Float4Value.x, 0.01f, 0.f, 1.f);
	color[0] = Param.Float4Value.x;
	color[1] = Param.Float4Value.y;
	color[2] = Param.Float4Value.z;
	color[3] = Param.Float4Value.w;
	ImGui::SameLine();

	ImGui::ColorEdit4(Param.Name.c_str(),
					  reinterpret_cast<float*>(&color),
					  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
	Param.Float4Value = FVector4{color[0], color[1], color[2], color[3]};


	if (cachedColor != Param.Float4Value)
	{
		mMaterialToEdit->EditInstanceParam(Param.Name, Param);
	}
}

void GUI_Editor_Material::ShowMaterialEditor()
{
	ImGui::BeginChild("MaterialView", ImVec2(400, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border);

	ImGui::Image(mViewport->SRV.Get(), ImGui::GetContentRegionAvail());

	if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
	{
		mCamera->Tick(mDeltaTime);
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("MaterialEditor", ImVec2(ImGui::GetContentRegionAvail().x, 0), true);

	// Mesh Selector
	if (ImGui::BeginCombo("MeshSelector", mPreviewMeshObject->mName.c_str()))
	{
		const JArray<JMeshObject*>& loaded = GetWorld.MeshManager->GetManagedList();
		for (auto& loadedMesh : loaded)
		{
			JText meshName = loadedMesh->mName;
			if (ImGui::Selectable(meshName.c_str()))
			{
				SetMeshObject(meshName);
				break;
			}
		}

		ImGui::EndCombo();
	}

	if (ImGui::CollapsingHeader("Properties"))
	{
		ImGui::InputText("Name",
						 &mMaterialToEdit->mFileName[0],
						 256,
						 ImGuiInputTextFlags_CharsNoBlank);

		auto materialRef = mMaterialToEdit->mParentMaterial;

		ImGui::Text("Parent Material");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##ParentMaterial", materialRef ? materialRef->GetMaterialName().c_str() : "None Selected"))
		{
			const JArray<JMaterial*>& loaded = GetWorld.MaterialManager->GetManagedList();
			for (auto& loadedMaterial : loaded)
			{
				JText materialName = loadedMaterial->GetMaterialName();
				if (ImGui::Selectable(materialName.c_str()))
				{
					mMaterialToEdit->SetParentMaterial(loadedMaterial);
					break;
				}
			}

			ImGui::EndCombo();
		}
	}

	if (ImGui::CollapsingHeader("Shader Params"))
	{
		auto& params = mMaterialToEdit->mInstanceParams;
		for (int32_t i = 0; i < params.size(); ++i)
		{
			FMaterialParam& param = params[i];

			switch (param.ParamValue)
			{
			case EMaterialParamValue::Integer:
				HandleIntegerType(param);
				break;
			case EMaterialParamValue::Float:
				HandleFloatType(param);
				break;
			case EMaterialParamValue::Float2:
				HandleFloat2Type(param);
				break;
			case EMaterialParamValue::Float3:
			case EMaterialParamValue::Float4:
				HandleFloat4Type(param, i);
				break;
			case EMaterialParamValue::Texture2D:
				ShowTextureSlot(param, i);
				break;
			}
		}
	}

	ImGui::Separator();

	Button_AddParam();

	ImGui::Separator();

	if (ImGui::Button("Cancel"))
	{}

	ImGui::SameLine();

	if (ImGui::Button("Save"))
	{
		Utils::Serialization::Serialize(mMaterialToEdit->mFileName.c_str(), mMaterialToEdit);
	}

	ImGui::EndChild();
}

void GUI_Editor_Material::ShowTextureSlot(FMaterialParam& Param, uint32_t Index) const
{
	ImGui::Text(Param.Name.c_str());
	ImGui::SameLine();

	bool bUseTexture = Param.ParamValue == EMaterialParamValue::Texture2D;
	ImGui::Text("Use Texture");
	ImGui::SameLine();
	ImGui::Checkbox(Param.Name.c_str(), &bUseTexture);
	if (!bUseTexture)
	{
		Param.ParamValue = EMaterialParamValue::Float4;
		if (FMaterialParam* flagParam = mMaterialToEdit->
				GetInstanceParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_USAGE_FLAG))
		{
			uint32_t flag = static_cast<uint32_t>(flagParam->IntegerValue);
			flag &= ~(1 << (Index));
			flagParam->IntegerValue = flag;
			mMaterialToEdit->EditInstanceParam(CBuffer::NAME_CONSTANT_VARIABLE_MATERIAL_USAGE_FLAG, *flagParam);
		}
	}


	JText label = std::format("##{}", Param.Name);

	// Texture Path
	if (ImGui::BeginCombo(label.c_str(), Param.StringValue.c_str()))
	{
		std::vector<JTexture*> loaded = GetWorld.TextureManager->GetManagedList();

		for (const auto& tex : loaded)
		{
			JText texName = WString2String(tex->GetTextureName());
			if (ImGui::Selectable(texName.c_str()))
			{
				Param.StringValue  = texName;
				Param.TextureValue = tex;

				break;
			}
		}


		ImGui::EndCombo();
	}

	ImGui::Text(u8("텍스처 드랍"));
	ImGui::SameLine();
	ImGui::Image(Param.TextureValue ? Param.TextureValue->GetSRV() : nullptr, {150, 150});

	if (ImGui::IsMouseReleased(0) && ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();

		const char* assetPath = static_cast<const char*>(payload->Data);
		Param.StringValue     = assetPath;
		Param.TextureValue    = GetWorld.TextureManager->Load(assetPath);
	}

}

void GUI_Editor_Material::Button_AddParam()
{
	// 파라미터 추가 버튼
	if (ImGui::Button("New Param"))
	{
		bOpenNewParamPopup = true;
	}
}


void GUI_Editor_Material::Update_Implementation(float DeltaTime)
{
	mDeltaTime = DeltaTime;

	ShowMaterialEditor();

	if (bOpenNewParamPopup)
	{
		ImGui::OpenPopup("NewParamPopup");
		if (ImGui::BeginPopup("NewParamPopup"))
		{
			// Name Slot
			static char paramName[256];
			ImGui::InputText("Param Name", paramName, 256);

			static uint8_t selectedType = 0; // 기본값을 원하는 초기 값으로 설정

			// Type Slot
			if (ImGui::BeginMenu("ParamType"))
			{
				for (uint8_t i = 0; i < static_cast<uint8_t>(EMaterialParamValue::Max); ++i)
				{
					if (ImGui::MenuItem(HASH_MATERIAL_PARAM_VALUE_TYPE[i], nullptr, selectedType == i))
					{
						selectedType = i; // 선택된 타입 업데이트
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::Button("Add"))
			{
				FMaterialParam newParam;
				newParam.Name           = paramName;
				newParam.Key            = StringHash(paramName);
				newParam.bInstanceParam = true;
				newParam.ParamValue     = static_cast<EMaterialParamValue>(selectedType);
				newParam.Float4Value    = FVector4::ZeroVector;

				mMaterialToEdit->AddInstanceParam(newParam);
				ImGui::CloseCurrentPopup();
				bOpenNewParamPopup = false;
			}

			ImGui::EndPopup();

		}
	}
}
