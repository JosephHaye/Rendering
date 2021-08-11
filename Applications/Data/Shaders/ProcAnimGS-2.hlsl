Texture2D       DiffuseTexture1 : register(t0);   
Texture2D       DiffuseTexture2 : register(t1);
SamplerState    TextureSampler : register( s0 );

cbuffer TransformMatrices1
{
	matrix WorldMatrix;		// ---- Addition ---
	matrix ViewMatrix;		// ---- Addition ---
	matrix ProjMatrix;		// ---- Addition ---

	matrix WorldViewProjMatrix;	
};

cbuffer instances
{
	float4 instanceposition;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float2 tex		: TEXCOORD0;
	float4 rotationdata		: COLOR;	// x:speed, yzw:axis ---- Addition ---
};

struct VS_OUTPUT
{
	float4 position			: SV_Position;
	float2 tex				: TEXCOORD0;
	float4 rotationdata		: COLOR;	// x:speed, yzw:axis ---- Addition ---
};

// ---- Addition Start ---
struct GS_INPUTOUTPUT
{
	float4 position	: SV_Position;
	float2 tex		: TEXCOORD0;
	float4 rotationdata		: COLOR;	// x:speed, yzw:axis ---- Addition ---
};

cbuffer Time
{
	float4 time;
};

float4x4 AngleAxis4x4(float angle, float3 axis)
{
	float c, s;
	sincos(angle, s, c);

	float t = 1 - c;
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	return float4x4(
		t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0,
		0, 0, 0, 1
		);
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


[instance(1)]
[maxvertexcount(3)]
void GSMain(triangle GS_INPUTOUTPUT input[3], inout TriangleStream<GS_INPUTOUTPUT> TriangleOutputStream, uint InstanceID : SV_GSInstanceID)
{
	GS_INPUTOUTPUT output;

	float4 instancepositions[1];
	instancepositions[0] = instanceposition;

	float4x4 transMatrix = TransMatrix(instanceposition[InstanceID]);

	float explosion_speed = 10;
	float explosion_radius = 80;
	float elapsed_time = time.x;
	float total_time = time.y;


	// calculate sine and cosine
	float c, s;
	sincos(total_time, s, c);
	s = saturate(s);
	c = saturate(c);

	// calculate the average rotation speed of the three vertices of the triangle
	float aveRotSpeed = (input[0].rotationdata.x + input[1].rotationdata.x + input[2].rotationdata.x) / 3;

	// calculate the average rotation axis of the three vertices of the triangle
	float3 axis = normalize(input[0].rotationdata.yzw + input[1].rotationdata.yzw + input[2].rotationdata.yzw);

	// get the rotation angle
	float rotAngle = aveRotSpeed * total_time * explosion_speed;

	rotAngle = rotAngle * s;

	// make sure it is within 0 - 2 x pi
	rotAngle = fmod(rotAngle, 2 * 3.141592);

	// calculate the rotation matrix - we modulate using the saturate(s) so that it starts opaque and 
	float4x4 rotMat = AngleAxis4x4(rotAngle, axis);





	// calculate the center of the triangle. This willbe the origin of the primitive space
	float3 centre = (input[0].position.xyz + input[1].position.xyz + input[2].position.xyz) / 3;

	float3 outwardvector = normalize(centre);
	outwardvector = normalize(outwardvector + axis);

	// This is the current outward displacement of this primitive, i.e., how far from the centre
	float3 displacement = explosion_radius * outwardvector * s * aveRotSpeed;
	
	// Output vertices
	for (int i = 0; i < 3; i++) {

		// calculate the vertex location in the primitive space		
		float4 position = input[i].position;

		// translate to origin
		position = float4(position.xyz - centre, 1);

		// Rotate the vertex using the calculated rotation matrix
		position = mul(position, rotMat);

		// translate the vertex back to its model space by offsetting by the primitive space origin
		position = float4(position.xyz + centre, 1);

		// here we translate the vertex by the displacement vector
		position = float4(position.xyz + displacement, 1);
		
		// then we do the rest of the transforms



		position = mul(position, WorldMatrix);
		position = mul(position, transMatrix);
		position = mul(position, ViewMatrix);
		output.position = mul(position, ProjMatrix);

		//output.position = mul(position, WorldViewProjMatrix);
		output.tex = input[i].tex;

		output.rotationdata = float4(0,0,0, c);		// ---- Addition ---

		TriangleOutputStream.Append(output);
	}

	TriangleOutputStream.RestartStrip();
}
// ---- Addition End ---


VS_OUTPUT VSMain( in VS_INPUT input )
{
	VS_OUTPUT output;
	
	output.position = float4(input.position, 1);

	output.tex = input.tex;

	output.rotationdata = input.rotationdata;		// ---- Addition ---

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

	pixelcolour.w = input.rotationdata.w;
		
	return(pixelcolour);
}

