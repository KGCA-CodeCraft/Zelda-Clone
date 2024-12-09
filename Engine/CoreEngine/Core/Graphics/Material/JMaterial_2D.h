﻿#pragma once
#include "JMaterial.h"

class JMaterial_2D : public JMaterial
{
public:
	JMaterial_2D(JTextView InName);

public:
	void BindShader(ID3D11DeviceContext*          InDeviceContext) override;

private:
	void InitializeParams() override;
};
