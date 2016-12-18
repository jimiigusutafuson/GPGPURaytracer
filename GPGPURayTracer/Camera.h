#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include "Global.h"
using namespace DirectX;
class Camera
{
private:
	XMFLOAT4	pos,
		dir,
		up;
	float pitchRotation;
	void updateViewMatrix();

public:

	Camera(float x, float y, float z);

	void		move();
	void		fly(XMFLOAT3 movement);
	void		rotate(float yaw, float pitch, float roll);
	XMFLOAT4	getPos() { return pos; }
	XMFLOAT4	getDir() { return dir; }
	XMFLOAT4	getUp() { return up; }
	~Camera();
};

