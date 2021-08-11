Texture2D DiffuseTexture1 : register(t0);
Texture2D DiffuseTexture2 : register(t1);

SamplerState TextureSampler : register(s0);

cbuffer TerrainParams
{
	float4 TextureBoundry;
};

cbuffer Transforms
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjMatrix;
	matrix WorldViewProjMatrix;
};


cbuffer SurfaceReflectanceInfo
{
	float4 SurfaceEmissiveColour;
	float4 SurfaceConstants;			// x: ambient, y: diffuse, z: specular, w: shininess
};

cbuffer PointLightInfo
{
	float4 PointLightColour;
	float3 PointLightPosition;
	float2 PointLightRange;

	float4 PointLightColour1;
	float3 PointLightPosition1;
	float2 PointLightRange1;

	float4 PointLightColour2;
	float3 PointLightPosition2;
	float2 PointLightRange2;
};

cbuffer DirectionalLightInfo
{
	float4 DirectionalLightColour;
	float3 DirectionalLightPosition;
	float3 DirectionalLightDirection;

	float4 DirectionalLightColour1;
	float3 DirectionalLightPosition1;
	float3 DirectionalLightDirection1;

	float4 DirectionalLightColour2;
	float3 DirectionalLightPosition2;
	float3 DirectionalLightDirection2;
};

cbuffer SpotLightInfo
{
	float4 SpotLightColour;
	float3 SpotLightPosition;
	float3 SpotLightDirection;
	float2 SpotLightRange;
	float2 SpotLightFocus;

	float4 SpotLightColour1;
	float3 SpotLightPosition1;
	float3 SpotLightDirection1;
	float2 SpotLightRange1;
	float2 SpotLightFocus1;

	float4 SpotLightColour2;
	float3 SpotLightPosition2;
	float3 SpotLightDirection2;
	float2 SpotLightRange2;
	float2 SpotLightFocus2;
};

cbuffer SceneInfo
{
	float4 ViewPosition;
};

//-----------------------------------------------------------------------------
struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float4 color    : COLOR;
	float2 tex		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 worldPos : POSITION;
	float4 normal   : NORMAL;
	float4 color    : COLOR;
	float2 tex		: TEXCOORD0;
};

float3 calculateDirectionalLight(float3 surfaceNormal, float3 position)
{
	float3 viewVector_d = normalize(position - ViewPosition.xyz);
	float3 directionallightreflectVector = reflect(DirectionalLightDirection, surfaceNormal);

	// Ambience
	float3 DirectionalLightAmbience = DirectionalLightColour.xyz * SurfaceConstants.x;

	// Diffuse
	float3 DirectionalDiffuse = saturate(dot(surfaceNormal, -DirectionalLightDirection)) * DirectionalLightColour.xyz * SurfaceConstants.y;

	// Specular
	float3 DirectionalSpecular = pow(saturate(dot(directionallightreflectVector, -viewVector_d)), SurfaceConstants.w) * DirectionalLightColour.xyz * SurfaceConstants.z;

	return (DirectionalLightAmbience + DirectionalDiffuse + DirectionalSpecular);

}

float3 calculateDirectionalLight1(float3 surfaceNormal, float3 position)
{
	float3 viewVector_d = normalize(position - ViewPosition.xyz);
	float3 directionallightreflectVector1 = reflect(DirectionalLightDirection1, surfaceNormal);

	// Ambience
	float3 DirectionalLightAmbience1 = DirectionalLightColour1.xyz * SurfaceConstants.x;

	// Diffuse
	float3 DirectionalDiffuse1 = saturate(dot(surfaceNormal, -DirectionalLightDirection1)) * DirectionalLightColour1.xyz * SurfaceConstants.y;

	// Specular
	float3 DirectionalSpecular1 = pow(saturate(dot(directionallightreflectVector1, -viewVector_d)), SurfaceConstants.w) * DirectionalLightColour1.xyz * SurfaceConstants.z;

	return (DirectionalLightAmbience1 + DirectionalDiffuse1 + DirectionalSpecular1);

}

float3 calculateDirectionalLight2(float3 surfaceNormal, float3 position)
{
	float3 viewVector_d = normalize(position - ViewPosition.xyz);
	float3 directionallightreflectVector2 = reflect(DirectionalLightDirection2, surfaceNormal);

	// Ambience
	float3 DirectionalLightAmbience2 = DirectionalLightColour2.xyz * SurfaceConstants.x;

	// Diffuse
	float3 DirectionalDiffuse2 = saturate(dot(surfaceNormal, -DirectionalLightDirection2)) * DirectionalLightColour2.xyz * SurfaceConstants.y;

	// Specular
	float3 DirectionalSpecular2 = pow(saturate(dot(directionallightreflectVector2, -viewVector_d)), SurfaceConstants.w) * DirectionalLightColour2.xyz * SurfaceConstants.z;

	return (DirectionalLightAmbience2 + DirectionalDiffuse2 + DirectionalSpecular2);

}

