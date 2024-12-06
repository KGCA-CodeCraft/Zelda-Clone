﻿#include "JSkeletalMeshComponent.h"

#include "Core/Entity/Animation/JAnimator.h"
#include "Core/Entity/Camera/MCameraManager.h"
#include "Core/Graphics/Mesh/JSkeletalMeshObject.h"
#include "Core/Graphics/Mesh/MMeshManager.h"
#include "Core/Interface/JWorld.h"

JSkeletalMeshComponent::JSkeletalMeshComponent()
{
	mObjectType = NAME_OBJECT_SKELETAL_MESH_COMPONENT;
}

JSkeletalMeshComponent::JSkeletalMeshComponent(JTextView InName, AActor* InOwnerActor, JSceneComponent* InParentComponent)
	: JSceneComponent(InName, InOwnerActor, InParentComponent)
{
	mObjectType = NAME_OBJECT_SKELETAL_MESH_COMPONENT;
}

uint32_t JSkeletalMeshComponent::GetType() const
{
	return StringHash("SkeletonMeshComponent");
}

bool JSkeletalMeshComponent::Serialize_Implement(std::ofstream& FileStream)
{
	if (!JSceneComponent::Serialize_Implement(FileStream))
	{
		return false;
	}

	bool bHasMeshObject = mSkeletalMeshObject != nullptr;
	Utils::Serialization::Serialize_Primitive(&bHasMeshObject, sizeof(bool), FileStream);
	if (bHasMeshObject)
	{
		// mMeshObject->Serialize_Implement(FileStream);
		auto filePath = std::format("Game/Mesh/{}.jasset", mSkeletalMeshObject->GetName());
		Utils::Serialization::Serialize_Text(filePath, FileStream);
	}

	// // MaterialInstance
	// int32_t materialCount = mSkeletalMeshObject->GetMaterialCount();
	// Utils::Serialization::Serialize_Primitive(&materialCount, sizeof(int32_t), FileStream);
	// for (int32_t i = 0; i < materialCount; ++i)
	// {
	// 	auto* materialInstance = mSkeletalMeshObject->GetMaterialInstance(i);
	// 	auto  materialPath     = materialInstance->GetMaterialPath();
	// 	Utils::Serialization::Serialize_Text(materialPath, FileStream);
	// }

	return true;
}

bool JSkeletalMeshComponent::DeSerialize_Implement(std::ifstream& InFileStream)
{
	if (!JSceneComponent::DeSerialize_Implement(InFileStream))
	{
		return false;
	}

	bool bHasMeshObject;
	Utils::Serialization::DeSerialize_Primitive(&bHasMeshObject, sizeof(bool), InFileStream);

	if (bHasMeshObject)
	{
		JText filePath;
		Utils::Serialization::DeSerialize_Text(filePath, InFileStream);

		SetSkeletalMesh(filePath);
	}

	// int32_t materialCount;
	// Utils::Serialization::DeSerialize_Primitive(&materialCount, sizeof(int32_t), InFileStream);
	// for (int32_t i = 0; i < materialCount; ++i)
	// {
	// 	JText materialPath;
	// 	Utils::Serialization::DeSerialize_Text(materialPath, InFileStream);
	//
	// 	auto materialInstance = MMaterialInstanceManager::Get().Load(materialPath);
	// 	SetMaterialInstance(materialInstance, i);
	// }

	return true;
}

void JSkeletalMeshComponent::Tick(float DeltaTime)
{
	// TransformComponent에서 위치 업데이트
	JSceneComponent::Tick(DeltaTime);

	// Step1.1 프러스텀 박스(OBB) 업데이트
	mBoundingBox.Box.LocalAxis[0] = XMVector3TransformNormal(FVector(1, 0, 0), XMLoadFloat4x4(&mWorldMat));
	mBoundingBox.Box.LocalAxis[1] = XMVector3TransformNormal(FVector(0, 1, 0), XMLoadFloat4x4(&mWorldMat));
	mBoundingBox.Box.LocalAxis[2] = XMVector3TransformNormal(FVector(0, 0, 1), XMLoadFloat4x4(&mWorldMat));

	mBoundingBox.Box.Center = 0.5f * (mBoundingBox.Max + mBoundingBox.Min);
	mBoundingBox.Box.Center = XMVector3Transform(mBoundingBox.Box.Center, mWorldMat);
	mBoundingBox.Box.Extent = 0.5f * (mBoundingBox.Max - mBoundingBox.Min);

	// MeshObject의 버퍼 업데이트
	if (mSkeletalMeshObject)
	{
		mSkeletalMeshObject->Tick(DeltaTime);
		mSkeletalMeshObject->UpdateInstance_Transform(mWorldMat);
	}
}

void JSkeletalMeshComponent::Draw()
{
	// MeshObject의 Draw 호출
	if (mSkeletalMeshObject)
	{
		FVector distance = mWorldLocation - GetWorld.CameraManager->GetCurrentMainCam()->GetWorldLocation();
		float   dist     = distance.Length();
		mBoundingBox.DrawDebug();
		mSkeletalMeshObject->AddInstance(dist);
	}

	JSceneComponent::Draw();
}

void JSkeletalMeshComponent::DrawID(uint32_t ID)
{
	if (mSkeletalMeshObject)
	{
		mSkeletalMeshObject->DrawID(ID);
	}

	JSceneComponent::DrawID(ID);
}

void JSkeletalMeshComponent::SetSkeletalMesh(JTextView InSkeletalMeshPath)
{
	mSkeletalMeshObject = GetWorld.MeshManager->Clone<JSkeletalMeshObject>(InSkeletalMeshPath.data());
	// mAnimator           = MakeUPtr<JAnimator>(mSkeletalMeshObject->mSkeletalMesh->mSkeleton);
	mBoundingBox = mSkeletalMeshObject->GetBoundingBox();
}

void JSkeletalMeshComponent::SetAnimation(JAnimationClip* InAnimationClip)
{
	if (mSkeletalMeshObject)
	{
		mSkeletalMeshObject->SetAnimation(InAnimationClip);
	}
}

void JSkeletalMeshComponent::ShowEditor()
{
	JSceneComponent::ShowEditor();

	ImGui::SeparatorText(u8("스켈레탈 메시"));

	ImGui::Image(nullptr, ImVec2(100, 100));
	if (ImGui::IsMouseReleased(0) && ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();;
		{
			const char* assetPath = static_cast<const char*>(payload->Data);
			auto        metaData  = Utils::Serialization::GetType(assetPath);

			if (metaData.AssetType == HASH_ASSET_TYPE_SKELETAL_MESH)
			{
				SetSkeletalMesh(assetPath);
			}

		}
	}
}
