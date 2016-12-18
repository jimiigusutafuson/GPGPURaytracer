#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "ShaderWrapping.h"
struct Light
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 dir;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 focusPoint;
	float reach;
	float speed;
	float progress;
	float distanceFromCenter;
};
class LightShow
{
public:
	const int MAXLIGHTS = 10;
public:
	LightShow(DirectX::XMFLOAT3 center);
	~LightShow();
	void update(float dt);
	void feedBuffer(CBLights *cblights);
	void setLightType(int type);
	int lightsUsed;
private:
	std::vector<Light> lights;
	int numLights;
	DirectX::XMFLOAT3 center;
	int lightType;
};

