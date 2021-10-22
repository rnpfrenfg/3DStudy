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
	Camera()
	{
		DirectX::XMFLOAT3 R = { 1.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 U = { 0.0f, 1.0f, 0.0f };
		DirectX::XMFLOAT3 L = { 0.0f, 0.0f, 1.0f };

		SetPos(0, 0, 0);
		mRight = DX::XMLoadFloat3(&R);
		mUp = DX::XMLoadFloat3(&U);
		mLook = DX::XMLoadFloat3(&L);
	}

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

	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp)
	{
		DX::XMVECTOR L = DX::XMVector3Normalize(DX::XMVectorSubtract(target, pos));
		DX::XMVECTOR R = DX::XMVector3Normalize(DX::XMVector3Cross(worldUp, L));
		DX::XMVECTOR U = DX::XMVector3Cross(L, R);

		mPos = pos;
		mLook = L;
		mRight = R;
		mUp = U;
	}
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up)
	{
		DX::XMVECTOR P = DX::XMLoadFloat3(&pos);
		DX::XMVECTOR T = DX::XMLoadFloat3(&target);
		DX::XMVECTOR U = DX::XMLoadFloat3(&up);

		LookAt(P, T, U);
	}

	void SetPos(float x, float y, float z)
	{
		mPos = DX::XMVectorSet(x, y, z, 0);
	}

	void Strafe(float d)
	{
		DX::XMVECTOR s = DX::XMVectorReplicate(d);
		mPos = DX::XMVectorMultiplyAdd(s, mRight, mPos);
	}

	void Walk(float d)
	{
		DX::XMVECTOR s = DX::XMVectorReplicate(d);
		mPos = DX::XMVectorMultiplyAdd(s, mLook, mPos);
	}

	void Pitch(float angle)
	{
		DX::XMMATRIX R = DX::XMMatrixRotationAxis(mRight, angle);

		mUp = XMVector3TransformNormal(mUp, R);
		mLook = XMVector3TransformNormal(mLook, R);
	}

	void RotateY(float angle)
	{
		DX::XMMATRIX R = DX::XMMatrixRotationY(angle);

		mRight = XMVector3TransformNormal(mRight, R);
		mUp = XMVector3TransformNormal(mUp, R);
		mLook = XMVector3TransformNormal(mLook, R);
	}

	void SetView()
	{
		mLook = DX::XMVector3Normalize(mLook);
		mUp = DX::XMVector3Normalize(mUp);
		mRight = DX::XMVector3Cross(mUp, mLook);



		// Fill in the view matrix entries.
		float x = -DX::XMVectorGetX(DX::XMVector3Dot(mPos, mRight));
		float y = -DX::XMVectorGetX(DX::XMVector3Dot(mPos, mUp));
		float z = -DX::XMVectorGetX(DX::XMVector3Dot(mPos, mLook));

		DX::XMFLOAT3 R;
		DX::XMFLOAT3 U;
		DX::XMFLOAT3 L;
		DX::XMStoreFloat3(&R, mRight);
		DX::XMStoreFloat3(&U, mUp);
		DX::XMStoreFloat3(&L, mLook);

		DX::XMFLOAT4X4 viewMat;
		viewMat(0, 0) = R.x;
		viewMat(1, 0) = R.y;
		viewMat(2, 0) = R.z;
		viewMat(3, 0) = x;

		viewMat(0, 1) = U.x;
		viewMat(1, 1) = U.y;
		viewMat(2, 1) = U.z;
		viewMat(3, 1) = y;

		viewMat(0, 2) = L.x;
		viewMat(1, 2) = L.y;
		viewMat(2, 2) = L.z;
		viewMat(3, 2) = z;

		viewMat(0, 3) = 0.0f;
		viewMat(1, 3) = 0.0f;
		viewMat(2, 3) = 0.0f;
		viewMat(3, 3) = 1.0f;

		mView = DX::XMLoadFloat4x4(&viewMat);
		return;

		if (CameraMode::Look == mode)
		{
			target = DX::XMVectorSet(0, -1, 0, 0);//TODO
			target = DX::XMVectorAdd(mPos, target);

			mUp = DX::XMVectorSet(0.0f, 0, 1, 0.0f);

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
		mProj = DX::XMMatrixPerspectiveFovLH(fov, width / height, mNearZ, mFarZ);
	}

	CameraMode mode = CameraMode::Look;

	float fov = DX::XM_PI * 0.25f;
	
	DX::XMVECTOR mPos;
	DX::XMVECTOR mRight;
	DX::XMVECTOR mUp;
	DX::XMVECTOR mLook;

	DX::XMVECTOR mFollowingLoc;
	DX::XMVECTOR* mFollowingTarget;

	DX::XMMATRIX mProj;
	DX::XMMATRIX mView;

	float mNearZ = 0.01;
	float mFarZ = 100000;


	DX::XMVECTOR target;
};

