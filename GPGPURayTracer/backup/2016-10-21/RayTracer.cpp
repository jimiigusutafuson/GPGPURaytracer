#include "RayTracer.h"
#include <algorithm>
#include "ProperObjLoader.h"


RayTracer::RayTracer(Graphics *g, UINT resolutionX, UINT resolutionY)
{
	numRays = resolutionX*resolutionY; //set the correct number of rays
	std::vector<std::pair<std::string, int>> macros(4);
	int N;
	macros[0] = std::pair<std::string, int>("BLOCK_SIZE_X", GROUP_SIZE[0]);
	macros[1] = std::pair<std::string, int>("BLOCK_SIZE_Y", GROUP_SIZE[1]);
	macros[2] = std::pair<std::string, int>("BLOCK_SIZE_Z", GROUP_SIZE[2]);
	macros[3] = std::pair<std::string, int>("N", N);

	GRID_SIZE[0] = 128;
	GRID_SIZE[1] = 128;
	GRID_SIZE[2] = 1;

	GROUP_SIZE[0] = 8;
	GROUP_SIZE[1] = 8;
	GROUP_SIZE[2] = 1;

	macros[0].second = GROUP_SIZE[0];
	macros[1].second = GROUP_SIZE[1];
	macros[2].second = GROUP_SIZE[2];
	macros[3].second = GROUP_SIZE[0] * GRID_SIZE[0];

	csPrimaryRays.createComputeShader(L"Shaders/RayTracer.hlsl", "csGeneratePrimaryRays", g, macros);
	csIntersections.createComputeShader(L"Shaders/RayTracer.hlsl", "csComputeIntersections", g, macros);
	csColorComputation.createComputeShader(L"Shaders/RayTracer.hlsl", "csComputeColor", g, macros);
	csWorldInteraction.createComputeShader(L"Shaders/RayTracer.hlsl", "csWorldInteraction", g, macros);

	D3D11_SAMPLER_DESC desc;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 16;
	desc.ComparisonFunc = D3D11_COMPARISON_EQUAL;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	csColorComputation.createSampler(desc, g);

	initCBuffers(g);
	ProperObjLoader objLoader;
	//load model
	box = new Model3D(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	house = new Model3D(XMFLOAT3(100, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	prepareModel(g, &objLoader, box, "cube/boax.obj", true);

	buffer_rays.init(numRays, false, true, NULL, 48, g);
	buffer_intersections.init(numRays, false, true, NULL, 16, g);
	buffer_accumulation.init(numRays, false, true, NULL, 16, g);

	struct PointLight //36 bytes
	{
		XMFLOAT3 pos;
		XMFLOAT3 color;
		float reach;
	};
	vector<PointLight> initialLightData(10);
	initialLightData[0].pos = XMFLOAT3(-30, 100, 50);
	initialLightData[1].pos = XMFLOAT3(40, 100, -20);
	initialLightData[2].pos = XMFLOAT3(-50, 60, 30);
	initialLightData[3].pos = XMFLOAT3(33, 33, 33);
	initialLightData[4].pos = XMFLOAT3(-100, -50, 0);
	initialLightData[5].pos = XMFLOAT3(-30, -10, -70);
	initialLightData[6].pos = XMFLOAT3(-70, 0, -10);
	initialLightData[7].pos = XMFLOAT3(0, 200, 0);
	initialLightData[8].pos = XMFLOAT3(-50, 30, 40);
	initialLightData[9].pos = XMFLOAT3(40, -40, 10);
	initialLightData[0].color = XMFLOAT3(1, 1, 1);
	initialLightData[1].color = XMFLOAT3(1, 1, 0);
	initialLightData[2].color = XMFLOAT3(0, 1, 1);
	initialLightData[3].color = XMFLOAT3(1, 0, 1);
	initialLightData[4].color = XMFLOAT3(0.5, 0.5, 0.5);
	initialLightData[5].color = XMFLOAT3(1, 1, 1);
	initialLightData[6].color = XMFLOAT3(1, 1, 1);
	initialLightData[7].color = XMFLOAT3(0, 0, 1);
	initialLightData[8].color = XMFLOAT3(1, 0, 0);
	initialLightData[9].color = XMFLOAT3(0, 1, 0);
	initialLightData[0].reach = 200;
	initialLightData[1].reach = 200;
	initialLightData[2].reach = 100;
	initialLightData[3].reach = 100;
	initialLightData[4].reach = 100;
	initialLightData[5].reach = 100;
	initialLightData[6].reach = 100;
	initialLightData[7].reach = 100;
	initialLightData[8].reach = 100;
	initialLightData[9].reach = 100;
	buffer_pointLights.init(10, false, true, &initialLightData[0], 28, g);

	//srv
	buffer_vertices.srvStartSlot = 0;
	//textures.srvStartSlot = 1;
	//normals.srvStartSlot = 2;
	buffer_materials.srvStartSlot = 3;
	buffer_materialIndices.srvStartSlot = 4;
	//-------------------------------------------------------------------------------------------------------------------------------------------------
	//                                            TODO: load the SRVS for textures and normals as textureArrays!
	//-------------------------------------------------------------------------------------------------------------------------------------------------

	//uavs
	buffer_rays.uavStartSlot = 1;
	buffer_accumulation.uavStartSlot = 2;
	buffer_intersections.uavStartSlot = 3;
	buffer_pointLights.uavStartSlot = 4;


	buffer_vertices.use(g);
	//textures.setShaderResourceView(g);
	//normals.setShaderResourceView(g);
	buffer_materials.use(g);
	buffer_materialIndices.use(g);
	buffer_worlds.use(g);
	buffer_rays.use(g);
	buffer_accumulation.use(g);
	buffer_intersections.use(g);
	buffer_pointLights.use(g);

	cbOnce.resolution = DirectX::XMFLOAT4((float)resolutionX, (float)resolutionY, 0, 0);

	cbSettings.g_envMappingFlag = -1;
	cbSettings.g_isBVHUsed = 0;
	cbSettings.g_isGlossMappingOn = 0;
	cbSettings.g_isNormalMapspingOn = 1;
	cbSettings.g_isPhongShadingOn = 1;
	cbSettings.g_isShadowOn = 1;
	cbSettings.g_numBounces = 1;
	cbSettings.g_numPointLights = 1;

	cbVertexInfo.noVertices = modelVertices.size();
	cbVertexInfo.noTriangles = modelVertices.size() / 3;

	g->updateSubResource(CBONCE, 0, NULL, &cbOnce, 0, 0);
	g->updateSubResource(CBVERTEXINFO, 0, NULL, &cbVertexInfo, 0, 0);
	g->updateSubResource(CBSETTINGS, 0, NULL, &cbSettings, 0, 0);

	csColorComputation.useSampler(g, 0, 1);
}


RayTracer::~RayTracer()
{
	delete box;
}

void RayTracer::update(double dt)
{
	box->move(0.1, 0.1, 0.1);
}

void RayTracer::draw(double dt, Graphics *g)
{
	csWorldInteraction.dispatch(g, GRID_SIZE[0], GRID_SIZE[1], GRID_SIZE[2]); //apply matrix transformations on vertex data
	csPrimaryRays.dispatch(g, GRID_SIZE[0], GRID_SIZE[1], GRID_SIZE[2]); //generate the primary rays
	for (unsigned int i = 0; i < cbSettings.g_numBounces + 1; i++) //for every bounce, we run an intersection and color computation pass
	{
		csIntersections.dispatch(g, GRID_SIZE[0], GRID_SIZE[1], GRID_SIZE[2]);
		csColorComputation.dispatch(g, GRID_SIZE[0], GRID_SIZE[1], GRID_SIZE[2]);
	}
}

void RayTracer::updateCamera(XMFLOAT4 dir, XMFLOAT4 pos, XMFLOAT4 up, Graphics *g)
{
	cbCamera.dir = dir;
	cbCamera.pos = pos;
	cbCamera.up = up;
	g->updateSubResource(1, 0, NULL, &cbCamera, 0, 0);
}

void RayTracer::initCBuffers(Graphics *g)
{
	//cbuffer mapping
	UINT cbufferDataSizes[4] =
	{
		sizeof(float) * 4,					//cbOnce
		sizeof(float) * 12,					//cbCamera
		sizeof(int) * 2 + sizeof(float) * 2,//cbVertexInfo
		32									//cbSettings
	};

	//initialize all cbuffers
	for (unsigned int i = 0; i < 4; i++)
	{
		if (FAILED(g->createCBuffer(cbufferDataSizes[i], i)))
		{
			MessageBox(0, "Error creating cBuffer!", 0, 0);
			return;
		}
	}
}

void RayTracer::prepareModel(Model3D *model)
{
	//add all models' vectors into one vector list
	modelVertices.insert(modelVertices.end(), model->vertices.begin(), model->vertices.end());


	//append textures to the global texture path vectors
	int modelTexPathStart = modelTexPaths.size();
	int modelNormPathStart = modelNormPaths.size();
	modelTexPaths.insert(modelTexPaths.end(), model->texturePaths.begin(), model->texturePaths.end());
	modelNormPaths.insert(modelNormPaths.end(), model->normalPaths.begin(), model->normalPaths.end());
	//convert the obj material into the more lightweight shadermaterial used in our raytracer
	for (UINT i = 0; i < model->materials.size(); i++)
	{
		ShaderMaterial material;
		material.ka = model->materials[i].ka;
		material.kd = model->materials[i].kd;
		material.ks = model->materials[i].ks;
		material.ns = model->materials[i].ns;
		if (model->materials[i].map_kd >= 0)
			material.map_kd = model->materials[i].map_kd + modelTexPathStart;
		else
			material.map_kd = -1;

		if (model->materials[i].map_normal >= 0)
			material.map_normal = model->materials[i].map_normal + modelNormPathStart;
		else
			material.map_normal = -1;

		materials.push_back(material);
	}
}

void RayTracer::loadActiveTextures(Graphics *g)
{
	//load all textures
	modelTextures.loadFromFile(modelTexPaths, g);
	modelNormalTextures.loadFromFile(modelNormPaths, g);
	//use in shader
	modelTextures.g_srvStartSlot = 1;
	modelNormalTextures.g_srvStartSlot = 2;
	modelTextures.setShaderResourceView(g);
	modelNormalTextures.setShaderResourceView(g);

	int numTriangles = 0;
	int materialIndex = 0;

	for (unsigned int i = 0; i < box->iGroups.size(); i++) //for every index group (material group)
	{
		numTriangles = ((box->iGroups.at(i).iEnd + 1) - box->iGroups.at(i).iStart) / 3; //get the number of triangles in this group
		materialIndices.resize(materialIndices.size() + numTriangles, materialIndex); //add the current materialIndex to the materialIndices vector. one index for each triangle
		materialIndex++; //next index
	}

	buffer_vertices.init(modelVertices.size(), true, false, &modelVertices[0], sizeof(MeshVertex), g);
	buffer_materials.init(materials.size(), true, false, &materials[0], sizeof(ShaderMaterial), g);
	buffer_materialIndices.init(materialIndices.size(), true, false, &materialIndices[0], sizeof(int), g);
}

void RayTracer::prepareModel(Graphics *g, ProperObjLoader *loader, Model3D *model, const char *path, bool forceTriangles)
{
	loader->loadObj(path, forceTriangles);
	model->iGroups = loader->g_indexGroups;
	model->materials = loader->g_materials;
	model->texturePaths = loader->g_map_kdPaths;
	model->normalPaths = loader->g_map_normalPaths;
	loader->generateVertexBufferNoIndices(&model->vertices);
	prepareModel(model);
	loadActiveTextures(g);
}