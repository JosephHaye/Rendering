//------------Include the Application Header File----------------------------
#include "LJMULevelDemo.h"

//------------DX TK AND STD/STL Includes-------------------------------------
#include <sstream>

//------------Include Hieroglyph Engine Files--------------------------------

//Include the Logging System
#include "Log.h"

//Include the Event System
#include "EventManager.h"
#include "EvtFrameStart.h"
#include "EvtChar.h"
#include "EvtKeyUp.h"
#include "EvtKeyDown.h"
#include "ScriptManager.h"

//Include the DirectX Rendering Components
#include "PipelineManagerDX11.h"
#include "BlendStateConfigDX11.h"
#include "BufferConfigDX11.h"
#include "DepthStencilStateConfigDX11.h"
#include "RasterizerStateConfigDX11.h"
#include "SwapChainConfigDX11.h"
#include "Texture2dConfigDX11.h"
#include "MaterialGeneratorDX11.h"

#include "FirstPersonCamera.h"
#include <SamplerStateConfigDX11.h>

#include <FileSystem.h>
#include "LJMUMeshOBJ.h"
#include "FastNoise.h"

//Add a Using Directive to avoid typing Glyph3 for basic constructs
using namespace Glyph3;
//Include our own application Namespace
using namespace LJMUDX;

LJMULevelDemo AppInstance; 

//---------CONSTRUCTORS-------------------------------------------------------

///////////////////////////////////////
//
///////////////////////////////////////
LJMULevelDemo::LJMULevelDemo():
m_pRender_text(nullptr),
m_pRenderView(nullptr),
m_pCamera(nullptr),
m_pRenderer11(nullptr),
m_pWindow(nullptr),
m_iSwapChain(0),
m_DepthTarget(nullptr),
m_RenderTarget(nullptr)
{
	
}

BasicMeshPtr LJMULevelDemo::generateOBJMesh_mod(std::wstring pmeshname, Vector4f pmeshcolour)
{
	FileSystem fs;
	LJMUDX::LJMUMeshOBJ* tmesh = new LJMUDX::LJMUMeshOBJ(fs.GetModelsFolder() + pmeshname);
	int tvertcount = tmesh->positions.size();

	auto tia = std::make_shared<DrawExecutorDX11<BasicVertexDX11::Vertex>>();
	tia->SetLayoutElements(BasicVertexDX11::GetElementCount(), BasicVertexDX11::Elements);
	tia->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	tia->SetMaxVertexCount(tvertcount);

	BasicVertexDX11::Vertex tv;
	// tv.color = pmeshcolour;

	for (auto& tobject : tmesh->objects)
	{
		for (auto& tface : tobject.faces)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				tv.position = tmesh->positions[tface.PositionIndices[i]];
				tv.normal = tmesh->normals[tface.NormalIndices[i]];
				tv.texcoords = tmesh->coords[tface.CoordIndices[i]];

				float spinSpeed = (float)(rand() % 100) / 100;
				float spinDirX = (float)(rand() % 100) / 100;
				float spinDirY = (float)(rand() % 100) / 100;
				float spinDirZ = (float)(rand() % 100) / 100;
				tv.color = Vector4f(spinSpeed, spinDirX, spinDirY, spinDirZ);

				tia->AddVertex(tv);
			}
		}
	}
	return tia;
}

