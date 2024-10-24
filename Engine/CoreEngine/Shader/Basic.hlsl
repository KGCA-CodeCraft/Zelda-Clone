#include "CommonConstantBuffers.hlslinc"
#include "InputLayout.hlslinc"
#include "Params.hlslinc"
#include "ShaderUtils.hlslinc"

Texture2D g_DiffuseTexture : register(t0);
Texture2D g_EmissiveTexture : register(t1);
Texture2D g_SpecularTexture : register(t2);
Texture2D g_ReflectionTexture : register(t3);
Texture2D g_AmbientOcclusionTexture : register(t4);
Texture2D g_NormalTexture : register(t5);
Texture2D g_DisplacementTexture : register(t6);
Texture2D g_RoughnessTexture : register(t7);
Texture2D g_MetallicTexture : register(t8);


SamplerState g_DiffuseSampler : register(s0);
SamplerState g_TextureSampler0 : register(s1);
SamplerState g_TextureSampler1 : register(s2);

PixelInput_Base VS(VertexIn Input)
{
	PixelInput_Base output;

	output.Pos = float4(Input.Pos, 1.f);

	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	float3 worldPos      = mul(Input.Pos, (float3x3)World);
	float3 lightDir      = worldPos.xyz - DirectionalLightPos.xyz;
	float3 viewDir       = worldPos.xyz - WorldCameraPos.xyz;
	float3 worldNormal   = mul(Input.Normal, (float3x3)World);
	float3 worldTangent  = mul(Input.Tangent, (float3x3)World);
	float3 worldBinormal = mul(Input.Binormal, (float3x3)World);

	output.Color    = Input.Color;
	output.UV       = Input.UV;
	output.LightDir = normalize(lightDir);
	output.ViewDir  = normalize(viewDir);
	output.Normal   = normalize(worldNormal);
	output.Tangent  = normalize(worldTangent);
	output.Binormal = normalize(worldBinormal);

	return output;
}


float4 PS(PixelInput_Base Input) : SV_TARGET
{
	return float4(1.f, 1.f, 1.f, 1.f);

	float3 diffuseColor = Diffuse.Color;
	float3 normalColor  = normalize(Input.Normal * 2.0f - 1.0f); // -1 ~ 1 사이로 정규화

	// 	R (Red): Ambient Occlusion (AO)
	//  G (Green): Roughness
	//  B (Blue): Metallic
	float ambientColor = AmbientOcclusion.Value;
	float roughness    = Roughness.Value;
	float metallic     = Metallic.Value;

	// Texture Map
	if (TextureUsageFlag & TEXTURE_USE_DIFFUSE)
	{
		diffuseColor = g_DiffuseTexture.Sample(g_DiffuseSampler, Input.UV);
	}
	if (TextureUsageFlag & TEXTURE_USE_NORMAL)
	{
		normalColor = g_NormalTexture.Sample(g_DiffuseSampler, Input.UV).rgb;
	}
	if (TextureUsageFlag & TEXTURE_USE_AMBIENTOCCLUSION)
	{
		ambientColor = g_AmbientOcclusionTexture.Sample(g_DiffuseSampler, Input.UV).r;
	}
	if (TextureUsageFlag & TEXTURE_USE_ROUGHNESS)
	{
		roughness = g_RoughnessTexture.Sample(g_DiffuseSampler, Input.UV).g;
	}
	if (TextureUsageFlag & TEXTURE_USE_METALLIC)
	{
		metallic = g_MetallicTexture.Sample(g_DiffuseSampler, Input.UV).b;
	}

	// 아래 값들은 이미 VS에서 정규화 되었지만 보간기에서 보간(선형 보간)된 값들이므로 다시 정규화
	float3 lightDir = normalize(Input.LightDir);
	float  NdotL    = saturate(dot(normalColor, -lightDir));
	float3 diffuse  = NdotL * diffuseColor.rgb * DirectionalLightColor.rgb;

	float3 specular = 0;

	return float4(NdotL, 0, 0, 1);

	// 난반사광이 없으면 애초에 반사되는 색상이 없음
	if (diffuse.x > 0)
	{
		// 노말과 라이트의 반사각
		float3 reflection = reflect(lightDir, normalColor);
		float3 viewDir    = normalize(Input.ViewDir);

		// Specular는 카메라뷰와 반사각의 내적값을 제곱(할수록 광 나는 부분이 줄어듦) 사용
		specular = saturate(dot(reflection, -viewDir));
		specular = pow(specular, 1 / roughness);
	}

	// 주변광 (없으면 반사광이 없는곳은 아무것도 보이지 않음)
	float3 ambient = ambientColor * diffuseColor.rgb;
	//
	float3 finalColor = float3(diffuse + ambient + (specular * metallic));
	// finalColor.rgb    = lerp(finalColor.rgb, finalColor.rgb * metallic, metallic);

	return float4(finalColor, 1.f);

}
