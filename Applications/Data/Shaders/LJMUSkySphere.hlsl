Texture2D       SkysphereTexture : register( t0 );
SamplerState    TextureSampler : register( s0 );

cbuffer SkysphereTransformMatrices
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjMatrix;
	float4 ViewPosition;
};

cbuffer TerrainParams
{
	float2 TerrainHeightBorders;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float2 tex		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 worldpos	: POSITION;
	float2 tex		: TEXCOORD0;
};


VS_OUTPUT VSMain( in VS_INPUT input )
{
	VS_OUTPUT output;
	float4 positionWS = mul(float4(input.position, 1.0f), WorldMatrix);
	positionWS = float4(positionWS.xyz + ViewPosition.xyz, 1.0f);
	float4 positionVS = mul(positionWS, ViewMatrix);
	output.position = mul(positionVS, ProjMatrix);
	output.tex = input.tex * 10;
	return output;
}


float4 PSMain(in VS_OUTPUT input) : SV_Target
{
	float4 sampledTextureColour = SkysphereTexture.Sample(TextureSampler, input.tex);
	
	return(sampledTextureColour);
}

