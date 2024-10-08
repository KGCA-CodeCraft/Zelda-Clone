#pragma once
#include "common_include.h"

//--------------------------------------------- String Func -------------------------------------------------------------
constexpr uint32_t djb2_impl(const char* InString, uint32_t PrevHash)
{
	int32_t c;
	while ((c    = *InString++))
		PrevHash = ((PrevHash << 5) + PrevHash) + c;

	return PrevHash;
}

constexpr uint32_t StringHash(const char* InString)
{
	return djb2_impl(InString, 5381);
}

constexpr uint32_t StringHash(const wchar_t* InString)
{
	return djb2_impl(WString2String(InString).data(), 5381);
}
//--------------------------------------------- String Func -------------------------------------------------------------


// ------------------------------------------- JHash Table ---------------------------------------------------------------
constexpr uint32_t JAssetHash = StringHash("JASSET\0");

constexpr uint32_t Hash_EXT_FBX    = StringHash(".fbx");
constexpr uint32_t Hash_EXT_JASSET = StringHash(".jasset");

constexpr uint32_t HASH_EXT_PNG = StringHash(".png");
constexpr uint32_t HASH_EXT_JPG = StringHash(".jpg");
constexpr uint32_t HASH_EXT_TGA = StringHash(".tga");
constexpr uint32_t HASH_EXT_BMP = StringHash(".bmp");
constexpr uint32_t HASH_EXT_DDS = StringHash(".dds");

constexpr uint32_t HASH_EXT_HLSL = StringHash(".hlsl");
//---------------------------------------------- String Func --------------------------------------------------------------


//------------------------------------------------- Enum  -----------------------------------------------------------------
template <typename EnumType>
constexpr uint8_t EnumAsByte(EnumType Type)
{
	return static_cast<uint8_t>(Type);
}
//------------------------------------------------- Enum  -----------------------------------------------------------------
