#pragma once

#include "Application.h"

#include "Win32RenderWindow.h"
#include "RendererDX11.h"

#include "ViewPerspective.h"

//Hieroglyph Includes
#include "Camera.h"
#include "Scene.h"
#include "GeometryActor.h"
#include "PointLight.h"

//STL Includes
#include <vector>

//LJMU Framework Includes
#include "LJMUTextOverlay.h"

using namespace Glyph3;

// Stage 01 ---------------------------------
#include <DirectXMath.h>
// Declare structure for our Vertex Buffer
struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

D3D11_INPUT_ELEMENT_DESC VertexDesc[] =
{
	{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

// End marker --------------------------------

typedef std::shared_ptr<Glyph3::DrawExecutorDX11<BasicVertexDX11::Vertex>> BasicMeshPtr;

struct Edge
{
	int v0;
	int v1;

	Edge(int v0, int v1) : v0(v0 < v1 ? v0 : v1), v1(v0 < v1 ? v1 : v0)
	{

	}

	bool operator <(const Edge& rhs) const
	{
		return v0 < rhs.v0 || (v0 == rhs.v0 && v1 < rhs.v1);
	}
};

namespace CubeToSphere
{
	static const Vector3f origins[6] =
	{
		Vector3f(-1.0, -1.0, -1.0),
		Vector3f(1.0, -1.0, -1.0),
		Vector3f(1.0, -1.0, 1.0),
		Vector3f(-1.0, -1.0, 1.0),
		Vector3f(-1.0, 1.0, -1.0),
		Vector3f(-1.0, -1.0, 1.0)
	};
	static const Vector3f rights[6] =
	{
		Vector3f(2.0, 0.0, 0.0),
		Vector3f(0.0, 0.0, 2.0),
		Vector3f(-2.0, 0.0, 0.0),
		Vector3f(0.0, 0.0, -2.0),
		Vector3f(2.0, 0.0, 0.0),
		Vector3f(2.0, 0.0, 0.0)
	};
	static const Vector3f ups[6] =
	{
		Vector3f(0.0, 2.0, 0.0),
		Vector3f(0.0, 2.0, 0.0),
		Vector3f(0.0, 2.0, 0.0),
		Vector3f(0.0, 2.0, 0.0),
		Vector3f(0.0, 0.0, 2.0),
		Vector3f(0.0, 0.0, -2.0)
	};
};

namespace LJMUDX
{
	//////////////////////////////////////
	//LJMULevelDemo.H
	//Class Application for a DirectX 11
	//Driven Application using the DirectX Toolkit
	//Hieroglyph 3 Rendering Engine and LUA.
	//
	//
	//AUTHORS:  DR PO YANG
	//			DR CHRIS CARTER
	//////////////////////////////////////

	class LJMULevelDemo : public Application //Inherit from the Hieroglyph Base Class
	{

	public:
		//------------CONSTRUCTORS------------------------------------------------
		LJMULevelDemo();	//Standard Empty Constructor which builds the object

	public:
		//------------INHERITED METHODS-------------------------------------------
		virtual void Initialize();					//Initialise the DirectX11 Scene
		void animatePlanet();
		virtual void Update();						//Update the DirectX Scene
		virtual void Shutdown();					//Shutdown the DirectX11 Scene

		virtual bool ConfigureEngineComponents();	//Initialise Hieroglyph and DirectX TK Modules
		virtual void ShutdownEngineComponents();	//Destroy Hieroglyph and DirectX TK Modules

		virtual void TakeScreenShot();				//Allow a screenshot to be generated

		virtual bool HandleEvent(EventPtr pEvent);	//Handle an I/O Event
		virtual std::wstring GetName();				//Get the Name of this App Instance

		//------------CUSTOM METHODS-----------------------------------------------
		void inputAssemblyStage();					//Stage to setup our VB and IB Info

		std::wstring outputFPSInfo();				//Convert the timer's Frames Per Second to a formatted string

	protected:
		//-------------CLASS MEMBERS-----------------------------------------------
		RendererDX11* m_pRenderer11;		//Pointer to our DirectX 11 Device
		Win32RenderWindow* m_pWindow;			//Pointer to our Windows-Based Window

		int						m_iSwapChain;		//Index of our Swap Chain 
		ResourcePtr				m_RenderTarget;		//Pointer to the GPU Render Target for Colour
		ResourcePtr				m_DepthTarget;		//Pointer to the GPU Render Target for Depth

		//--------------HIEROGLYPH OBJECTS-----------------------------------------
		ViewPerspective* m_pRenderView;		//3D Output View - DirectX 11 Accelerated
		LJMUTextOverlay* m_pRender_text;		//2D Output View - DirectX 11 Accelerated
		Camera* m_pCamera;			//Camera Object

		float					m_iscreenWidth = 1920.0f;
		float					m_iscreenHeight = 1080.0f;


		// Stage 01 ---------------------------------
		// -- Additional parameters for rotating cube



		ResourcePtr             m_pVertexBuffer;
		int                     m_VertexLayout;
		ResourcePtr             m_pIndexBuffer;
		RenderEffectDX11		m_Effect;

		int						m_irasterizerStateWireframe;
		int						m_irasterizerStateSolid;
		int						m_irasterizerStateSolid1;

		std::vector<Actor*>		m_pTerrianActors;
		std::vector<Actor*>		m_pTerrianActors1;
		MaterialPtr				m_terrainMaterial;
		MaterialPtr			m_terrainMesh;

		std::vector<Vector2f>	textureCoords;

		int						m_vertexSize;
		int						m_indexSize;

		void					setupCamera();
		void					setupProjMatrix();
		void					setupViewMatrix();

		BasicMeshPtr CreateSpherifiedCube(int divisions, Vector4f colour);

		void setMaterialSurfaceProperties(MaterialPtr material, Vector4f surfaceConstants, Vector4f surfaceEmissiveColour);

		void					setupGeometry();

		std::vector<Vector3f>		m_checkpoints;
		int							m_currentCheckpointID, m_nextCheckpointID;

		MaterialPtr createMultiTerrainTextureMaterial(std::wstring textureFile1, std::wstring texturefile2);

		BasicMeshPtr CreateTerrainMesh(int offseti, int offsetp, int terrainResolution, int terrainSpacing);

		BasicMeshPtr				createSpaceShipCaatmullRomPath();

		BasicMeshPtr				generateOBJMesh(std::wstring pmeshname, Vector4f pmeshcolour);

		MaterialPtr					createTextureMaterial(std::wstring texturefilename);

		MaterialPtr					createPathMaterial();

		BasicMeshPtr			skysphere_geometry;
		MaterialPtr				skysphere_material;
		MaterialPtr				createSkySphereMaterial();
		MaterialPtr				createGSInstancingMaterial();
		MaterialPtr createGSInstancingMaterial1();
		MaterialPtr				createAnimMaterial();
		BasicMeshPtr			planet_geometry;
		MaterialPtr				planet_material;
		MaterialPtr				GeoSphereMaterial1;
		BasicMeshPtr			planet_geometry1;
		MaterialPtr				planet_material1;
		MaterialPtr				createGSAnimMaterial();
		BasicMeshPtr			generateOBJMesh_mod(std::wstring pmeshname, Vector4f pmeshcolour);
		MaterialPtr				createBasicMaterial();

		MaterialPtr createStaticMaterial(std::wstring texturefilename);

		BasicMeshPtr CreateStandardSphere(int meridians, int parallels, Vector4f colour);

		BasicMeshPtr createGeodesicPolyhedron(int subdivision, Vector4f colour);

		MaterialPtr createDummyTerrainTextureMaterial(std::wstring textureFile1, std::wstring texturefile2);

		int subdivideEdge(int f0, int f1, const Vector3f& v0, const Vector3f& v1, std::vector<Vector3f>& additionalVertices, int vertexOffset, std::map<Edge, int>& io_division);

		BasicMeshPtr CreateNormalisedCube(int divisions, Vector4f colour);

		BasicMeshPtr			CreateTerrainMeshFromNoiseFunction(int offseti, int offsetp, int terrainResolution, int terrainSpacing);

		void setLightsParameters();

		void					ConvertQuadToTriangles(std::vector<int>& indices, int Vert0, int Vert1, int Vert2, int Vert3);

		void setLights2Material(MaterialPtr material);

		void					CreateTerrainIndexArray(int terrainWidth, int terrainLength, bool bWinding, std::vector<int>& indices);

		void					setupGeometryTransform();

		const float				DEG_TO_RAD = GLYPH_PI / 180.0f;

		Vector4f				m_vTextureBoundry;

		std::vector<Vector3f>		m_cameraPositions;
		Vector3f					m_cameraPosition;
		int							m_icpindex;

		Actor* skysphereActor;
		Node3D* skysphere_node;
		Glyph3::Entity3D* skysphere_entity;
		Actor* m_pathActor;
		Actor* m_spaceshipActor;
		Vector3f					m_spaceshipDirection, m_spaceshipRefDirection, m_spaceshipTargetDirection;
		float						m_spaceshipMovementSpeed, m_spaceshipRotationSpeed;
		float						m_spaceshipPrevDist2Target;
		int							m_spaceshipState;
		void						moveSpaceShip();
		float					m_totalTime;

		Vector4f	m_vSurfaceConstants;
		Vector4f	m_vSurfaceEmissiveColour;


		Vector4f	m_vDirectionalLightColour;
		Vector3f	m_vDirectionalLightDirection;

		Vector4f	m_vDirectionalLightColour1;
		Vector3f	m_vDirectionalLightDirection1;

		Vector4f	m_vDirectionalLightColour2;
		Vector3f	m_vDirectionalLightDirection2;

		Vector4f	m_vSpotLightColour;
		Vector3f	m_vSpotLightDirection;
		Vector4f	m_vSpotLightPosition;
		Vector4f	m_vSpotLightRange;
		Vector4f	m_vSpotLightFocus;

		Vector4f	m_vSpotLightColour1;
		Vector3f	m_vSpotLightDirection1;
		Vector4f	m_vSpotLightPosition1;
		Vector4f	m_vSpotLightRange1;
		Vector4f	m_vSpotLightFocus1;

		Vector4f	m_vSpotLightColour2;
		Vector3f	m_vSpotLightDirection2;
		Vector4f	m_vSpotLightPosition2;
		Vector4f	m_vSpotLightRange2;
		Vector4f	m_vSpotLightFocus2;

		Vector4f	m_vPointLightColour;
		Vector4f	m_vPointLightPosition;
		Vector4f	m_vPointLightRange;

		Vector4f	m_vPointLightColour1;
		Vector4f	m_vPointLightPosition1;
		Vector4f	m_vPointLightRange1;

		Vector4f	m_vPointLightColour2;
		Vector4f	m_vPointLightPosition2;
		Vector4f	m_vPointLightRange2;


		Actor* m_pActor;
		Node3D* planet_node;
		Node3D* static_node;
		Glyph3::Entity3D* planet_entity;
		float					planet_rotation;

		double* LoadRawHeightMap(const char* filename, int actualTerrainWidth, int actualTerrainLength, int terrainLength, int terrainWidth);

		BasicMeshPtr	CreateTerrainMeshFromFile(const char* filename, int actualTerrainWidth, int actualTerrainHeight);

		/*Actor* m_pCubeActor;*/
		Actor* terrainSegmentActor;
		Actor* terrainSegmentActor1;

		MaterialPtr m_Dummy;
	};

}

Vector3f CatmullRom(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f p3, float t);
