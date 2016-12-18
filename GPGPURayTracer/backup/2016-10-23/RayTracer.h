#pragma once
#include <directxmath.h>
#include <iostream>
#include <string>

#include "Graphics.h"
#include "ShaderWrapping.h"
#include "Model3D.h"
#include "D3DComputeShader.h"
#include "D3DStructuredBuffer.h"
#include "D3DTextureArray.h"

class ProperObjLoader;

using namespace DirectX;

struct ShaderMaterial
{
	float ns;		//specular intensity (0-1000)
	XMFLOAT3 ka;	//ambient
	XMFLOAT3 kd;	//diffuse
	XMFLOAT3 ks;	//specular lightning. black = off.

	int	map_kd;		//index of diffuse map
	int map_normal;	//index of normal map
};

struct TriangleInfo
{
	int material;
	int model;
};

class RayTracer
{
public:
	RayTracer(Graphics *g, UINT resolutionX, UINT resolutionY);
	void update(double dt);
	void draw(double dt, Graphics *g);
	void updateCamera(XMFLOAT4 dir, XMFLOAT4 pos, XMFLOAT4 up, Graphics *g);
	void loadModel(Graphics *g, ProperObjLoader *loader, Model3D *model, const char *path, bool forceTriangles = false);
	~RayTracer();
private:
	void initCBuffers(Graphics *g);
	void prepareModels(vector<Model3D*> models);
	void setModelShaderData(Graphics *g);
private:
	//Constant Buffers
	CBOnce				cbOnce;
	CBCamera			cbCamera;
	CBVertexInfo		cbVertexInfo;
	CBSettings			cbSettings;

	//Compute Shaders
	D3DComputeShader	csPrimaryRays;
	D3DComputeShader	csIntersections;
	D3DComputeShader	csColorComputation;
	D3DComputeShader	csWorldInteraction;

	//Textures
	//D3DTexture		textures;
	//D3DTexture		normals;

	vector<string>		modelTexPaths;
	vector<string>		modelNormPaths;
	D3DTextureArray		modelTextures;
	D3DTextureArray		modelNormalTextures;

	//Buffers (SRV)
	D3DStructuredBuffer buffer_vertices;
	D3DStructuredBuffer buffer_materials;
	D3DStructuredBuffer buffer_triangleInfo;
	D3DStructuredBuffer buffer_worlds;

	//Buffers (UAV)
	D3DStructuredBuffer buffer_rays;
	D3DStructuredBuffer buffer_accumulation;
	D3DStructuredBuffer buffer_intersections;
	D3DStructuredBuffer buffer_pointLights;

	vector<MeshVertex>					modelVertices;
	vector<XMMATRIX>					worldMatrices;
	vector<ShaderMaterial>				materials;
	vector<TriangleInfo>				triangleInfo;


	//UAVs
	int					uavSceneIndex = 0;
	int					uavSpheresIndex;
	//int					uavBufferIndex;

	//SRVs
	int					srvBufferIndex;

#define				CBONCE (0)
#define				CBCAMERA (1)
#define				CBVERTEXINFO (2)
#define				CBSETTINGS (3)
#define				CBLIGHTS (4)

	UINT				GRID_SIZE[3]; // number of groups to execute (xyz)
	UINT				GROUP_SIZE[3]; // number of threads per group (xyz)
	UINT				numRays;
};

