Texture2D       DiffuseTexture : register(t0);
SamplerState    TextureSampler : register(s0);

cbuffer WorldTransforms
{
	matrix WorldMatrix;
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
};

cbuffer DirectionalLightInfo
{
	float4 DirectionalLightColour;
	float3 DirectionalLightPosition;
	float3 DirectionalLightDirection;
};

cbuffer SpotLightInfo
{
	float4 SpotLightColour;
	float3 SpotLightPosition;
	float3 SpotLightDirection;
	float2 SpotLightRange;
	float2 SpotLightFocus;
};

cbuffer SceneInfo
{
	float4 ViewPosition;
};

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
	float3 normal   : NORMAL;
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

VS_OUTPUT VSMain(in VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(float4(input.position, 1.0f), WorldViewProjMatrix);
	output.worldPos = mul(float4(input.position, 1.0f), WorldMatrix);
	output.normal = mul(normalize(input.normal), WorldMatrix);
	output.tex = input.tex;
	return output;
}

float4 PSMain(in VS_OUTPUT input) : SV_Target
{
	float3 worldPosition = input.worldPos.xyz;
	float3 normalVector = normalize(input.normal);

	float3 directionallightIntensity = calculateDirectionalLight(normalVector, worldPosition);
	float3 pointlightIntensity = calculatePointLight(normalVector, worldPosition);
	float3 spotlightIntensity = calculateSpotLight(normalVector, worldPosition);

	float3 lightIntensity3f = directionallightIntensity + pointlightIntensity +	spotlightIntensity + SurfaceEmissiveColour.xyz;

	float4 lightIntensity4f = float4(lightIntensity3f, 1);

	float4 pixelcolour = DiffuseTexture.Sample(TextureSampler, input.tex) * lightIntensity4f;
	
	pixelcolour.x = saturate(pixelcolour.x);
	pixelcolour.y = saturate(pixelcolour.y);
	pixelcolour.z = saturate(pixelcolour.z);
	//pixelcolour.w = saturate(pixelcolour.w*3);

	return pixelcolour;
}