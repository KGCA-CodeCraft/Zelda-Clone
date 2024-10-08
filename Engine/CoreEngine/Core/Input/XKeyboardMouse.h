﻿#pragma once
#include "enums.h"
#include "Core/Utils/Math/Vector2.h"

struct FKeyboardData
{
	EKeyCode  Key;
	EKeyState State;

	std::chrono::steady_clock::time_point PressTime;
	std::chrono::steady_clock::time_point ReleaseTime;
	std::chrono::milliseconds             PressDuration;
};

class XKeyboardMouse
{
public:
	XKeyboardMouse()  = default;
	~XKeyboardMouse() = default;

public:
	void Initialize();
	void Update();

public:
	FORCEINLINE bool                   IsKeyDown(const EKeyCode Key) const { return IsKeyDown_Internal(Key); }
	FORCEINLINE bool                   IsKeyUp(const EKeyCode Key) const { return IsKeyUp_Internal(Key); }
	FORCEINLINE bool                   IsKeyPressed(const EKeyCode Key) const { return IsKeyPressed_Internal(Key); }
	FORCEINLINE const JMath::TVector2& GetMousePosition() const { return mMousePosition; }
	FORCEINLINE POINT                  GetCurMouseDelta() const { return mCurrentMouseDelta; }

private:
	void CreateKeys();

	void UpdateKeys();
	void UpdateKey(FKeyboardData& InKey);
	void UpdateKeyDown(FKeyboardData& InKey);
	void UpdateKeyUp(FKeyboardData& InKey);

	void UpdateMouse();

	void ClearKeys();
	void ClearMouse();

private:
	inline bool IsKeyDown_Internal(EKeyCode Key) const
	{
		return mKeys[static_cast<UINT>(Key)].State == EKeyState::Down;
	}

	inline bool IsKeyUp_Internal(EKeyCode Key) const
	{
		return mKeys[static_cast<UINT>(Key)].State == EKeyState::Up;
	}

	inline bool IsKeyPressed_Internal(EKeyCode Key) const
	{
		return mKeys[static_cast<UINT>(Key)].State == EKeyState::Pressed;
	}

	static bool IsKeyDown_Impl(const EKeyCode InKey)
	{
		return GetAsyncKeyState(ASCII[static_cast<uint8_t>(InKey)]) & 0x8000;
	}

private:
	std::vector<FKeyboardData> mKeys;
	JMath::TVector2            mLastMousePosition;
	JMath::TVector2            mMousePosition;

	POINT mCurrentMouseDelta;
};
