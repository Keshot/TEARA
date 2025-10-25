#version 460 core

const int MaxPointLights    = 2;
const int MaxSpotLights     = 1;

struct Material {
    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
};

struct LightSpec {
    vec3    Color;
    float   Intensity;
    float   AmbientIntensity;
    float   SpecularIntensity;
};

struct LightAttenuation {
    vec3        Position;
    float       DisctanceMax;
    float       DisctanceMin;
    float       AttenuationFactor;
};

struct DirectionLight {
    LightSpec   Specification;
    vec3        Direction;
};

struct PointLight {
    LightSpec           Specification;
    LightAttenuation    AttenuationSpec;
};

struct SpotLight {
    LightSpec           Specification;
    LightAttenuation    AttenuationSpec;
    vec3                Direction;
    float               CosCutoffAngle;
    float               CutoffAttenuationFactor;
};

struct MeshFragmentInfo {
    Material    FragmentMaterial;
    vec3        FragmentNormal;
};

struct LightCalculationResult {
    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
};

in vec2 FragmentTextureCoordinate;
in vec3 FragmentNormal;
in vec3 FragmentPosition;
in vec4 FragmentPositionInLightSpace;

out vec4 FragmentColor;

uniform sampler2D   DiffuseTexture;
uniform sampler2D   SpecularExponentMap;
uniform sampler2D   ShadowMapTexture;
uniform Material    MeshMaterial;

uniform vec3            ViewerPosition;

uniform DirectionLight  SceneDirectionalLight;

uniform int             PointLightsAmount;
uniform PointLight      PointLights[MaxPointLights];

uniform int             SpotLightsAmount;
uniform SpotLight       SpotLights[MaxSpotLights];

MeshFragmentInfo Info;

float CalculateLightShadow()
{
    vec3 PosInLightSpaceProjected   = FragmentPositionInLightSpace.xyz / FragmentPositionInLightSpace.w;
    PosInLightSpaceProjected        = PosInLightSpaceProjected * 0.5 + 0.5;

    float CurrentDepth  = PosInLightSpaceProjected.z;
    float MapDepth      = texture(SpecularExponentMap, PosInLightSpaceProjected.xy).r;

    float ShadowFactor = CurrentDepth > MapDepth ? 0.1 : 1.0;

    return ShadowFactor;
}

LightCalculationResult MakeLightsCalculationResult()
{
    LightCalculationResult Result;

    Result.AmbientColor     = vec3(0.0);
    Result.DiffuseColor     = vec3(0.0);
    Result.SpecularColor    = vec3(0.0);

    return Result;
}

LightCalculationResult CalcLightInternal(in LightSpec Specification, in vec3 LightDirection)
{
    LightCalculationResult Result;

    vec3 LightColor = Specification.Intensity * Specification.Color;

    Result.AmbientColor = LightColor * Specification.AmbientIntensity * Info.FragmentMaterial.AmbientColor;

    vec3 N  = Info.FragmentNormal;
    vec3 LD = normalize(LightDirection);

    float LightFactor = max(dot(N, -LD), 0);

    Result.DiffuseColor = LightColor * Info.FragmentMaterial.DiffuseColor * LightFactor;

    vec3    VP          = normalize(ViewerPosition - FragmentPosition);
    vec3    R           = reflect(LD, N);
    float   SpecularExp = texture(SpecularExponentMap, FragmentTextureCoordinate).r * 255.0;

    float SpecularFactor = pow(max(dot(R, VP), 0), SpecularExp);

    Result.SpecularColor = LightColor * Info.FragmentMaterial.SpecularColor * SpecularFactor * Specification.SpecularIntensity;

    return Result;
}

LightCalculationResult CalcDirectionalLight(in DirectionLight Light)
{
    return CalcLightInternal(Light.Specification, Light.Direction);
}

void CalcLightToFragmentDirectionAndAttenuation(in LightAttenuation AttenuationSpec, out vec3 LightToFragmentDirection, out float Attenuation)
{
    LightToFragmentDirection  = FragmentPosition - AttenuationSpec.Position;

    float Distance  = length(LightToFragmentDirection);
    Attenuation     = pow(clamp((AttenuationSpec.DisctanceMax - Distance) / (AttenuationSpec.DisctanceMax - AttenuationSpec.DisctanceMin), 0.0, 1.0), AttenuationSpec.AttenuationFactor);
}