float3 calculatePointLight(float3 surfaceNormal, float3 position)
{

	float pointDistance = distance(PointLightPosition, position);
	float pointDistAtt = saturate(1 - pointDistance / PointLightRange.x);
	float3 pointlightDirection = normalize(position - PointLightPosition);
	float3 viewVector_p = normalize(position - ViewPosition.xyz);
	float3 pointlightreflectVector = reflect(pointlightDirection, surfaceNormal);

	// Ambience
	float3 PointLightAmbience = PointLightColour.xyz * SurfaceConstants.x;

	// Diffuse
	float3 PointDiffuse = saturate(dot(surfaceNormal, -pointlightDirection)) * pointDistAtt * PointLightColour.xyz * SurfaceConstants.y;

	// Specular
	float3 PointSpecular = pow(saturate(dot(pointlightreflectVector, -viewVector_p)), SurfaceConstants.w) * pointDistAtt * PointLightColour.xyz * SurfaceConstants.z;

	return (PointLightAmbience + PointDiffuse + PointSpecular);
}

float3 calculatePointLight1(float3 surfaceNormal, float3 position)
{

	float pointDistance1 = distance(PointLightPosition1, position);
	float pointDistAtt1 = saturate(1 - pointDistance1 / PointLightRange1.x);
	float3 pointlightDirection1 = normalize(position - PointLightPosition1);
	float3 viewVector_p = normalize(position - ViewPosition.xyz);
	float3 pointlightreflectVector1 = reflect(pointlightDirection1, surfaceNormal);

	// Ambience
	float3 PointLightAmbience1 = PointLightColour1.xyz * SurfaceConstants.x;

	// Diffuse
	float3 PointDiffuse1 = saturate(dot(surfaceNormal, -pointlightDirection1)) * pointDistAtt1 * PointLightColour1.xyz * SurfaceConstants.y;

	// Specular
	float3 PointSpecular1 = pow(saturate(dot(pointlightreflectVector1, -viewVector_p)), SurfaceConstants.w) * pointDistAtt1 * PointLightColour1.xyz * SurfaceConstants.z;

	return (PointLightAmbience1 + PointDiffuse1 + PointSpecular1);
}

float3 calculatePointLight2(float3 surfaceNormal, float3 position)
{

	float pointDistance2 = distance(PointLightPosition2, position);
	float pointDistAtt2 = saturate(1 - pointDistance2 / PointLightRange2.x);
	float3 pointlightDirection2 = normalize(position - PointLightPosition2);
	float3 viewVector_p = normalize(position - ViewPosition.xyz);
	float3 pointlightreflectVector2 = reflect(pointlightDirection2, surfaceNormal);

	// Ambience
	float3 PointLightAmbience2 = PointLightColour2.xyz * SurfaceConstants.x;

	// Diffuse
	float3 PointDiffuse2 = saturate(dot(surfaceNormal, -pointlightDirection2)) * pointDistAtt2 * PointLightColour2.xyz * SurfaceConstants.y;

	// Specular
	float3 PointSpecular2 = pow(saturate(dot(pointlightreflectVector2, -viewVector_p)), SurfaceConstants.w) * pointDistAtt2 * PointLightColour2.xyz * SurfaceConstants.z;

	return (PointLightAmbience2 + PointDiffuse2 + PointSpecular2);
}

float3 calculateSpotLight(float3 surfaceNormal, float3 position)
{

	float spotDistance = distance(SpotLightPosition, position);
	float spotDistAtt = saturate(1 - spotDistance / SpotLightRange.x);
	float3 spotlight2PixelVector = normalize(position - SpotLightPosition);
	float spotAngularAtt = saturate(pow(dot(spotlight2PixelVector, SpotLightDirection), SpotLightFocus.x));

	// Ambience
	float3 SpotLightAmbience = SpotLightColour.xyz * SurfaceConstants.x;

	// Diffuse
	float3 SpotDiffuse = saturate(dot(surfaceNormal, -SpotLightDirection)) * spotDistAtt * spotAngularAtt * SpotLightColour.xyz * SurfaceConstants.y;

	// Specular
	float3 viewVector_s = normalize(position - ViewPosition.xyz);
	float3 spotlightreflectVector = reflect(SpotLightDirection, surfaceNormal);
	float3 SpotSpecular = pow(saturate(dot(spotlightreflectVector, -viewVector_s)), SurfaceConstants.w) * spotDistAtt * spotAngularAtt * SpotLightColour.xyz * SurfaceConstants.z;

	return (SpotLightAmbience + SpotDiffuse + SpotSpecular);
}

