Texture2D       DiffuseTexture1 : register(t0);
Texture2D       DiffuseTexture2 : register(t1);
Texture2D       DiffuseTexture3 : register(t2);
Texture2D       DiffuseTexture4 : register(t3);
Texture2D       DiffuseTexture5 : register(t4);
Texture2D       DiffuseTexture6 : register(t5);
Texture2D       DiffuseTexture7 : register(t6);
Texture2D       DiffuseTexture8 : register(t7);
Texture2D       DiffuseTexture9 : register(t8);
Texture2D       DiffuseTexture10 : register(t9);
SamplerState    TextureSampler : register(s0);

cbuffer GSTransformMatrices
{
	matrix WorldMatrix;		// ---- Addition ---
	matrix ViewMatrix;		// ---- Addition ---
	matrix ProjMatrix;		// ---- Addition ---

	matrix WorldViewProjMatrix;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float2 tex		: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float2 tex		: TEXCOORD0;
};

struct GS_INPUTOUTPUT
{
	float4 position	: SV_Position;
	float2 tex		: TEXCOORD0;
	float2 textype	: TEXCOORD1;
};

cbuffer instance
{
	float4 instanceposition1;
	float4 instanceposition2;
	float4 instanceposition3;
	float4 instanceposition4;
	float4 instanceposition5;
	float4 instanceposition6;
	float4 instanceposition7;
	float4 instanceposition8;
	float4 instanceposition9;
	float4 instanceposition10;
};


VS_OUTPUT VSMAIN(in VS_INPUT input)
{
	VS_OUTPUT output;

	// output.position = mul(float4(input.position, 1.0f), WorldViewProjMatrix);
	output.position = float4(input.position,1);
	output.tex = input.tex;
	return output;
}

float4 PSMAIN(in GS_INPUTOUTPUT input) : SV_Target
{
	float4 pixelcolour;

	int textype = round(input.textype.x);

	if (textype == 0)
		pixelcolour = DiffuseTexture1.Sample(TextureSampler, input.tex);
	else if (textype == 1)
		pixelcolour = DiffuseTexture2.Sample(TextureSampler, input.tex);
	else if (textype == 2)
		pixelcolour = DiffuseTexture3.Sample(TextureSampler, input.tex);
	else if (textype == 3)
		pixelcolour = DiffuseTexture4.Sample(TextureSampler, input.tex);
	else if (textype == 4)
		pixelcolour = DiffuseTexture5.Sample(TextureSampler, input.tex);
	else if (textype == 5)
		pixelcolour = DiffuseTexture6.Sample(TextureSampler, input.tex);
	else if (textype == 6)
		pixelcolour = DiffuseTexture7.Sample(TextureSampler, input.tex);
	else if (textype == 7)
		pixelcolour = DiffuseTexture8.Sample(TextureSampler, input.tex);
	else if (textype == 8)
		pixelcolour = DiffuseTexture9.Sample(TextureSampler, input.tex);
	else if (textype == 9)
		pixelcolour = DiffuseTexture10.Sample(TextureSampler, input.tex);


	return pixelcolour;
}

float4x4 TransMatrix(float4 trans)
{
	return float4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		trans.x, trans.y, trans.z, 1
		);
}



[instance(10)]
[maxvertexcount(9)]
void GSMain(triangle GS_INPUTOUTPUT input[3],
			inout TriangleStream<GS_INPUTOUTPUT> TriangleOutputStream,
			uint InstanceID : SV_GSInstanceID)
{
	GS_INPUTOUTPUT output;
	float4 instancepositions[10];
	instancepositions[0] = instanceposition1;
	instancepositions[1] = instanceposition2;
	instancepositions[2] = instanceposition3;
	instancepositions[3] = instanceposition4;
	instancepositions[4] = instanceposition5;
	instancepositions[5] = instanceposition6;
	instancepositions[6] = instanceposition7;
	instancepositions[7] = instanceposition8;
	instancepositions[8] = instanceposition9;
	instancepositions[9] = instanceposition10;

	float4x4 transMatrix = TransMatrix(instancepositions[InstanceID]);

	for (int i = 0; i < 3; i++) {		
		float4 position = input[i].position;
		position = mul(position, WorldMatrix);
		position = mul(position, transMatrix);
		position = mul(position,ViewMatrix);
		output.position = mul(position, ProjMatrix);
		output.tex = input[i].tex;
		output.textype.x = InstanceID;
		output.textype.y = 0;
		TriangleOutputStream.Append(output);
	}

	TriangleOutputStream.RestartStrip();
}

//struct GS_INPUTOUTPUT
//{
//	float4 position	: SV_Position;
//	float2 tex		: TEXCOORD0;
//};
//
//[maxvertexcount(3)]
//void GSMain(triangle GS_INPUTOUTPUT input[3],
//	inout TriangleStream<GS_INPUTOUTPUT> TriangleOutputStream)
//{
//	GS_INPUTOUTPUT output;
//
//	for (int i = 0; i < 3; i++) {
//		float4 position = input[i].position;
//		position = mul(position, WorldViewProjMatrix);
//		output.position = position;
//		output.tex = input[i].tex;
//		TriangleOutputStream.Append(output);
//	}
//
//	TriangleOutputStream.RestartStrip();
//}