MaterialPtr LJMULevelDemo::createGSAnimMaterial()
{
	MaterialPtr material = MaterialPtr(new MaterialDX11());

	// Create and fill the effect that will be used for this view type
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	pEffect->SetVertexShader(m_pRenderer11->LoadShader(VERTEX_SHADER,
		std::wstring(L"ProcAnimGS-2.hlsl"),
		std::wstring(L"VSMain"),
		std::wstring(L"vs_4_0")));

	pEffect->SetPixelShader(m_pRenderer11->LoadShader(PIXEL_SHADER,
		std::wstring(L"ProcAnimGS-2.hlsl"),
		std::wstring(L"PSMain"),
		std::wstring(L"ps_4_0")));

	// Addition ---
	pEffect->SetGeometryShader(m_pRenderer11->LoadShader(GEOMETRY_SHADER,
		std::wstring(L"ProcAnimGS-2.hlsl"),
		std::wstring(L"GSMain"),
		std::wstring(L"gs_5_0")));
	// Addition ---

	ResourcePtr texture1 = RendererDX11::Get()->LoadTexture(std::wstring(L"earth.tif"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture1", texture1);

	ResourcePtr texture2 = RendererDX11::Get()->LoadTexture(std::wstring(L"mars.tif"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture2", texture2);

	SamplerStateConfigDX11 SamplerConfig;
	SamplerConfig.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerConfig.MaxAnisotropy = 0;

	int TextureSampler = RendererDX11::Get()->CreateSamplerState(&SamplerConfig);
	material->Parameters.SetSamplerParameter(L"TextureSampler", TextureSampler);

	BlendStateConfigDX11 blendConfig;
	blendConfig.AlphaToCoverageEnable = false;
	blendConfig.IndependentBlendEnable = false;
	for (int i = 0; i < 8; ++i)
	{
		blendConfig.RenderTarget[i].BlendEnable = true;
		blendConfig.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendConfig.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendConfig.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendConfig.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendConfig.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendConfig.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendConfig.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	pEffect->m_iBlendState = RendererDX11::Get()->CreateBlendState(&blendConfig);

	// Enable the material to render the given view type, and set its effect.
	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;

	material->Parameters.SetVectorParameter(L"instanceposition", Vector4f(1000, 0, 0, 1.0f));

	return material;
}

MaterialPtr LJMULevelDemo::createGSInstancingMaterial()
{
	MaterialPtr material = MaterialPtr(new MaterialDX11());

	// Create and fill the effect that will be used for this view type
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	pEffect->SetVertexShader(m_pRenderer11->LoadShader(VERTEX_SHADER,
		std::wstring(L"LJMUGSInstancing2.hlsl"),
		std::wstring(L"VSMAIN"),
		std::wstring(L"vs_4_0")));

	pEffect->SetPixelShader(m_pRenderer11->LoadShader(PIXEL_SHADER,
		std::wstring(L"LJMUGSInstancing2.hlsl"),
		std::wstring(L"PSMAIN"),
		std::wstring(L"ps_4_0")));

	pEffect->SetGeometryShader(m_pRenderer11->LoadShader(GEOMETRY_SHADER,
		std::wstring(L"LJMUGSInstancing2.hlsl"),
		std::wstring(L"GSMain"),
		std::wstring(L"gs_5_0")));

	ResourcePtr texture1 = RendererDX11::Get()->LoadTexture(std::wstring(L"earth.tif"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture1", texture1);

	ResourcePtr texture2 = RendererDX11::Get()->LoadTexture(std::wstring(L"mars.tif"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture2", texture2);

	ResourcePtr texture3 = RendererDX11::Get()->LoadTexture(std::wstring(L"sun.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture3", texture3);

	ResourcePtr texture4 = RendererDX11::Get()->LoadTexture(std::wstring(L"moon.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture4", texture4);

	ResourcePtr texture5 = RendererDX11::Get()->LoadTexture(std::wstring(L"saturn.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture5", texture5);

	ResourcePtr texture6 = RendererDX11::Get()->LoadTexture(std::wstring(L"mercury.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture6", texture6);

	ResourcePtr texture7 = RendererDX11::Get()->LoadTexture(std::wstring(L"uranus.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture7", texture7);

	ResourcePtr texture8 = RendererDX11::Get()->LoadTexture(std::wstring(L"venus.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture8", texture8);

	ResourcePtr texture9 = RendererDX11::Get()->LoadTexture(std::wstring(L"neptune.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture9", texture9);

	ResourcePtr texture10 = RendererDX11::Get()->LoadTexture(std::wstring(L"jupiter.jpg"));
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture10", texture10);

	SamplerStateConfigDX11 SamplerConfig;
	SamplerConfig.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerConfig.MaxAnisotropy = 0;

	int TextureSampler = RendererDX11::Get()->CreateSamplerState(&SamplerConfig);
	material->Parameters.SetSamplerParameter(L"TextureSampler", TextureSampler);

	BlendStateConfigDX11 blendConfig;
	blendConfig.AlphaToCoverageEnable = false;
	blendConfig.IndependentBlendEnable = false;
	for (int i = 0; i < 8; ++i)
	{
		blendConfig.RenderTarget[i].BlendEnable = true;
		blendConfig.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendConfig.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendConfig.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendConfig.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendConfig.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendConfig.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
		blendConfig.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	pEffect->m_iBlendState = RendererDX11::Get()->CreateBlendState(&blendConfig);

	// Enable the material to render the given view type, and set its effect.
	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;


	// ===========================
	material->Parameters.SetVectorParameter(L"instanceposition1", Vector4f(1000, 500, 0, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition2", Vector4f(1000, 500, 300, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition3", Vector4f(1000, 500, 600, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition4", Vector4f(1000, 500, 900, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition5", Vector4f(1000, 500, 1200, 1.0f)); 
	material->Parameters.SetVectorParameter(L"instanceposition6", Vector4f(1000, 500, 1500, 1.0f)); 
	material->Parameters.SetVectorParameter(L"instanceposition7", Vector4f(1000, 500, 1800, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition8", Vector4f(1000, 500, 2100, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition9", Vector4f(1000, 500, 2400, 1.0f));
	material->Parameters.SetVectorParameter(L"instanceposition10", Vector4f(1000, 500, 2700, 1.0f));

	return material;
}

Vector3f CatmullRom(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f p3, float t)
{
	Vector3f cm0 = p1;
	Vector3f cm1 = (p2 - p0) * 0.5f;
	Vector3f cm2 = (p0 * 2 - p1 * 5 + p2 * 4 - p3) * 0.5f;
	Vector3f cm3 = ((p1 - p2) * 3 + p3 - p0) * 0.5f;

	float t2 = t * t;
	float t3 = t2 * t;

	Vector3f result;

	result = cm0 + cm1 * t + cm2 * t2 + cm3 * t3;

	return result;
}

BasicMeshPtr LJMULevelDemo::createSpaceShipCaatmullRomPath()
{
	float center_x = 1250.0f;
	float center_y = 1250.0f;
	float radius = 1000.0f;
	float height = 256.0f;
	float start = -180;
	float end = 180;
	float increment = 15;

	std::vector<Vector3f> CatmullRomPoints;

	for (int i = start; i < end; i = i + increment)
	{
		float x = center_x + radius * cos(i * DEG_TO_RAD);
		float z = center_y + radius * sin(i * DEG_TO_RAD);
		float y = height + 100 * cos(i * DEG_TO_RAD * 10);
		CatmullRomPoints.push_back(Vector3f(x, y, z));
	}

	int numPoints = CatmullRomPoints.size();

	for (int pt = 1; pt <= numPoints; pt++)
	{
		int p0 = (pt - 1);
		int p1 = pt % numPoints;
		int p2 = (pt + 1) % numPoints;
		int p3 = (pt + 2) % numPoints;
		int numInterpoints = 75;

		for (int i = 0; i < numInterpoints; i++)
		{
			float time = ((float)i) / numInterpoints;
			Vector3f pt = CatmullRom(CatmullRomPoints[p0], CatmullRomPoints[p1], CatmullRomPoints[p2], CatmullRomPoints[p3], time);
			m_checkpoints.push_back(pt);
		}
	}

	auto pathMesh = std::make_shared<DrawExecutorDX11<BasicVertexDX11::Vertex>>();
	pathMesh->SetLayoutElements(BasicVertexDX11::GetElementCount(), BasicVertexDX11::Elements);
	pathMesh->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	pathMesh->SetMaxVertexCount(m_checkpoints.size());

	BasicVertexDX11::Vertex tv;

	for (int i = 0; i < m_checkpoints.size() + 1; i++)
	{
		int j = i % m_checkpoints.size();
		tv.position = m_checkpoints[j];
		tv.color = Vector4f(1, 0, 0, 1);
		pathMesh->AddVertex(tv);
	}

	CatmullRomPoints.clear();

	return pathMesh;

}

void LJMULevelDemo::moveSpaceShip()
{

	// #13: Get the elapsed time since the last update
	float tpf = m_pTimer->Elapsed();

	if (tpf > 10.0f / 60.0f)
	{
		// if this is triggered, most likely the user is debugging the code, 
		// hence time per frame is much larger than 10x 60fps then

		tpf = 1 / 60.0f;
	}

	// #14: When the spacehip is moving towards the next checkpoint
	if (m_spaceshipState == 0)
	{
		m_spaceshipTargetDirection = m_checkpoints[m_nextCheckpointID] - m_spaceshipActor->GetNode()->Position();
		float cur_dist = m_spaceshipTargetDirection.Magnitude();
		m_spaceshipTargetDirection.Normalize();
		m_spaceshipDirection = m_spaceshipTargetDirection;

		if (cur_dist > m_spaceshipPrevDist2Target)
		{
			m_currentCheckpointID++;
			m_nextCheckpointID++;
			m_currentCheckpointID = m_currentCheckpointID % m_checkpoints.size();
			m_nextCheckpointID = m_nextCheckpointID % m_checkpoints.size();
			m_spaceshipActor->GetNode()->Position() = m_checkpoints[m_currentCheckpointID];

			m_spaceshipTargetDirection = m_checkpoints[m_nextCheckpointID] - m_spaceshipActor->GetNode()->Position();
			cur_dist = m_spaceshipTargetDirection.Magnitude();

			m_spaceshipPrevDist2Target = cur_dist;

			m_spaceshipTargetDirection.Normalize();
			float angle = acos(m_spaceshipDirection.Dot(m_spaceshipTargetDirection));
			if (angle > m_spaceshipRotationSpeed * tpf)
			{
				m_spaceshipState = 1;
			}
		}
		else
		{
			m_spaceshipActor->GetNode()->Position() = m_spaceshipActor->GetNode()->Position() +
				m_spaceshipDirection *
				m_spaceshipMovementSpeed * tpf;
			m_spaceshipPrevDist2Target = cur_dist;
		}
	}

	// #15: When the spacehip is orienting to face the next checkpoint
	else
	{
		float angle = acos(m_spaceshipDirection.Dot(m_spaceshipTargetDirection));
		if (fabs(angle) < m_spaceshipRotationSpeed * tpf)
		{
			Vector3f axis = m_spaceshipDirection.Cross(m_spaceshipTargetDirection);
			axis.Normalize();

			Matrix3f tstartRotation;
			tstartRotation.RotationEuler(axis, angle);
			m_spaceshipActor->GetNode()->Rotation() *= tstartRotation;

			m_spaceshipTargetDirection = m_checkpoints[m_nextCheckpointID] - m_spaceshipActor->GetNode()->Position();
			m_spaceshipPrevDist2Target = m_spaceshipTargetDirection.Magnitude();

			m_spaceshipState = 0;
		}
		else
		{
			angle = angle + m_spaceshipRotationSpeed * tpf;
			Vector3f axis = m_spaceshipDirection.Cross(m_spaceshipTargetDirection);
			axis.Normalize();

			Matrix3f tstartRotation;
			tstartRotation.RotationEuler(axis, m_spaceshipRotationSpeed * tpf);
			m_spaceshipActor->GetNode()->Rotation() *= tstartRotation;
			m_spaceshipDirection = m_spaceshipActor->GetNode()->Rotation() * m_spaceshipRefDirection;
			m_spaceshipDirection.Normalize();
		}
	}

}

double* LJMULevelDemo::LoadRawHeightMap(const char* filename, int actualTerrainWidth, int actualTerrainLength, int terrainLength, int terrainWidth)
{
	int imageSize = terrainLength * terrainWidth;
	int actualSize = actualTerrainWidth * actualTerrainLength;

	unsigned short* rawImage = new unsigned short[actualSize];
	if (!rawImage)
	{
		return nullptr;
	}

	double* heightmap = new double[imageSize];
	if (!heightmap)
	{
		return nullptr;
	}

	FILE* fileptr;

	int error = fopen_s(&fileptr, filename, "rb");
	if (error != 0)
	{
		return nullptr;
	}

	int count = fread(rawImage, sizeof(unsigned short), actualSize, fileptr);
	if (count != actualSize)
	{
		return nullptr;
	}

	error = fclose(fileptr);
	if (error != 0)
	{
		return nullptr;
	}

	double maxHeight = 0;
	double minHeight = 100000;

	int i, j, index;

	for (j = 0; j < terrainLength; j++)
	{
		for (i = 0; i < terrainWidth; i++)
		{
			int index = (terrainWidth * j) + i;
			int actuialIndex = (actualTerrainWidth * j * 2) + i * 2;

			heightmap[index] = (double)(rawImage[actuialIndex]);
			if (maxHeight < heightmap[index]) maxHeight = heightmap[index];
			if (minHeight > heightmap[index]) minHeight = heightmap[index];

		}
	}

	double range = maxHeight - minHeight;

	for (i = 0; i < imageSize; i++)
	{
		heightmap[i] = (heightmap[i] - minHeight) / range;
	}

	delete[] rawImage;
	rawImage = 0;

	return heightmap;
}

BasicMeshPtr LJMULevelDemo::CreateTerrainMeshFromFile(const char* filename, int actualTerrainWidth, int actualTerrainLength)
{
	// Variable Declarations
	float textureMappingFactor = 0.1f;

	float heightScale = 512.0f;

	int terrainWidth = 512;
	int terrainLength = 512;

	std::vector<Vector3f> vertices;
	std::vector<Vector4f> vertexColors;
	std::vector<Vector3f> normal;

	float majorheightfrequency = 5;
	float majorheight = 1.0f;
	
	float spacing = 10.0f;

	float minorheightfrequency = 75;
	float minorheight = 0.25f;

	double* heightmap = LoadRawHeightMap(filename, actualTerrainWidth, actualTerrainLength, terrainLength, terrainWidth);

	for (int i = 0; i < terrainWidth; i++)
	{
		for (int p = 0; p < terrainLength; p++)
		{

			float height = heightmap[i * terrainLength + p] * heightScale;

			float shade = (height / heightScale + 1) / 2.0f;

			if (shade < 0)
				shade = 0;
			else if (shade > 1)
				shade = 1;

			m_vTextureBoundry = Vector4f(0.5f, 0, 0, 0) * heightScale;

			vertices.push_back(Vector3f(i * spacing, height, p * spacing));

			vertexColors.push_back(Vector4f(shade, 1 - shade, shade / 2, 1));

			textureCoords.push_back(Vector2f(i * textureMappingFactor, p * textureMappingFactor));
			
			normal.push_back(Vector3f(0.0f, 1.0f, 0.0f));
		}
	}

	std::vector<int> indices;

	CreateTerrainIndexArray(terrainWidth, terrainLength, true, indices);

	int numberOfIndices = indices.size();
	int numberOfLengths = numberOfIndices / 3;

	auto terrainMesh = std::make_shared<DrawExecutorDX11<BasicVertexDX11::Vertex>>();
	terrainMesh->SetLayoutElements(BasicVertexDX11::GetElementCount(), BasicVertexDX11::Elements);
	terrainMesh->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	terrainMesh->SetMaxVertexCount(numberOfIndices);

	BasicVertexDX11::Vertex tv;

	for (int i = 0; i < numberOfIndices; i++)
	{
		tv.position = vertices[indices[i]];
		tv.color = vertexColors[indices[i]];
		tv.texcoords = textureCoords[indices[i]];
		tv.normal = normal[indices[i]];

		terrainMesh->AddVertex(tv);
	}


	return terrainMesh;
}

BasicMeshPtr LJMULevelDemo::generateOBJMesh(std::wstring pmeshname, Vector4f pmeshcolour) 
{
	FileSystem fs;
	LJMUDX::LJMUMeshOBJ* tmesh = new LJMUDX::LJMUMeshOBJ(fs.GetModelsFolder() + pmeshname);
	int tvertcount = tmesh->positions.size();

	auto tia = std::make_shared<DrawExecutorDX11<BasicVertexDX11::Vertex>>();
	tia->SetLayoutElements(BasicVertexDX11::GetElementCount(), BasicVertexDX11::Elements);
	tia->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	tia->SetMaxVertexCount(tvertcount);

	BasicVertexDX11::Vertex tv;
	tv.color = pmeshcolour;

	for (auto& tobject : tmesh->objects)
	{
		for (auto& tface : tobject.faces)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				tv.position = tmesh->positions[tface.PositionIndices[i]];
				tv.normal = tmesh->normals[tface.NormalIndices[i]];
				tv.texcoords = tmesh->coords[tface.CoordIndices[i]];
				tia->AddVertex(tv);
			}
		}
	}
	return tia;
}

MaterialPtr LJMULevelDemo::createPathMaterial()
{

	MaterialPtr material = MaterialPtr(new MaterialDX11());
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	// -- Setup shader here
	pEffect->SetVertexShader(m_pRenderer11->LoadShader(VERTEX_SHADER,
		std::wstring(L"Basic.hlsl"),
		std::wstring(L"VSMain"),
		std::wstring(L"vs_4_0"),
		true));

	pEffect->SetPixelShader(m_pRenderer11->LoadShader(PIXEL_SHADER,
		std::wstring(L"Basic.hlsl"),
		std::wstring(L"PSMain"),
		std::wstring(L"ps_4_0"),
		true));

	RasterizerStateConfigDX11 rsConfig;
	rsConfig.CullMode = D3D11_CULL_NONE;
	rsConfig.FillMode = D3D11_FILL_SOLID;

	int rasterizerState = m_pRenderer11->CreateRasterizerState(&rsConfig);
	if (rasterizerState == -1) {
		Log::Get().Write(L"Failed to create light rasterizer state");
		assert(false);
	}

	pEffect->m_iRasterizerState = rasterizerState;

	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;

	return material;
}

MaterialPtr LJMULevelDemo::createTextureMaterial(std::wstring texturefilename)
{
	MaterialPtr material = MaterialPtr(new MaterialDX11());

	// Create and fill the effect that will be used for this view type
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	pEffect->SetVertexShader(m_pRenderer11->LoadShader(VERTEX_SHADER,
		std::wstring(L"TextureMapping.hlsl"),
		std::wstring(L"VSMain"),
		std::wstring(L"vs_4_0")));

	pEffect->SetPixelShader(m_pRenderer11->LoadShader(PIXEL_SHADER,
		std::wstring(L"TextureMapping.hlsl"),
		std::wstring(L"PSMain"),
		std::wstring(L"ps_4_0")));

	ResourcePtr texture = RendererDX11::Get()->LoadTexture(texturefilename);
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture", texture);

	SamplerStateConfigDX11 SamplerConfig;
	SamplerConfig.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerConfig.MaxAnisotropy = 0;

	int TextureSampler = RendererDX11::Get()->CreateSamplerState(&SamplerConfig);
	material->Parameters.SetSamplerParameter(L"TextureSampler", TextureSampler);

	// Enable the material to render the given view type, and set its effect.
	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;

	return material;
}

MaterialPtr LJMULevelDemo::createSkySphereMaterial()
{
	MaterialPtr material = MaterialPtr(new MaterialDX11());

	// Create and fill the effect that will be used for this view type
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	pEffect->SetVertexShader(RendererDX11::Get()->LoadShader(VERTEX_SHADER,
		std::wstring(L"LJMUSkySphere.hlsl"),
		std::wstring(L"VSMain"),
		std::wstring(L"vs_4_0")));

	pEffect->SetPixelShader(RendererDX11::Get()->LoadShader(PIXEL_SHADER,
		std::wstring(L"LJMUSkySphere.hlsl"),
		std::wstring(L"PSMain"),
		std::wstring(L"ps_4_0")));

	ResourcePtr texture = RendererDX11::Get()->LoadTexture(std::wstring(L"stars.tif"));
	material->Parameters.SetShaderResourceParameter(L"SkysphereTexture", texture);

	SamplerStateConfigDX11 SamplerConfig;
	SamplerConfig.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerConfig.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerConfig.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerConfig.MaxAnisotropy = 0;

	int TextureSampler = RendererDX11::Get()->CreateSamplerState(&SamplerConfig);
	material->Parameters.SetSamplerParameter(L"TextureSampler", TextureSampler);

	RasterizerStateConfigDX11 rsConfig;
	rsConfig.CullMode = D3D11_CULL_FRONT;
	int iRasterizerState = m_pRenderer11->CreateRasterizerState(&rsConfig);
	if (iRasterizerState == -1) {
		Log::Get().Write(L"Failed to create light rasterizer state");
		assert(false);
	}
	pEffect->m_iRasterizerState = iRasterizerState;

	// Enable the material to render the given view type, and set its effect.
	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;

	return material;
}

//---------METHODS------------------------------------------------------------
void LJMULevelDemo::setupCamera()
{

	// #8: Move the camera to near and above the spaceship
	Vector3f pos = m_spaceshipActor->GetNode()->Position();
	pos.y += 128;
	m_cameraPositions.push_back(pos);

	m_icpindex = 0;
	m_cameraPosition = m_cameraPositions[m_icpindex];

	m_pCamera = new FirstPersonCamera();
	m_pCamera->SetEventManager(&EvtManager);

	m_pCamera->Spatial().SetTranslation(m_cameraPosition);
	m_pCamera->Spatial().RotateXBy(45 * DEG_TO_RAD);

	m_pRenderView = new ViewPerspective(*m_pRenderer11,
		m_RenderTarget,
		m_DepthTarget);
	m_pRenderView->SetBackColor(Vector4f(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCamera->SetCameraView(m_pRenderView);

	m_pRender_text = new LJMUTextOverlay(*m_pRenderer11,
		m_RenderTarget,
		std::wstring(L"Cambria"),
		25);
	m_pCamera->SetOverlayView(m_pRender_text);

	m_pCamera->SetProjectionParams(0.1f,
		1000000.0f,
		m_iscreenWidth / m_iscreenHeight,
		static_cast<float>(GLYPH_PI) / 2.0f);

	m_pScene->AddCamera(m_pCamera);
}

void LJMULevelDemo::setupViewMatrix()
{
	// Create the view matrix
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

}

void LJMULevelDemo::setupProjMatrix()
{
	// Create the projection matrix	
	DirectX::XMMATRIX projMatrix;
	projMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2,
		(FLOAT)m_pWindow->GetWidth() / (FLOAT)m_pWindow->GetHeight(),
		0.01f,
		100.0f);
}

MaterialPtr LJMULevelDemo::createStaticMaterial(std::wstring texturefilename)
{
	MaterialPtr material = MaterialPtr(new MaterialDX11());

	// Create and fill the effect that will be used for this view type
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	pEffect->SetVertexShader(m_pRenderer11->LoadShader(VERTEX_SHADER,
		std::wstring(L"TextureMapping.hlsl"),
		std::wstring(L"VSMain"),
		std::wstring(L"vs_4_0")));

	pEffect->SetPixelShader(m_pRenderer11->LoadShader(PIXEL_SHADER,
		std::wstring(L"TextureMapping.hlsl"),
		std::wstring(L"PSMain"),
		std::wstring(L"ps_4_0")));

	ResourcePtr texture = RendererDX11::Get()->LoadTexture(texturefilename);
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture", texture);

	SamplerStateConfigDX11 SamplerConfig;
	SamplerConfig.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerConfig.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerConfig.MaxAnisotropy = 0;

	int TextureSampler = RendererDX11::Get()->CreateSamplerState(&SamplerConfig);
	material->Parameters.SetSamplerParameter(L"TextureSampler", TextureSampler);

	// Enable the material to render the given view type, and set its effect.
	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;

	return material;
}

void LJMULevelDemo::setMaterialSurfaceProperties(MaterialPtr material, Vector4f surfaceConstants, Vector4f surfaceEmissiveColour)
{
	material->Parameters.SetVectorParameter(L"SurfaceConstants", surfaceConstants);
	material->Parameters.SetVectorParameter(L"SurfaceEmissiveColour", surfaceEmissiveColour);
}

void LJMULevelDemo::setupGeometry()
{
	m_terrainMaterial = createMultiTerrainTextureMaterial(L"TerrainGrass.TIF", L"SnowScuffedGround.TIF");
	
	m_vSurfaceConstants = Vector4f(0.0f, 1.0f, 1.0f, 20.0f);
	m_vSurfaceEmissiveColour = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);

	setMaterialSurfaceProperties(m_terrainMaterial, m_vSurfaceConstants, m_vSurfaceEmissiveColour);
	
	setLightsParameters();
	setLights2Material(m_terrainMaterial);

	int terrainResolution = 512;
	int terrainSpacing = 10;
	int terrainOffsetX;
	int terrainOffsetZ;
	terrainOffsetX = 0;
	terrainOffsetZ = 0;
	int actualTerrainWidth = 1025;
	int actualTerrainLength = 1025;
			
	auto cubeMesh1 = CreateTerrainMeshFromFile("heightmap.r16", actualTerrainWidth, actualTerrainLength);

	auto cubeMesh = CreateTerrainMeshFromNoiseFunction(terrainOffsetX, terrainOffsetZ, terrainResolution, terrainSpacing);
			
	terrainSegmentActor = new Actor();
	terrainSegmentActor->GetBody()->SetGeometry(cubeMesh);

	terrainSegmentActor->GetBody()->SetMaterial(m_terrainMaterial);
	terrainSegmentActor->GetNode()->Position() = Vector3f(0.0f, 0.0f, 0.0f);
	terrainSegmentActor->GetNode()->Scale() = Vector3f(1, 1, 1);

	m_pTerrianActors.push_back(terrainSegmentActor);
	m_pScene->AddActor(terrainSegmentActor);

	terrainSegmentActor1 = new Actor();
	terrainSegmentActor1->GetBody()->SetGeometry(cubeMesh1);

	terrainSegmentActor1->GetBody()->SetMaterial(m_terrainMaterial);
	terrainSegmentActor1->GetNode()->Position() = Vector3f(0.0f, 0.0f, 0.0f);
	terrainSegmentActor1->GetNode()->Scale() = Vector3f(1, 1, 1);

	m_pTerrianActors1.push_back(terrainSegmentActor1);
	m_pScene->AddActor(terrainSegmentActor1);

	m_pScene->RemoveActor(terrainSegmentActor1);
	

	Vector4f spaceshipBaseColour = Vector4f(1, 1, 1, 1);
	BasicMeshPtr spaceshipGeometry = this->generateOBJMesh(L"spaceship.obj", spaceshipBaseColour);
	MaterialPtr spaceshipMaterial = createTextureMaterial(L"wedge_p1_diff_v1.png");

	m_spaceshipActor = new Actor();
	m_spaceshipActor->GetBody()->SetGeometry(spaceshipGeometry);
	m_spaceshipActor->GetBody()->SetMaterial(spaceshipMaterial);
	m_spaceshipActor->GetNode()->Position() = Vector3f(1024.0f, 256.0f, 1024.0f);
	m_spaceshipActor->GetNode()->Scale() = Vector3f(0.05f, 0.05f, 0.05f);
	this->m_pScene->AddActor(m_spaceshipActor);

	// #5: Creating and setting up the actor for the spaceship's path
	BasicMeshPtr path_geometry = createSpaceShipCaatmullRomPath();
	MaterialPtr path_material = createPathMaterial();
	m_pathActor = new Actor();
	m_pathActor->GetBody()->SetGeometry(path_geometry);
	m_pathActor->GetBody()->SetMaterial(path_material);
	m_pathActor->GetNode()->Position() = Vector3f(0.0f, 0.0f, 0.0f);
	m_pathActor->GetNode()->Scale() = Vector3f(1.0f, 1.0f, 1.0f);
	this->m_pScene->AddActor(m_pathActor);

	// #6: Placing the spaceship at the start checkpoint and facing the next checkpoint
	m_currentCheckpointID = 0;
	m_nextCheckpointID = m_currentCheckpointID + 1;
	m_spaceshipState = 0;

	Matrix3f tstartRotation;
	m_spaceshipRefDirection = Vector3f(-1, 0, 0);
	m_spaceshipDirection = m_spaceshipRefDirection;
	m_spaceshipTargetDirection = m_checkpoints[m_nextCheckpointID] - m_checkpoints[m_currentCheckpointID];
	m_spaceshipPrevDist2Target = m_spaceshipTargetDirection.Magnitude();
	m_spaceshipTargetDirection.Normalize();

	float angle = acos(m_spaceshipRefDirection.Dot(m_spaceshipTargetDirection));

	Vector3f axis = m_spaceshipRefDirection.Cross(m_spaceshipTargetDirection);
	axis.Normalize();

	tstartRotation.RotationEuler(axis, angle);

	m_spaceshipActor->GetNode()->Rotation() = tstartRotation;
	m_spaceshipActor->GetNode()->Position() = m_checkpoints[m_currentCheckpointID];

	// #7: Set the spaceship properties
	m_spaceshipMovementSpeed = 160.0f;
	m_spaceshipRotationSpeed = 1.0f;
	m_spaceshipState = 0;

	skysphere_geometry = generateOBJMesh(L"geosphere.obj", Vector4f(1, 1, 1, 1));
	skysphere_material = createSkySphereMaterial();
	skysphereActor = new Actor();

	Node3D* skysphere_node = skysphereActor->GetNode();

	skysphere_entity = new Glyph3::Entity3D();
	skysphere_entity->SetGeometry(skysphere_geometry);
	skysphere_entity->SetMaterial(skysphere_material);
	skysphere_entity->Scale() = Vector3f(10000.0f, 10000.0f, 10000.0f);
	skysphere_node->AttachChild(skysphere_entity);

	m_pScene->AddActor(skysphereActor);

	planet_geometry1 = generateOBJMesh(L"geosphere.obj", Vector4f(1, 1, 1, 1));

	planet_geometry = generateOBJMesh_mod(L"geosphere.obj", Vector4f(1, 1, 1, 1));
	
	planet_material1 = createGSAnimMaterial();

	planet_material = createGSInstancingMaterial();

	m_pActor = new Actor();
	Node3D* rootnode = m_pActor->GetNode();

	planet_node = new Node3D();
	rootnode->AttachChild(planet_node);

	planet_entity = new Glyph3::Entity3D();
	planet_entity->SetGeometry(planet_geometry);
	planet_entity->SetMaterial(planet_material);
	planet_node->AttachChild(planet_entity);

	planet_entity = new Glyph3::Entity3D();
	planet_entity->SetGeometry(planet_geometry1);
	planet_entity->SetMaterial(planet_material1);
	planet_node->AttachChild(planet_entity);

	m_pScene->AddActor(m_pActor);

	planet_rotation = 0;

	Vector4f HumanColour = Vector4f(0.0f, 1.0f, 1.0f, 1.0f);
	Vector3f HumanPos = Vector3f(100.0f, 100.0, 900.0);
	Vector3f HumanScale = Vector3f(1, 1, 1);

	auto HumanMesh = generateOBJMesh(L"human.obj", HumanColour);
	Actor* HumanActor = new Actor();
	HumanActor->GetBody()->SetGeometry(HumanMesh);
	MaterialPtr HumanMaterial = createStaticMaterial(L"human_diff.PNG");
	HumanActor->GetBody()->SetMaterial(HumanMaterial);
	HumanActor->GetBody()->Position() = HumanPos;
	HumanActor->GetNode()->Scale() = HumanScale;
	m_pScene->AddActor(HumanActor);

	Vector4f RexColour = Vector4f(0.0f, 1.0f, 1.0f, 1.0f);
	Vector3f RexPos = Vector3f(300.0, 50.0, 900.0);
	Vector3f RexScale = Vector3f(1, 1, 1);

	auto RexMesh = generateOBJMesh(L"trex.obj", RexColour);
	Actor* RexActor = new Actor();
	RexActor->GetBody()->SetGeometry(RexMesh);
	MaterialPtr RexMaterial = createStaticMaterial(L"trex_diff.PNG");
	RexActor->GetBody()->SetMaterial(RexMaterial);
	RexActor->GetBody()->Position() = RexPos;
	RexActor->GetNode()->Scale() = RexScale;
	m_pScene->AddActor(RexActor);

	Vector4f BullColour = Vector4f(0.0f, 1.0f, 1.0f, 1.0f);
	Vector3f BullPos = Vector3f(500.0, 50.0, 900.0);
	Vector3f BullScale = Vector3f(1, 1, 1);

	auto BullMesh = generateOBJMesh(L"bull.obj", BullColour);
	Actor* BullActor = new Actor();
	BullActor->GetBody()->SetGeometry(BullMesh);
	MaterialPtr BullMaterial = createStaticMaterial(L"bull_diff.PNG");
	BullActor->GetBody()->SetMaterial(BullMaterial);
	BullActor->GetBody()->Position() = BullPos;
	BullActor->GetNode()->Scale() = BullScale;
	m_pScene->AddActor(BullActor);

	Vector4f ReptileColour = Vector4f(0.0f, 1.0f, 1.0f, 1.0f);
	Vector3f ReptilePos = Vector3f(700.0, 50.0, 900.0);
	Vector3f ReptileScale = Vector3f(1, 1, 1);

	auto ReptileMesh = generateOBJMesh(L"reptile.obj", ReptileColour);
	Actor* ReptileActor = new Actor();
	ReptileActor->GetBody()->SetGeometry(ReptileMesh);
	MaterialPtr ReptileMaterial = createStaticMaterial(L"reptile_diff_alt.PNG");
	ReptileActor->GetBody()->SetMaterial(BullMaterial);
	ReptileActor->GetBody()->Position() = ReptilePos;
	ReptileActor->GetNode()->Scale() = ReptileScale;
	m_pScene->AddActor(ReptileActor);

	Vector4f GeoSphereColour = Vector4f(0.0f, 1.0f, 1.0f, 1.0f);
	Vector3f GeoSpherePos = Vector3f(0.0f, 0.0f, 0.0f);
	Vector3f GeoSphereScale = Vector3f(5, 5, 5);

	auto GeoSphereMesh = generateOBJMesh(L"geosphere.obj", GeoSphereColour);
	Actor* GeoSphereActor = new Actor();
	GeoSphereActor->GetBody()->SetGeometry(GeoSphereMesh);
	MaterialPtr GeoSphereMaterial = createGSInstancingMaterial();
	GeoSphereActor->GetBody()->SetMaterial(GeoSphereMaterial);
	GeoSphereActor->GetBody()->Position() = GeoSpherePos;
	GeoSphereActor->GetNode()->Scale() = GeoSphereScale;
	m_pScene->AddActor(GeoSphereActor);

}

MaterialPtr LJMULevelDemo::createMultiTerrainTextureMaterial(std::wstring textureFile1, std::wstring texturefile2)
{

	MaterialPtr material = MaterialPtr(new MaterialDX11());
	RenderEffectDX11* pEffect = new RenderEffectDX11();

	pEffect->SetVertexShader(m_pRenderer11->LoadShader(VERTEX_SHADER, std::wstring(L"MultiterrainMappingAndLighting.hlsl"), std::wstring(L"VSMain"), std::wstring(L"vs_4_0"), true));

	pEffect->SetPixelShader(m_pRenderer11->LoadShader(PIXEL_SHADER, std::wstring(L"MultiterrainMappingAndLighting.hlsl"), std::wstring(L"PSMain"), std::wstring(L"ps_4_0"), true));

	material->Parameters.SetVectorParameter(std::wstring(L"TextureBoundry"), m_vTextureBoundry);

	ResourcePtr texture1 = RendererDX11::Get()->LoadTexture(textureFile1);
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture1", texture1);

	ResourcePtr texture2 = RendererDX11::Get()->LoadTexture(texturefile2);
	material->Parameters.SetShaderResourceParameter(L"DiffuseTexture2", texture2);

	SamplerStateConfigDX11 SamplerConfig;
	SamplerConfig.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerConfig.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerConfig.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerConfig.MaxAnisotropy = 0;

	int TextureSampler = RendererDX11::Get()->CreateSamplerState(&SamplerConfig);
	material->Parameters.SetSamplerParameter(L"TextureSampler", TextureSampler);

	DepthStencilStateConfigDX11 dsConfig;
	int iDepthStencilState = m_pRenderer11->CreateDepthStencilState(&dsConfig);
	if (iDepthStencilState == -1)
	{
		Log::Get().Write(L"Failed to create light depth stencil state");
		assert(false);
	}
	pEffect->m_iDepthStencilState = iDepthStencilState;
	pEffect->m_uStencilRef = iDepthStencilState;

	BlendStateConfigDX11 blendConfig;
	blendConfig.AlphaToCoverageEnable = false;
	blendConfig.IndependentBlendEnable = false;
	for (int i = 0; i < 8; ++i)
	{
		blendConfig.RenderTarget[i].BlendEnable = true;
		blendConfig.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendConfig.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendConfig.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendConfig.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendConfig.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendConfig.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendConfig.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	pEffect->m_iBlendState = RendererDX11::Get()->CreateBlendState(&blendConfig);

	RasterizerStateConfigDX11 rsConfig;
	rsConfig.CullMode = D3D11_CULL_BACK;
	rsConfig.FillMode = D3D11_FILL_SOLID;

	m_irasterizerStateSolid1 = m_pRenderer11->CreateRasterizerState(&rsConfig);

	if (m_irasterizerStateSolid1 == -1)
	{
		Log::Get().Write(L"Failed to create light rasterizer state");
		assert(false);
	}

	rsConfig.CullMode = D3D11_CULL_BACK;
	rsConfig.FillMode = D3D11_FILL_WIREFRAME;

	m_irasterizerStateWireframe = m_pRenderer11->CreateRasterizerState(&rsConfig);

	if (m_irasterizerStateWireframe == -1)
	{
		Log::Get().Write(L"Failed to create light rasterizer state");
		assert(false);
	}



	pEffect->m_iRasterizerState = m_irasterizerStateSolid1;

	material->Params[VT_PERSPECTIVE].bRender = true;
	material->Params[VT_PERSPECTIVE].pEffect = pEffect;

	return material;

	return MaterialPtr();
}

void	LJMULevelDemo::setLightsParameters()
{
	m_vDirectionalLightColour = Vector4f(0.3f, 0.3f, 0.3f, 1.0f);
	m_vDirectionalLightDirection = Vector3f(0.5f, -0.5f, 0.0f);

	m_vDirectionalLightColour1 = Vector4f(0.3f, 0.3f, 0.3f, 1.0f);
	m_vDirectionalLightDirection1 = Vector3f(0.5f, -1.0f, 0.0f);

	m_vDirectionalLightColour2 = Vector4f(0.3f, 0.3f, 0.3f, 1.0f);
	m_vDirectionalLightDirection2 = Vector3f(0.5f, -1.0f, 0.5f);

	m_vSpotLightColour = Vector4f(1.0f, 1.0f, 0.0f, 0.0f);
	m_vSpotLightDirection = Vector3f(0.0f, -1.0f, 0.0f);
	m_vSpotLightPosition = Vector4f(900.0f, 100.0f, 900.0f, 1.0f);
	m_vSpotLightRange = Vector4f(1000.0f, 1000.0f, 1000.0f, 1000.0f);
	m_vSpotLightFocus = Vector4f(10.0f, 10.0f, 10.0f, 10.0f);

	m_vSpotLightColour1 = Vector4f(1.0f, 1.0f, 0.0f, 0.0f);
	m_vSpotLightDirection1 = Vector3f(-0.5f, -0.5f, -0.5f);
	m_vSpotLightPosition1 = Vector4f(1200.0f, 100.0f, 900.0f, 1.0f);
	m_vSpotLightRange1 = Vector4f(1000.0f, 1000.0f, 1000.0f, 1000.0f);
	m_vSpotLightFocus1 = Vector4f(10.0f, 10.0f, 10.0f, 10.0f);

	m_vSpotLightColour2 = Vector4f(1.0f, 1.0f, 0.0f, 0.0f);
	m_vSpotLightDirection2 = Vector3f(-0.5f, -0.5f, 0.0f);
	m_vSpotLightPosition2 = Vector4f(1500, 100.0f, 900.0f, 1.0f);
	m_vSpotLightRange2 = Vector4f(1000.0f, 1000.0f, 1000.0f, 1000.0f);
	m_vSpotLightFocus2 = Vector4f(10.0f, 10.0f, 10.0f, 10.0f);

	m_vPointLightColour = Vector4f(1.0f, 0.0f, 0.0f, 0.0f);
	m_vPointLightPosition = Vector4f(500.0f, 100.0f, 900.0f, 1.0f);
	m_vPointLightRange = Vector4f(500.0f, 500.0f, 500.0f, 500.0f);

	m_vPointLightColour1 = Vector4f(1.0f, 0.0f, 0.0f, 0.0f);
	m_vPointLightPosition1 = Vector4f(500.0f, 100.0f, 1500.0f, 1.0f);
	m_vPointLightRange1 = Vector4f(500.0f, 500.0f, 500.0f, 500.0f);

	m_vPointLightColour2 = Vector4f(1.0f, 0.0f, 0.0f, 0.0f);
	m_vPointLightPosition2 = Vector4f(500.0f, 100.0f, 600.0f, 1.0f);
	m_vPointLightRange2 = Vector4f(500.0f, 500.0f, 500.0f, 500.0f);

}

void	LJMULevelDemo::setLights2Material(MaterialPtr material)
{

	m_vDirectionalLightDirection.Normalize();
	material->Parameters.SetVectorParameter(L"DirectionalLightColour", m_vDirectionalLightColour);
	material->Parameters.SetVectorParameter(L"DirectionalLightDirection", Vector4f(m_vDirectionalLightDirection, 1.0f));

	m_vDirectionalLightDirection1.Normalize();
	material->Parameters.SetVectorParameter(L"DirectionalLightColour1", m_vDirectionalLightColour1);
	material->Parameters.SetVectorParameter(L"DirectionalLightDirection1", Vector4f(m_vDirectionalLightDirection1, 1.0f));

	m_vDirectionalLightDirection2.Normalize();
	material->Parameters.SetVectorParameter(L"DirectionalLightColour2", m_vDirectionalLightColour2);
	material->Parameters.SetVectorParameter(L"DirectionalLightDirection2", Vector4f(m_vDirectionalLightDirection2, 1.0f));

	m_vSpotLightDirection.Normalize();
	material->Parameters.SetVectorParameter(L"SpotLightColour", m_vSpotLightColour);
	material->Parameters.SetVectorParameter(L"SpotLightDirection", Vector4f(m_vSpotLightDirection, 1.0f));
	material->Parameters.SetVectorParameter(L"SpotLightPosition", m_vSpotLightPosition);
	material->Parameters.SetVectorParameter(L"SpotLightRange", m_vSpotLightRange);
	material->Parameters.SetVectorParameter(L"SpotLightFocus", m_vSpotLightFocus);

	m_vSpotLightDirection1.Normalize();
	material->Parameters.SetVectorParameter(L"SpotLightColour1", m_vSpotLightColour1);
	material->Parameters.SetVectorParameter(L"SpotLightDirection1", Vector4f(m_vSpotLightDirection1, 1.0f));
	material->Parameters.SetVectorParameter(L"SpotLightPosition1", m_vSpotLightPosition1);
	material->Parameters.SetVectorParameter(L"SpotLightRange1", m_vSpotLightRange1);
	material->Parameters.SetVectorParameter(L"SpotLightFocus1", m_vSpotLightFocus1);

	m_vSpotLightDirection2.Normalize();
	material->Parameters.SetVectorParameter(L"SpotLightColour2", m_vSpotLightColour2);
	material->Parameters.SetVectorParameter(L"SpotLightDirection2", Vector4f(m_vSpotLightDirection2, 1.0f));
	material->Parameters.SetVectorParameter(L"SpotLightPosition2", m_vSpotLightPosition2);
	material->Parameters.SetVectorParameter(L"SpotLightRange2", m_vSpotLightRange2);
	material->Parameters.SetVectorParameter(L"SpotLightFocus2", m_vSpotLightFocus2);

	material->Parameters.SetVectorParameter(L"PointLightColour", m_vPointLightColour);
	material->Parameters.SetVectorParameter(L"PointLightPosition", m_vPointLightPosition);
	material->Parameters.SetVectorParameter(L"PointLightRange", m_vPointLightRange);

	material->Parameters.SetVectorParameter(L"PointLightColour1", m_vPointLightColour1);
	material->Parameters.SetVectorParameter(L"PointLightPosition1", m_vPointLightPosition1);
	material->Parameters.SetVectorParameter(L"PointLightRange1", m_vPointLightRange1);

	material->Parameters.SetVectorParameter(L"PointLightColour2", m_vPointLightColour2);
	material->Parameters.SetVectorParameter(L"PointLightPosition2", m_vPointLightPosition2);
	material->Parameters.SetVectorParameter(L"PointLightRange2", m_vPointLightRange2);

}

void LJMULevelDemo::CreateTerrainIndexArray(int terrainWidth, int terrainLength, bool bWinding, std::vector<int>& indices)
{
	indices.clear();

	if (terrainWidth >= 2 && terrainLength >= 2)
	{
		for (int XIdx = 0; XIdx < terrainWidth - 1; XIdx++)
		{
			for (int YIdx = 0; YIdx < terrainLength - 1; YIdx++)
			{
				const int I0 = (XIdx + 0) * terrainLength + (YIdx + 0);
				const int I1 = (XIdx + 1) * terrainLength + (YIdx + 0);
				const int I2 = (XIdx + 1) * terrainLength + (YIdx + 1);
				const int I3 = (XIdx + 0) * terrainLength + (YIdx + 1);

				if (bWinding)
					ConvertQuadToTriangles(indices, I0, I1, I2, I3);
				else
					ConvertQuadToTriangles(indices, I0, I1, I2, I3);
			}
		}
	}
}

BasicMeshPtr LJMULevelDemo::CreateTerrainMeshFromNoiseFunction(int offseti, int offsetp, int terrainResolution, int terrainSpacing)
{

	// Variable Declarations
	float textureMappingFactor = 0.1f;

	float spacing = terrainSpacing;
	float heightScale = 128.0f;

	int terrainWidth = terrainResolution;
	int terrainLength = terrainResolution;

	int offsetx = offseti * (terrainResolution - 1);
	int offsetz = offsetp * (terrainResolution - 1);

	int seed = 123456;
	float frequency = 0.01f;

	FastNoise noiseGenerator;
	noiseGenerator.SetNoiseType(FastNoise::SimplexFractal);
	noiseGenerator.SetSeed(seed);
	noiseGenerator.SetFrequency(frequency);

	std::vector<Vector3f> vertices;
	std::vector<Vector4f> vertexColors;
	std::vector<Vector3f> normal;

	float majorheightfrequency = 5;
	float majorheight = 1.0f;

	float minorheightfrequency = 75;
	float minorheight = 0.25f;

	for (int i = 0; i < terrainWidth; i++)
	{
		for (int p = 0; p < terrainLength; p++)
		{
			float height = noiseGenerator.GetNoise(i, p) * heightScale;

			vertices.push_back(Vector3f(i * spacing, height, p * spacing));

			float shade = (height / heightScale + 1) / 2.0f;

			if (shade < 0)
				shade = 0;
			else if (shade > 1)
				shade = 1;

			m_vTextureBoundry = Vector4f(0.5f, 0, 0, 0) * heightScale;

			vertexColors.push_back(Vector4f(shade, 1 - shade, shade / 2, 1));

			textureCoords.push_back(Vector2f(i * textureMappingFactor, p * textureMappingFactor));

			normal.push_back(Vector3f(0.0f, 1.0f, 0.0f));
		}
	}

	std::vector<int> indices;

	CreateTerrainIndexArray(terrainWidth, terrainLength, true, indices);

	int numberOfIndices = indices.size();
	int numberOfLengths = numberOfIndices / 3;

	auto terrainMesh = std::make_shared<DrawExecutorDX11<BasicVertexDX11::Vertex>>();
	terrainMesh->SetLayoutElements(BasicVertexDX11::GetElementCount(), BasicVertexDX11::Elements);
	terrainMesh->SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	terrainMesh->SetMaxVertexCount(numberOfIndices);

	BasicVertexDX11::Vertex tv;

	for (int i = 0; i < numberOfIndices; i++)
	{
		tv.position = vertices[indices[i]];
		tv.color = vertexColors[indices[i]];
		tv.texcoords = textureCoords[indices[i]];
		tv.normal = normal[indices[i]];

		terrainMesh->AddVertex(tv);
	}


	return terrainMesh;
}

void LJMULevelDemo::ConvertQuadToTriangles(std::vector<int>& indices, int Vert0, int Vert1, int Vert2, int Vert3)
{
	indices.push_back(Vert3);
	indices.push_back(Vert1);
	indices.push_back(Vert0);

	indices.push_back(Vert3);
	indices.push_back(Vert2);
	indices.push_back(Vert1);
}

void LJMULevelDemo::setupGeometryTransform()
{
	//m_WorldMatrix = Matrix4f::Identity();
	//m_pRenderer11->m_pParamMgr->SetWorldMatrixParameter(&m_WorldMatrix);

	Vector3f vScale = Vector3f(1, 1, 1);
	terrainSegmentActor->GetNode()->Scale() = vScale;

	Matrix3f mRotation;
	mRotation.MakeIdentity();
	terrainSegmentActor->GetNode()->Rotation() = mRotation;

	Vector3f vTranslation = Vector3f(0, 0, 0);
	terrainSegmentActor->GetNode()->Position() = vTranslation;

}

//////////////////////////////////////
// Get the Window Name of this Application
//////////////////////////////////////
std::wstring LJMULevelDemo::GetName()
{
	return(std::wstring(L"6107COMP: Coursework Template"));
}

/////////////////////////////////////
// Assemble our Input Layouts for this
// Stage of the Pipeline.
/////////////////////////////////////
void LJMULevelDemo::inputAssemblyStage()
{			
	//-----SETUP OUR GEOMETRY FOR THIS SCENE-----------------------------------------
	// Stage 01 ---------------------------------	
	setupGeometry();
	// End marker --------------------------------

}

////////////////////////////////////
// Initialise our DirectX 3D Scene
////////////////////////////////////
void LJMULevelDemo::Initialize()
{
	// Stage 01 ---------------------------------

	inputAssemblyStage();			// Call the Input Assembly Stage to setup the layout of our Engine Objects
	setupGeometryTransform();		// Setup the initial pose of our geometry
	setupCamera();					// Setup the camera
	// End marker --------------------------------

}

void LJMULevelDemo::animatePlanet()
{
	Matrix3f planetRotationMatrix;
	planet_rotation += m_pTimer->Elapsed() * 0.5f;
	planetRotationMatrix.RotationZYX(Vector3f(GLYPH_PI, planet_rotation, 0.0f));
	planet_entity->Scale() = Vector3f(1.0f, 1.0f, 1.0f);
	planet_entity->Rotation() = planetRotationMatrix;
	
}

///////////////////////////////////
// Update the State of our Game and 
// Output the Results to Screen (Render)
/////////////////////////////////// 
void LJMULevelDemo::Update()
{
	this->m_pTimer->Update();
	EvtManager.ProcessEvent(EvtFrameStartPtr(new EvtFrameStart(this->m_pTimer->Elapsed())));

	//----------START RENDERING--------------------------------------------------------------

	animatePlanet();
	
	float elapsedTime = m_pTimer->Elapsed();
	m_totalTime += m_pTimer->Elapsed();
	Vector4f time = Vector4f(elapsedTime, m_totalTime, 0.0f, 0.0f);
	planet_material->Parameters.SetVectorParameter(L"time", time);

	m_pCamera->Spatial().SetTranslation(Vector3f(m_spaceshipActor->GetNode()->Position().x, m_spaceshipActor->GetNode()->Position().y, m_spaceshipActor->GetNode()->Position().z));
	moveSpaceShip();

	// Stage 01 ---------------------------------
	// Update the object position in world Space

	this->m_pScene->Update(m_pTimer->Elapsed());
	this->m_pScene->Render(this->m_pRenderer11);

	// End marker --------------------------------


	//--------END RENDERING-------------------------------------------------------------
	this->m_pRenderer11->Present(this->m_pWindow->GetHandle(), this->m_pWindow->GetSwapChain());
}

///////////////////////////////////
// Configure the DirectX 11 Programmable
// Pipeline Stages and Create the Window
// Calls 
///////////////////////////////////
bool LJMULevelDemo::ConfigureEngineComponents()
{

	// Set the render window parameters and initialize the window
	this->m_pWindow = new Win32RenderWindow();
	this->m_pWindow->SetPosition(25, 25);
	this->m_pWindow->SetSize(m_iscreenWidth, m_iscreenHeight);
	this->m_pWindow->SetCaption(this->GetName());
	this->m_pWindow->Initialize(this);


	// Create the renderer and initialize it for the desired device
	// type and feature level.
	this->m_pRenderer11 = new RendererDX11();

	if (!this->m_pRenderer11->Initialize(D3D_DRIVER_TYPE_HARDWARE, D3D_FEATURE_LEVEL_11_0))
	{
		Log::Get().Write(L"Could not create hardware device, trying to create the reference device...");

		if (!this->m_pRenderer11->Initialize(D3D_DRIVER_TYPE_REFERENCE, D3D_FEATURE_LEVEL_10_0))
		{
			ShowWindow(this->m_pWindow->GetHandle(), SW_HIDE);
			MessageBox(this->m_pWindow->GetHandle(), L"Could not create a hardware or software Direct3D 11 device!", L"5108COMP Coursework Template", MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
			this->RequestTermination();
			return(false);
		}
		// If using the reference device, utilize a fixed time step for any animations.
		this->m_pTimer->SetFixedTimeStep(1.0f / 10.0f);
	}

	// Create a swap chain for the window that we started out with.  This
	// demonstrates using a configuration object for fast and concise object
	// creation.
	SwapChainConfigDX11 tconfig;
	tconfig.SetWidth(this->m_pWindow->GetWidth());
	tconfig.SetHeight(this->m_pWindow->GetHeight());
	tconfig.SetOutputWindow(this->m_pWindow->GetHandle());
	this->m_iSwapChain = this->m_pRenderer11->CreateSwapChain(&tconfig);
	this->m_pWindow->SetSwapChain(this->m_iSwapChain);
	
	//Create Colour and Depth Buffers
	this->m_RenderTarget = this->m_pRenderer11->GetSwapChainResource(this->m_iSwapChain);

	Texture2dConfigDX11 tdepthconfig;
	tdepthconfig.SetDepthBuffer(m_iscreenWidth, m_iscreenHeight);
	this->m_DepthTarget = this->m_pRenderer11->CreateTexture2D(&tdepthconfig, 0);

	// Bind the swap chain render target and the depth buffer for use in rendering.  
	this->m_pRenderer11->pImmPipeline->ClearRenderTargets();
	this->m_pRenderer11->pImmPipeline->OutputMergerStage.DesiredState.RenderTargetViews.SetState(0, this->m_RenderTarget->m_iResourceRTV);
	this->m_pRenderer11->pImmPipeline->OutputMergerStage.DesiredState.DepthTargetViews.SetState(this->m_DepthTarget->m_iResourceDSV);
	this->m_pRenderer11->pImmPipeline->ApplyRenderTargets();

	D3D11_VIEWPORT tviewport;
	tviewport.Width = static_cast< float >(m_iscreenWidth);
	tviewport.Height = static_cast< float >(m_iscreenHeight);
	tviewport.MinDepth = 0.0f;
	tviewport.MaxDepth = 1.0f;
	tviewport.TopLeftX = 0;
	tviewport.TopLeftY = 0;

	int tvpindex = this->m_pRenderer11->CreateViewPort(tviewport);
	this->m_pRenderer11->pImmPipeline->RasterizerStage.DesiredState.ViewportCount.SetState(1);
	this->m_pRenderer11->pImmPipeline->RasterizerStage.DesiredState.Viewports.SetState(0, tvpindex);
	return(true);
}

//////////////////////////////////
//Handle Input Events in the Application
//////////////////////////////////
bool LJMULevelDemo::HandleEvent(EventPtr pevent)
{
	eEVENT e = pevent->GetEventType();

	if (e == SYSTEM_KEYBOARD_KEYDOWN)
	{
		EvtKeyDownPtr tkey_down = std::static_pointer_cast<EvtKeyDown>(pevent);
		unsigned int  tkeycode = tkey_down->GetCharacterCode();

		if (tkeycode == VK_UP)
		{
			RenderEffectDX11* pEffect = m_terrainMaterial->Params[VT_PERSPECTIVE].pEffect;

			pEffect->m_iRasterizerState = m_irasterizerStateWireframe;

			m_terrainMaterial->Params[VT_PERSPECTIVE].bRender = true;
			m_terrainMaterial->Params[VT_PERSPECTIVE].pEffect = pEffect;

		}
		if (tkeycode == VK_DOWN)
		{
			RenderEffectDX11* pEffect = m_terrainMaterial->Params[VT_PERSPECTIVE].pEffect;

			pEffect->m_iRasterizerState = m_irasterizerStateSolid;

			m_terrainMaterial->Params[VT_PERSPECTIVE].bRender = true;
			m_terrainMaterial->Params[VT_PERSPECTIVE].pEffect = pEffect;
		}
		if (tkeycode == VK_RIGHT)
		{
			m_pScene->AddActor(terrainSegmentActor);
			m_pScene->RemoveActor(terrainSegmentActor1);

			m_terrainMaterial->Parameters.SetVectorParameter(std::wstring(L"TextureBoundry"), m_vTextureBoundry);
		}
		if (tkeycode == VK_LEFT)
		{
			m_pScene->AddActor(terrainSegmentActor1);
			m_pScene->RemoveActor(terrainSegmentActor);

			m_terrainMaterial->Parameters.SetVectorParameter(std::wstring(L"TextureBoundry"), m_vTextureBoundry);
		}
	}
	else if (e == SYSTEM_KEYBOARD_KEYUP)
	{
		EvtKeyUpPtr tkey_up = std::static_pointer_cast<EvtKeyUp>(pevent);
		unsigned int tkeycode = tkey_up->GetCharacterCode();
	}

	return(Application::HandleEvent(pevent));
}

//////////////////////////////////
// Destroy Resources created by the engine
//////////////////////////////////
void LJMULevelDemo::ShutdownEngineComponents()
{
	if (this->m_pRenderer11)
	{
		this->m_pRenderer11->Shutdown();
		delete this->m_pRenderer11;
	}

	if (this->m_pWindow)
	{
		this->m_pWindow->Shutdown();
		delete this->m_pWindow;
	}
}

//////////////////////////////////
// Shutdown the Application
//////////////////////////////////
void LJMULevelDemo::Shutdown()
{
	//NOTHING TO DO HERE
}

//////////////////////////////////
// Take a Screenshot of the Application
//////////////////////////////////
void LJMULevelDemo::TakeScreenShot()
{
	if (this->m_bSaveScreenshot)
	{
		this->m_bSaveScreenshot = false;
		this->m_pRenderer11->pImmPipeline->SaveTextureScreenShot(0, this->GetName());
	}
}

//////////////////////////////////////
// Output our Frame Rate
//////////////////////////////////////
std::wstring LJMULevelDemo::outputFPSInfo()
{
	std::wstringstream out;
	out << L"FPS: " << m_pTimer->Framerate();
	return out.str();
}