float3 calculateSpotLight1(float3 surfaceNormal, float3 position)
{

	float spotDistance1 = distance(SpotLightPosition1, position);
	float spotDistAtt1 = saturate(1 - spotDistance1 / SpotLightRange1.x);
	float3 spotlight2PixelVector1 = normalize(position - SpotLightPosition1);
	float spotAngularAtt1 = saturate(pow(dot(spotlight2PixelVector1, SpotLightDirection1), SpotLightFocus1.x));

	// Ambience
	float3 SpotLightAmbience1 = SpotLightColour1.xyz * SurfaceConstants.x;

	// Diffuse
	float3 SpotDiffuse1 = saturate(dot(surfaceNormal, -SpotLightDirection1)) * spotDistAtt1 * spotAngularAtt1 * SpotLightColour1.xyz * SurfaceConstants.y;

	// Specular
	float3 viewVector_s = normalize(position - ViewPosition.xyz);
	float3 spotlightreflectVector1 = reflect(SpotLightDirection1, surfaceNormal);
	float3 SpotSpecular1 = pow(saturate(dot(spotlightreflectVector1, -viewVector_s)), SurfaceConstants.w) * spotDistAtt1 * spotAngularAtt1 * SpotLightColour1.xyz * SurfaceConstants.z;

	return (SpotLightAmbience1 + SpotDiffuse1 + SpotSpecular1);
}

float3 calculateSpotLight2(float3 surfaceNormal, float3 position)
{

	float spotDistance2 = distance(SpotLightPosition2, position);
	float spotDistAtt2 = saturate(1 - spotDistance2 / SpotLightRange2.x);
	float3 spotlight2PixelVector2 = normalize(position - SpotLightPosition2);
	float spotAngularAtt2 = saturate(pow(dot(spotlight2PixelVector2, SpotLightDirection2), SpotLightFocus2.x));

	// Ambience
	float3 SpotLightAmbience2 = SpotLightColour2.xyz * SurfaceConstants.x;

	// Diffuse
	float3 SpotDiffuse2 = saturate(dot(surfaceNormal, -SpotLightDirection2)) * spotDistAtt2 * spotAngularAtt2 * SpotLightColour2.xyz * SurfaceConstants.y;

	// Specular
	float3 viewVector_s = normalize(position - ViewPosition.xyz);
	float3 spotlightreflectVector2 = reflect(SpotLightDirection2, surfaceNormal);
	float3 SpotSpecular2 = pow(saturate(dot(spotlightreflectVector2, -viewVector_s)), SurfaceConstants.w) * spotDistAtt2 * spotAngularAtt2 * SpotLightColour2.xyz * SurfaceConstants.z;

	return (SpotLightAmbience2 + SpotDiffuse2 + SpotSpecular2);
}

//-----------------------------------------------------------------------------
VS_OUTPUT VSMain(in VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(float4(input.position, 1.0f), WorldViewProjMatrix);
	output.worldPos = mul(float4(input.position, 1.0f), WorldMatrix);
	output.normal = mul(float4(normalize(input.normal),1.0f), WorldMatrix);
	output.color = input.color;
	output.tex = input.tex;
	return output;
}

//-----------------------------------------------------------------------------
float4 PSMain(in VS_OUTPUT input) : SV_Target
{
	float3 worldPosition = input.worldPos.xyz;
	float3 normalVector = normalize(input.normal.xyz);

	float3 directionallightIntensity = calculateDirectionalLight(normalVector, worldPosition);
	float3 directionallightIntensity1 = calculateDirectionalLight1(normalVector, worldPosition);
	float3 directionallightIntensity2 = calculateDirectionalLight2(normalVector, worldPosition);

	float3 pointlightIntensity = calculatePointLight(normalVector, worldPosition);
	float3 pointlightIntensity1 = calculatePointLight1(normalVector, worldPosition);
	float3 pointlightIntensity2 = calculatePointLight2(normalVector, worldPosition);

	float3 spotlightIntensity = calculateSpotLight(normalVector, worldPosition);
	float3 spotlightIntensity1 = calculateSpotLight1(normalVector, worldPosition);
	float3 spotlightIntensity2 = calculateSpotLight2(normalVector, worldPosition);

	float3 lightIntensity3f = saturate(directionallightIntensity + pointlightIntensity + spotlightIntensity + SurfaceEmissiveColour.xyz);
	float3 lightIntensity3f1 = saturate(directionallightIntensity1 + pointlightIntensity1 + spotlightIntensity1 + SurfaceEmissiveColour.xyz);
	float3 lightIntensity3f2 = saturate(directionallightIntensity2 + pointlightIntensity2 + spotlightIntensity2 + SurfaceEmissiveColour.xyz);

	float4 lightIntensity4f = float4(lightIntensity3f, 1);
	float4 lightIntensity4f1 = float4(lightIntensity3f1, 1);
	float4 lightIntensity4f2 = float4(lightIntensity3f2, 1);

	float terrainheight = input.worldPos.y;
	float texweight = 1 / (1 + exp(terrainheight - TextureBoundry.x));

	float4 SampledTextureColour1 = (1 - texweight) * DiffuseTexture1.Sample(TextureSampler, input.tex);
	float4 SampledTextureColour2 = texweight * DiffuseTexture2.Sample(TextureSampler, input.tex);

	float4 SampledTextureColour = SampledTextureColour1 + SampledTextureColour2;

	float4 pixelcolour = SampledTextureColour * (lightIntensity4f + lightIntensity4f1 + lightIntensity4f2);

	return pixelcolour;

}