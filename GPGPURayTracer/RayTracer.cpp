#include "RayTracer.h"
#include <algorithm>
#include "ProperObjLoader.h"


RayTracer::RayTracer(Settings settings, Graphics *g, UINT resolutionX, UINT resolutionY)
{
	numRays = resolutionX*resolutionY; //set the correct number of rays
	std::vector<std::pair<std::string, int>> macros(4); //these are predefines for the shader compiler
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
	vector<Model3D*> models;
	ProperObjLoader objLoader;
	//load model
	Model3D box(XMFLOAT3(10, -20, 10), XMFLOAT3(0, 3, 0), XMFLOAT3(1, 1, 1));
	loadModel(g, &objLoader, &box, "cube/boax.obj", false);
	models.push_back(&box);

	Model3D wall(XMFLOAT3(-5, -20, 40), XMFLOAT3(0, 0.05, 0), XMFLOAT3(1, 1, 1));
	if (settings.wall) //if wall is enabled, we will have a multi-textured wall. this cannot be changed during runtime
	{
		loadModel(g, &objLoader, &wall, "wall/wall.obj", false);
		models.push_back(&wall);
	}

	Model3D floor(XMFLOAT3(0, -20, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(50, 50, 50));
	loadModel(g, &objLoader, &floor, "plane/plane.obj", false);
	models.push_back(&floor);

	prepareModels(models);
	setModelShaderData(g);

	buffer_rays.init(numRays, false, true, NULL, 48, g);
	buffer_intersections.init(numRays, false, true, NULL, 16, g);
	buffer_accumulation.init(numRays, false, true, NULL, 16, g);

	lightShow = new LightShow(XMFLOAT3(0, 0, -30));
	lightShow->setLightType(settings.spotLights);

	//srv
	buffer_materials.srvStartSlot = 2;
	buffer_triangleInfo.srvStartSlot = 3;
	//uavs
	buffer_vertices.uavStartSlot = 1;
	buffer_rays.uavStartSlot = 2;
	buffer_accumulation.uavStartSlot = 3;
	buffer_intersections.uavStartSlot = 4;

	//use all srvs and uavs
	buffer_vertices.use(g);
	buffer_materials.use(g);
	buffer_triangleInfo.use(g);
	buffer_worlds.use(g);
	buffer_rays.use(g);
	buffer_accumulation.use(g);
	buffer_intersections.use(g);

	//set the settings
	cbSettings.g_envMappingFlag = -1;
	cbSettings.g_isBVHUsed = 0;
	cbSettings.g_isGlossMappingOn = 0;
	cbSettings.g_isNormalMapspingOn = 1;
	cbSettings.g_isPhongShadingOn = 1;
	cbSettings.g_isShadowOn = 1;
	cbSettings.g_numBounces = settings.bounces;

	//set the number of vertices and number of triangles
	cbVertexInfo.noVertices = modelVertices.size();
	cbVertexInfo.noTriangles = modelVertices.size() / 3;

	g->updateSubResource(CBVERTEXINFO, 0, NULL, &cbVertexInfo, 0, 0);
	//g->updateSubResource(CBSETTINGS, 0, NULL, &cbSettings, 0, 0);
	setSettings(settings, g); //change the settings.

	csColorComputation.useSampler(g, 0, 1);
	csWorldInteraction.dispatch(g, GRID_SIZE[0], GRID_SIZE[1], GRID_SIZE[2]); //apply matrix transformations on vertex data
}


RayTracer::~RayTracer()
{
	delete lightShow;

}

void RayTracer::update(double dt, Graphics *g)
{
	lightShow->update(dt);
	lightShow->feedBuffer(&cbLights);
	g->updateSubResource(CBLIGHTS, 0, NULL, &cbLights, 0, 0);
}

void RayTracer::draw(double dt, Graphics *g)
{
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
	g->updateSubResource(CBCAMERA, 0, NULL, &cbCamera, 0, 0);
}

void RayTracer::initCBuffers(Graphics *g)
{
	//cbuffer mapping
	UINT cbufferDataSizes[4] =
	{
		sizeof(CBCamera),			//cbCamera
		sizeof(CBVertexInfo),		//cbVertexInfo
		sizeof(CBSettings),			//cbSettings
		sizeof(CBLights)			//cbLights
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

void RayTracer::prepareModels(vector<Model3D*> models)
{
	int numTriangles = 0;
	int materialIndex = 0;
	for (unsigned int k = 0; k < models.size(); k++)
	{
		//add all models' vectors into one vector list
		modelVertices.insert(modelVertices.end(), models[k]->vertices.begin(), models[k]->vertices.end());

		//append textures to the global texture path vectors
		int modelTexPathStart = modelTexPaths.size();
		int modelNormPathStart = modelNormPaths.size();
		modelTexPaths.insert(modelTexPaths.end(), models[k]->texturePaths.begin(), models[k]->texturePaths.end());
		modelNormPaths.insert(modelNormPaths.end(), models[k]->normalPaths.begin(), models[k]->normalPaths.end());
		//convert the obj material into the more lightweight shadermaterial used in our raytracer
		for (UINT i = 0; i < models[k]->materials.size(); i++)
		{
			ShaderMaterial material;
			material.ka = models[k]->materials[i].ka;
			material.kd = models[k]->materials[i].kd;
			material.ks = models[k]->materials[i].ks;
			material.ns = models[k]->materials[i].ns;
			if (models[k]->materials[i].map_kd >= 0)
				material.map_kd = models[k]->materials[i].map_kd + modelTexPathStart;
			else
				material.map_kd = -1;

			if (models[k]->materials[i].map_normal >= 0)
				material.map_normal = models[k]->materials[i].map_normal + modelNormPathStart;
			else
				material.map_normal = -1;

			materials.push_back(material);
		}

		for (unsigned int i = 0; i < models[k]->iGroups.size(); i++) //for every index group (material group)
		{
			TriangleInfo ti;
			ti.material = materialIndex + models[k]->iGroups[i].material;
			ti.model = k;
			numTriangles = ((models[k]->iGroups.at(i).iEnd + 1) - models[k]->iGroups.at(i).iStart) / 3; //get the number of triangles in this group
			triangleInfo.resize(triangleInfo.size() + numTriangles, ti); //add the current materialIndex to the materialIndices vector. one index for each triangle
		}
		materialIndex += models[k]->materials.size(); //next model's indices
		worldMatrices.push_back(XMLoadFloat4x4(&models[k]->mWorld));
	}
}

void RayTracer::setModelShaderData(Graphics *g)
{
	//load all textures
	modelTextures.loadFromFile(modelTexPaths, g);
	modelNormalTextures.loadFromFile(modelNormPaths, g);
	//use in shader
	modelTextures.g_srvStartSlot = 0;
	modelNormalTextures.g_srvStartSlot = 1;
	modelTextures.setShaderResourceView(g);
	modelNormalTextures.setShaderResourceView(g);
	buffer_worlds.uavStartSlot = 5;
	

	buffer_vertices.init(modelVertices.size(), false, true, &modelVertices[0], sizeof(MeshVertex), g);
	buffer_materials.init(materials.size(), true, false, &materials[0], sizeof(ShaderMaterial), g);
	buffer_triangleInfo.init(triangleInfo.size(), true, false, &triangleInfo[0], sizeof(TriangleInfo), g);
	buffer_worlds.init(worldMatrices.size(), false, true, &worldMatrices[0], sizeof(XMMATRIX), g);
}

void RayTracer::loadModel(Graphics *g, ProperObjLoader *loader, Model3D *model, const char *path, bool forceTriangles)
{
	loader->loadObj(path, forceTriangles);
	model->iGroups = loader->g_indexGroups;
	model->materials = loader->g_materials;
	model->texturePaths = loader->g_map_kdPaths;
	model->normalPaths = loader->g_map_normalPaths;
	loader->generateVertexBufferNoIndices(&model->vertices);
	loader->clear();
}

void RayTracer::setSettings(Settings settings, Graphics *g)
{
	lightShow->setLightType(settings.spotLights);
	lightShow->lightsUsed = settings.numLights;
	cbSettings.g_isShadowOn = settings.shadows;
	cbSettings.g_numBounces = settings.bounces;

	g->updateSubResource(CBSETTINGS, 0, NULL, &cbSettings, 0, 0);
}