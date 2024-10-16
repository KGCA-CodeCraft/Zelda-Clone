#include "ConstantBuffers.hlslinc"
#include "InputLayout.hlslinc"
#include "ShaderUtils.hlslinc"
#include "Params.hlslinc"

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

/*
MaterialProperties g_MatDiffuse = MakeMaterialProperties(DiffuseColor, g_DiffuseTexture, g_DiffuseSampler, bUseDiffuseMap);
MaterialProperties g_MatEmissive = MakeMaterialProperties(EmissiveColor, g_EmissiveTexture, g_TextureSampler0, bUseEmissiveMap);
MaterialProperties g_MatSpecular = MakeMaterialProperties(SpecularColor, g_SpecularTexture, g_TextureSampler1, bUseSpecularMap);
MaterialProperties g_MatReflection = MakeMaterialProperties(ReflectionColor, g_ReflectionTexture, g_TextureSampler0, bUseReflectionMap);
MaterialProperties g_MatAmient = MakeMaterialProperties(AmbientColor, g_AmbientOcclusionTexture, g_TextureSampler0, bUseAmbientMap);
MaterialProperties g_MatNormal = MakeMaterialProperties(NormalColor, g_NormalTexture, g_TextureSampler0, bUseNormalMap);
MaterialProperties g_MatDisplacement = MakeMaterialProperties(DisplacementColor, g_DisplacementTexture, g_TextureSampler0, bUseDisplacementMap);
*/

PixelInput_Base VS(VertexInput_Base Input)
{
	PixelInput_Base output;

	output.Pos = float4(Input.Pos, 1.f);

	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	float3 worldPos      = mul(Input.Pos, (float3x3)World);
	float3 lightDir      = worldPos.xyz - WorldLightPos.xyz;
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

	// Texture Map
    float3 albedoColor = g_DiffuseTexture.Sample(g_DiffuseSampler, Input.UV);
	float3 normalColor = g_NormalTexture.Sample(g_DiffuseSampler, Input.UV).rgb;
	// 	R (Red): Ambient Occlusion (AO)
	//  G (Green): Roughness
	//  B (Blue): Metallic
	float  ambientColor  = g_AmbientOcclusionTexture.Sample(g_DiffuseSampler, Input.UV).r;
	float  roughness     = g_RoughnessTexture.Sample(g_DiffuseSampler, Input.UV).g;
	float  metallic      = g_MetallicTexture.Sample(g_DiffuseSampler, Input.UV).b;

	ambientColor = 0.1;
	roughness = 0.5;
 	metallic = 0.5;    

	if (bUseDiffuseMap != 1)
	{
		albedoColor.rgb = DiffuseColor.rgb;
	}

	normalColor = normalize(normalColor * 2.0f - 1.0f); // -1 ~ 1 사이로 정규화
	if (bUseNormalMap != 1)
	{
		normalColor = Input.Normal;
	}

	if (bUseAmbientMap != 1)
    {
        ambientColor = AmbientColor;
    }

	float3 worldNormal = mul( Input.Normal, normalColor);

	// 아래 값들은 이미 VS에서 정규화 되었지만 보간기에서 보간(선형 보간)된 값들이므로 다시 정규화
	float3 lightDir = normalize(Input.LightDir);
	float  NdotL    = saturate(dot(normalColor, -lightDir));
	float3 diffuse  = NdotL * albedoColor.rgb * LightColor.rgb;
	/*
		// Specular 계산 (Cook-Torrance 모델 적용 가능)
		float3 viewDir = normalize(Input.ViewDir);
		float3 halfDir = normalize(viewDir + lightDir);
		float NdotV = saturate(dot(worldNormal, viewDir));
		float NdotH = saturate(dot(worldNormal, halfDir));
		float VdotH = saturate(dot(viewDir, halfDir));
	
		// Fresnel 계산 (Schlick's approximation)
		float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedoColor.rgb, metallic); // 금속일수록 알베도가 반사색으로
		float3 F = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);
	
		// 거칠기(Roughness)에 따른 반사광(Specular) 감소
		float roughnessSquared = roughness * roughness;
		float specular = (F * NdotL) / (4.0f * NdotV * NdotL + 0.0001f);
	
		// 주변광(Ambient)
		float3 ambient = float3(0.1f, 0.1f, 0.1f) * ambientColor * albedoColor.rgb;
	
		// 최종 조명 결과
		float3 finalColor = diffuse + specular + ambient;
	
		return float4(finalColor, 1.0f);*/


	float3 specular = 0;

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
	float3 ambient = ambientColor * albedoColor.rgb;
	//
	float3 finalColor = float3(diffuse +  ambient + (specular * metallic));
	// finalColor.rgb    = lerp(finalColor.rgb, finalColor.rgb * metallic, metallic);

	return float4(finalColor, 1.f);

}
