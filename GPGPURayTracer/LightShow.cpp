#include "LightShow.h"
#include "Global.h"


LightShow::LightShow(DirectX::XMFLOAT3 pCenter)
{
	numLights = MAXLIGHTS;
	lightsUsed = numLights;
	lightType = 0;
	center = center;
	this->numLights = numLights;
	assert(numLights >= 0);
	for (unsigned int i = 0; i < numLights && i < MAXLIGHTS; i++)
	{
		lights.push_back(Light());
		lights.back().color = DirectX::XMFLOAT4(randf(), randf(), randf(), 1.0f);
		lights[i].pos.y = center.y + randf(5.0f, 50.0f);
		lights.back().reach = 50.0f + randf(50);
		lights.back().speed = randf();
		lights.back().distanceFromCenter = randf(10.0f, 50.0f);

		lights[i].dir.x = 0;
		lights[i].dir.y = -1;
		lights[i].dir.z = 1;
		DirectX::XMStoreFloat3(&lights[i].dir, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&lights[i].dir))); //normalize
	}
}


LightShow::~LightShow()
{

}

void LightShow::update(float dt)
{
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		lights[i].progress += dt*lights[i].speed;
		while (lights[i].progress >= 6.28318531f) //2*PI
			lights[i].progress -= 6.28318531f;
		lights[i].pos.x = cos(lights[i].progress)*lights[i].distanceFromCenter + center.x;
		lights[i].pos.z = sin(lights[i].progress)*lights[i].distanceFromCenter + center.z;
	}
}

void LightShow::setLightType(int type)
{
	lightType = type;
}

void LightShow::feedBuffer(CBLights *cblights)
{
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		cblights->g_lightColor[i] = lights[i].color;
		cblights->g_lightDir[i] = DirectX::XMFLOAT4(lights[i].dir.x, lights[i].dir.y, lights[i].dir.z, lights[i].reach);
		cblights->g_lightPos[i] = DirectX::XMFLOAT4(lights[i].pos.x, lights[i].pos.y, lights[i].pos.z, 1.0f);
	}
	cblights->g_numLights = lightsUsed;
	cblights->g_lightType = lightType;
}