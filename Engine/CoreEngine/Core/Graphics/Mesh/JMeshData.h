﻿#pragma once
#include "common_include.h"
#include "Core/Graphics/ShaderStructs.h"
#include "Core/Utils/FileIO/JSerialization.h"

class JMaterial;
namespace Utils::Fbx
{
	class FbxFile;
}

enum class EMeshType : uint8_t
{
	Static = 0, // 정적 메시
	Skeletal,   // 스켈레탈 메시
	DUMMY, 		// 더미 메시
	BIPED		// 바이패드 메시 (뭔지는 모르겠음 maya에서 관리하는것같음)
};

/**
 * Mesh 
 */
class JMeshData : public ISerializable, public std::enable_shared_from_this<JMeshData>// 직렬화 해서 저장
{
public:
	JMeshData() = default;

public:
	uint32_t GetType() const override;
	bool     Serialize_Implement(std::ofstream& FileStream) override;
	bool     DeSerialize_Implement(std::ifstream& InFileStream) override;

public:
	void AddInfluenceBone(const JText& InBoneName, FMatrix InBindPose);

public:
	[[nodiscard]] bool ApplyMaterial() const;

public:
	[[nodiscard]] JText GetName() const { return mName; }
	[[nodiscard]] int32_t GetIndex() const { return mIndex; }
	[[nodiscard]] EMeshType GetClassType() const { return mClassType; }
	[[nodiscard]] int32_t GetMaterialRefNum() const { return mMaterialRefNum; }
	[[nodiscard]] int32_t GetFaceNum() const { return mFaceCount; }
	[[nodiscard]] Ptr<JMeshData> GetParentMesh() const { return mParentMesh.lock(); }
	[[nodiscard]] const std::vector<Ptr<JMeshData>>& GetSubMesh() const { return mSubMesh; }
	[[nodiscard]] const std::vector<Ptr<JMeshData>>& GetChildMesh() const { return mChildMesh; }
	[[nodiscard]] const Ptr<JVertexData<Vertex::FVertexInfo_Base>>& GetVertexData() const { return mVertexData; }
	[[nodiscard]] Ptr<JMaterial> GetMaterial() const { return mMaterial; }
	[[nodiscard]] FMatrix GetInitialModelTransform() const { return mInitialModelTransform; }

protected:
	// -------------------------- Mesh Info --------------------------
	JText     mName;
	int32_t   mIndex;
	EMeshType mClassType;

	// -------------------------- Face Count --------------------------
	int32_t mFaceCount;

	// -------------------------- Mesh Hierarchy --------------------------
	WPtr<JMeshData>        mParentMesh;
	JArray<Ptr<JMeshData>> mSubMesh;
	JArray<Ptr<JMeshData>> mChildMesh;

	// -------------------------- Vertex Data --------------------------
	Ptr<JVertexData<Vertex::FVertexInfo_Base>> mVertexData;

	// -------------------------- Skin Mesh Data --------------------------
	JHash<JText, FMatrix> mBindPoseMap;

	// -------------------------- Material --------------------------
	Ptr<JMaterial> mMaterial;
	int32_t        mMaterialRefNum;

	// -------------------------- Initial Model Transform --------------------------
	FMatrix mInitialModelTransform;

private:
	friend class Utils::Fbx::FbxFile;
	friend class GUI_Editor_Material;
};


/**
 * Skin Mesh
 */
class JSkinnedMeshData
{
public:
	JSkinnedMeshData()  = default;
	~JSkinnedMeshData() = default;

public:
	void Initialize();
	void Initialize(int32_t InVertexCount, int32_t InVertexStride);

	void AddInfluenceBone(const JText& InBoneName);
	void AddBindPose(const JText& InBoneName, const FMatrix& InBindPose);
	void AddInverseBindPose(const JText& InBoneName, const FMatrix& InInverseBindPose);
	void AddWeight(int32_t InIndex, uint32_t InBoneIndex, float InBoneWeight);

	FORCEINLINE void SetVertexCount(int32_t InVertexCount) { mVertexCount = InVertexCount; }
	FORCEINLINE void SetVertexStride(int32_t InVertexStride) { mVertexStride = InVertexStride; }

	[[nodiscard]] FORCEINLINE JText    GetInfluenceBoneName(int32_t InIndex) const { return mInfluenceBones.at(InIndex); }
	[[nodiscard]] FORCEINLINE FMatrix GetInfluenceBoneBindPose(const JText& InBoneName) const { return mBindPoseMap.at(InBoneName); }
	[[nodiscard]] FORCEINLINE FMatrix GetInfluenceBoneInverseBindPose(const JText& InBoneName) const { return mInverseBindPoseMap.at(InBoneName); }

	[[nodiscard]] FORCEINLINE int32_t  GetVertexCount() const { return mVertexCount; }
	[[nodiscard]] FORCEINLINE int32_t  GetVertexStride() const { return mVertexStride; }
	[[nodiscard]] FORCEINLINE int32_t  GetBoneCount() const { return mInfluenceBones.size(); }
	[[nodiscard]] FORCEINLINE uint32_t GetBoneIndex(int32_t InIndex) const { return mBoneIndices[InIndex]; }
	[[nodiscard]] FORCEINLINE float    GetBoneWeight(int32_t InIndex) const { return mBoneWeights[InIndex]; }

public:
	enum { MAX_BONE_INFLUENCES = 4 };

private:
	JArray<JText> mInfluenceBones;

	JHash<JText, FMatrix> mBindPoseMap;
	JHash<JText, FMatrix> mInverseBindPoseMap;

	int32_t mVertexCount  = 0;
	int32_t mVertexStride = 0;

	UPtr<uint32_t[]> mBoneIndices;
	UPtr<float[]>    mBoneWeights;

};