LightCalculationResult CalcPointLight(in PointLight Light)
{
    vec3    LightDirection;
    float   Attenuation;
    
    CalcLightToFragmentDirectionAndAttenuation(Light.AttenuationSpec, LightDirection, Attenuation);

    LightCalculationResult Result = CalcLightInternal(Light.Specification, LightDirection);

    Result.AmbientColor     *= Attenuation;
    Result.DiffuseColor     *= Attenuation;
    Result.SpecularColor    *= Attenuation;

    return Result;
}

LightCalculationResult CalcSpotLight(in SpotLight Light)
{
    vec3    ToFragmentDirection;
    float   Attenuation;

    LightCalculationResult Result = CalcLightInternal(Light.Specification, Light.Direction);

    CalcLightToFragmentDirectionAndAttenuation(Light.AttenuationSpec, ToFragmentDirection, Attenuation);

    ToFragmentDirection     = normalize(ToFragmentDirection);
    float Alpha             = dot(Light.Direction, ToFragmentDirection);
    float CutoffAttenuation = pow(clamp((Alpha - Light.CosCutoffAngle) / (1.0 - Light.CosCutoffAngle), 0.0, 1.0), Light.CutoffAttenuationFactor);

    Result.AmbientColor     = Result.AmbientColor   * Attenuation * CutoffAttenuation;
    Result.DiffuseColor     = Result.DiffuseColor   * Attenuation * CutoffAttenuation;
    Result.SpecularColor    = Result.SpecularColor  * Attenuation * CutoffAttenuation;

    return Result;
}

void main()
{
    Info.FragmentMaterial   = MeshMaterial;
    Info.FragmentNormal     = normalize(FragmentNormal);

    LightCalculationResult DirectionalLightResult = CalcDirectionalLight(SceneDirectionalLight);

    LightCalculationResult PointLightsResult = MakeLightsCalculationResult();
    for (int PointLightIndex = 0; PointLightIndex < PointLightsAmount; ++PointLightIndex) {
        LightCalculationResult CurrentPointLightResult = CalcPointLight(PointLights[PointLightIndex]);

        PointLightsResult.AmbientColor  += CurrentPointLightResult.AmbientColor;
        PointLightsResult.DiffuseColor  += CurrentPointLightResult.DiffuseColor;
        PointLightsResult.SpecularColor += CurrentPointLightResult.SpecularColor;
    }

    LightCalculationResult SpotLightsResult = MakeLightsCalculationResult();
    for (int SpotLightIndex = 0; SpotLightIndex < SpotLightsAmount; ++SpotLightIndex) {
        LightCalculationResult CurrentSpotLightResult = CalcSpotLight(SpotLights[SpotLightIndex]);

        SpotLightsResult.AmbientColor  += CurrentSpotLightResult.AmbientColor;
        SpotLightsResult.DiffuseColor  += CurrentSpotLightResult.DiffuseColor;
        SpotLightsResult.SpecularColor += CurrentSpotLightResult.SpecularColor;
    }

    float ShadowFactor = CalculateLightShadow();
    vec3 Albedo = texture(DiffuseTexture, FragmentTextureCoordinate).rgb;

    vec3 AmbientColor   = DirectionalLightResult.AmbientColor  + PointLightsResult.AmbientColor  + SpotLightsResult.AmbientColor;
    vec3 DiffuseColor   = (DirectionalLightResult.DiffuseColor  * ShadowFactor) + PointLightsResult.DiffuseColor  + SpotLightsResult.DiffuseColor;
    vec3 SpecularColor  = (DirectionalLightResult.SpecularColor * ShadowFactor) + PointLightsResult.SpecularColor + SpotLightsResult.SpecularColor;

    vec3 FinalColor = (Albedo * (AmbientColor + DiffuseColor)) + SpecularColor;

    FragmentColor = vec4(clamp(FinalColor, 0.0, 1.0), 1.0);
}