Texture2D       DiffuseTexture1 : register(t0);   
Texture2D       DiffuseTexture2 : register(t1);
SamplerState    TextureSampler : register( s0 );

cbuffer TransformMatrices
{
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
};

cbuffer Time
{
	float4 time;
};

[maxvertexcount(3)]
void GSMain(triangle GS_INPUTOUTPUT input[3], 
			inout TriangleStream<GS_INPUTOUTPUT> TriangleOutputStream)
{
	GS_INPUTOUTPUT output;

	float explosion_speed = 10;
	float explosion_radius = 80;
	float elapsed_time = time.x;
	float total_time = time.y;

	// calculate sine and cosine
	float c, s;
	sincos(total_time, s, c);
	s = saturate(s);

	// calculate the center of the triangle. This willbe the origin of the primitive space
	float3 centre = (input[0].position.xyz + input[1].position.xyz + input[2].position.xyz) / 3;

	float3 outwardvector = normalize(centre);

	// This is the current outward displacement of this primitive, i.e., how far from the centre
	float3 displacement = explosion_radius * outwardvector * s;

	// Output vertices
	for (int i = 0; i < 3; i++) {

		// calculate the vertex location in the primitive space		
		float4 position = input[i].position;

		// here we translate the vertex by the displacement vector
		position = float4(position.xyz + displacement, 1);

		// then we do the rest of the transforms
		output.position = mul(position, WorldViewProjMatrix);
		output.tex = input[i].tex;

		TriangleOutputStream.Append(output);
	}

	TriangleOutputStream.RestartStrip();
}
// ---- Addition End ---


VS_OUTPUT VSMain( in VS_INPUT input )
{
	VS_OUTPUT output;
	
	output.position = float4(input.position, 1); // ---- Addition ---
	output.tex = input.tex;

	return output;
}


float4 PSMain(in VS_OUTPUT input) : SV_Target
{
	float4 pixelcolour = 0;

	float4 pixelcolour1 = DiffuseTexture1.Sample(TextureSampler, input.tex);
	float4 pixelcolour2 = DiffuseTexture2.Sample(TextureSampler, input.tex);

	// Number of transitions/2
	int n = 3;
	
	// Method 1: Using fmod - clear separation of textures

	//float weight = fmod(round(input.tex.x * n * 2), 2*n);
	
	//
	//for (int i = 0; i < n; i++)
	//{
	//	int rep = i * 2;
	//	pixelcolour += (weight == rep) * pixelcolour1 + (weight == rep + 1) * pixelcolour2;
	//}

	// Method 2: Using trig - smooth transition of textures

	float pi = 3.1415f;
	float weight = (sin(input.tex.x * 2 * pi * n) + 1)/2;
	pixelcolour = pixelcolour1 * weight + pixelcolour2 * (1 - weight);
		
	return(pixelcolour);
}

