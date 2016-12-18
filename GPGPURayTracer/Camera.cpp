#include "Camera.h"

using namespace DirectX;

Camera::Camera(float x, float y, float z)
{
	pitchRotation = 0;
	//maybe not the best place to have the view matrix for the light which has shadowmap
	XMMATRIX mProj, mProjInv, mLightView;
	//GraphicsCore *g = GraphicsCore::getInstance();

	mProj = XMMatrixPerspectiveFovLH(XM_PI * 0.4f,
		(float)DEFAULTSCRNWIDTH / (float)DEFAULTSCRNHEIGHT,
		0.1f, 8000.0f);
	mProjInv = XMMatrixInverse(&XMMatrixDeterminant(mProj), mProj);
	XMStoreFloat4(&pos, XMVectorSet(x, y, z, 1.0f));
	XMStoreFloat4(&dir, XMVector3Normalize(XMVectorSet(0, 0, 1, 0.0f)));
	XMStoreFloat4(&up, XMVectorSet(.0f, 1.0f, .0f, .0f));

	/*CBOnce cb;

	cb.resolution = XMFLOAT4(DEFAULTSCRNWIDTH, DEFAULTSCRNHEIGHT, 0, 0);
	g->immediateContext->UpdateSubresource(g->cbOnce, 0, NULL, &cb, 0, 0);*/
	updateViewMatrix();
}
void Camera::updateViewMatrix()
{
	XMMATRIX	mView, mViewInv;
	XMVECTOR	xPos = XMLoadFloat4(&pos);
	XMVECTOR	xDir = XMLoadFloat4(&dir);
	//Graphics *g = Graphics::getInstance();

	mView = XMMatrixLookAtLH(xPos, XMVectorAdd(xPos, XMLoadFloat4(&dir)), XMLoadFloat4(&up));
	mViewInv = XMMatrixInverse(&XMMatrixDeterminant(mView), mView);



	XMStoreFloat4(&pos, xPos);
	XMStoreFloat4(&dir, xDir);


	//------------- do this outside the camera class -------------

	/*CBCamera cb;
	cb.pos = pos;
	cb.dir = dir;
	cb.up = up;

	g->immediateContext->UpdateSubresource(g->cbCamera, 0, NULL, &cb, 0, 0);*/
}

void Camera::move()
{
	//throw new exception("not implemented yet.");
}

void Camera::fly( XMFLOAT3 movement)
{
	XMVECTOR xDir = XMLoadFloat4(&dir);
	XMVECTOR xUp = XMLoadFloat4(&up);
	XMVECTOR strafe = XMVector3Cross(XMLoadFloat4(&dir), XMLoadFloat4(&up));

	XMVECTOR newpos = XMLoadFloat4(&pos) +
		strafe * movement.x * 100 +
		xDir * movement.z * 100 +
		XMLoadFloat4(&XMFLOAT4(0, movement.y * 100, 0, 0));
	XMStoreFloat4(&pos, newpos);

	updateViewMatrix();
}
void Camera::rotate(float yaw, float pitch, float roll)
{
	XMMATRIX mRotation;
	XMVECTOR xDir = XMLoadFloat4(&dir);
	XMVECTOR xUp = XMLoadFloat4(&up);
	XMVECTOR rotationAxis;

	if (pitchRotation - pitch / XMConvertToRadians(180) > XMConvertToRadians(90))
		pitch = max(pitchRotation, XMConvertToRadians(90)) - min(pitchRotation, XMConvertToRadians(90));
	else if (pitchRotation - pitch / XMConvertToRadians(180) < -XMConvertToRadians(90))
		pitch = min(pitchRotation, -XMConvertToRadians(90)) - max(pitchRotation, -XMConvertToRadians(90));
	pitchRotation -= pitch;

	rotationAxis = XMVector3Cross(xDir, xUp);
	rotationAxis = XMVector3Normalize(rotationAxis);

	mRotation = XMMatrixRotationAxis(rotationAxis, -pitch) * XMMatrixRotationY(yaw) * XMMatrixRotationZ(roll);

	xDir = XMVector3TransformCoord(xDir, mRotation);
	xDir = XMVector3Normalize(xDir);
	XMStoreFloat4(&dir, xDir);

	xUp = XMVector3TransformCoord(xUp, mRotation);
	xUp = XMVector3Normalize(xUp);
	XMStoreFloat4(&up, xUp);

	updateViewMatrix();
}
Camera::~Camera()
{

}
