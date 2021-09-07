#pragma once

#include "include.h"

enum class CameraMode
{
	TargetFollow,
	LocationFollow,
	Look
};

class Camera
{
public:

	void SetModeLook()
	{
		mode = CameraMode::Look;
	}

	void SetModeTargetFollow(DX::XMVECTOR* loc)
	{
		mFollowingTarget = loc;
		mode = CameraMode::TargetFollow;
	}

	void SetModeLocationFollow(DX::FXMVECTOR loc)
	{
		mFollowingLoc = loc;
		mode = CameraMode::LocationFollow;
	}


	void Rotate(float difX, float difY)
	{
		mX += difX;
		mY -= difY;

		if (mY < -DX::XM_PI)
			mY = DX::XM_PI;
		else if (mY > DX::XM_PI - 0.1f)
			mY = DX::XM_PI - 0.1f;
	}

	void SetPos(float x, float y, float z)
	{
		mPos = DX::XMVectorSet(x, y, z, 0);
	}

	void SetRotate(float x, float y)
	{
		mX = x;
		mY = y;
	}

	void SetView()
	{
		if (CameraMode::Look == mode)
		{
			target = DX::XMVectorSet(DX::XMScalarSin(mX), mY, DX::XMScalarCos(mX), 0);
			target = DX::XMVectorAdd(mPos, target);

			mUp = DX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			mView = DX::XMMatrixLookAtLH(mPos, target, mUp);
		}
		else if (CameraMode::LocationFollow == mode)
		{
			mView = DX::XMMatrixLookAtLH(mPos, mFollowingLoc, mUp);
		}
		else//CameraMode::TargetFollow
		{
			mView = DX::XMMatrixLookAtLH(mPos, *mFollowingTarget, mUp);
		}
	}

	void SetProj(float width, float height)
	{
		mProj = DX::XMMatrixPerspectiveFovLH(0.25 * DX::XM_PI, width / height, mNearZ, mFarZ);
	}

	CameraMode mode = CameraMode::Look;

	float mX;
	float mY;
	
	DX::XMVECTOR mPos;
	DX::XMVECTOR mFollowingLoc;
	DX::XMVECTOR* mFollowingTarget;
	DX::XMVECTOR mUp;

	DX::XMMATRIX mProj;
	DX::XMMATRIX mView;

	float mNearZ = 0.01;
	float mFarZ = 100000;


	DX::XMVECTOR target;
};

