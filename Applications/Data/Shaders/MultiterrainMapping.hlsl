
Texture2D DiffuseTexture1 : register(t0);
Texture2D DiffuseTexture2 : register(t1);

SamplerState TextureSampler : register(s0);

cbuffer TerrainParams
{
	float4 TextureBoundry;
};

cbuffer WorldTransforms
{
	matrix WorldMatrix;
	matrix ViewMatrix;
	matrix ProjMatrix;
	matrix WorldViewProjMatrix;
};

//-----------------------------------------------------------------------------
struct VS_INPUT
{
	float4 position : POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;

	float4 worldpos : POSITION;
};

//-----------------------------------------------------------------------------
VS_OUTPUT VSMain(in VS_INPUT v)
{
	VS_OUTPUT o;

	float4 WorldSpace = mul(v.position, WorldMatrix);
	float4 ViewSpace = mul(WorldSpace, ViewMatrix);
	float4 ScreenSpace = mul(ViewSpace, ProjMatrix);

	o.position = ScreenSpace;
	o.color = v.color;

	o.worldpos = v.position;

	o.tex = v.tex;

	return o;
}

//-----------------------------------------------------------------------------
float4 PSMain(in VS_OUTPUT input) : SV_Target
{
	//float4 sampledTextureColour = DiffuseTexture.Sample(TextureSampler, input.tex);

	float terrainheight = input.worldpos.y;
	float texweight = 1 / (1 + exp(terrainheight - TextureBoundry.x));

	float4 SampledTextureColour1 = (1 - texweight) * DiffuseTexture1.Sample(TextureSampler, input.tex);
	float4 SampledTextureColour2 = texweight * DiffuseTexture2.Sample(TextureSampler, input.tex);

	float4 SampledTextureColour = SampledTextureColour1 + SampledTextureColour2;

	return(SampledTextureColour);
}