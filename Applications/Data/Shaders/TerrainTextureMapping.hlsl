
Texture2D       DiffuseTexture : register(t0);
SamplerState    TextureSampler : register(s0);

//Texture2D       DiffuseTexture1 : register(t0);
//Texture2D       DiffuseTexture2 : register(t1);




cbuffer TransformMatrices
{
	matrix WorldViewProjMatrix;	
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
	
	output.position = mul(float4(input.position, 1.0f), WorldViewProjMatrix);
	output.worldpos = input.position;
	output.tex = input.tex;

	return output;
}


float4 PSMain(in VS_OUTPUT input) : SV_Target
{
	//float4 sampledTextureColour = DiffuseTexture.Sample(TextureSampler, input.tex );

	float terrainheight = input.worldpos.y;
	float texweight = 1 / (1 + exp(terrainheight - TerrainHeightBorders.x));

	float4 sampledTextureColour = DiffuseTexture.Sample(TextureSampler, input.tex);
	
	return(sampledTextureColour);
}

