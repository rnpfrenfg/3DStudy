#pragma once

#include "include.h"

#include <unordered_map>

struct KeyFrame
{
	KeyFrame();
	~KeyFrame();

	float TimePos;
	DX::XMFLOAT3 Translation;
	DX::XMFLOAT3 Scale;
	DX::XMFLOAT4 RotationQuat;
};

class BoneAnimation
{
public:
	float GetStartTime() const;
	float GetEndTime() const;

	void Interpolate(float t, DX::XMMATRIX& M) const
	{
		using DX::XMVECTOR;
		if (t <= Keyframes.front().TimePos)
		{
			XMVECTOR S = DX::XMLoadFloat3(&Keyframes.front().Scale);
			XMVECTOR P = DX::XMLoadFloat3(&Keyframes.front().Translation);
			XMVECTOR Q = DX::XMLoadFloat4(&Keyframes.front().RotationQuat);
			XMVECTOR zero = DX::XMVectorSet(0,0,0,1.0f);

			M = DX::XMMatrixAffineTransformation(S, zero, Q, P);
		}
		else if (t >= Keyframes.back().TimePos)
		{
			XMVECTOR S = DX::XMLoadFloat3(&Keyframes.back().Scale);
			XMVECTOR P = DX::XMLoadFloat3(&Keyframes.back().Translation);
			XMVECTOR Q = DX::XMLoadFloat4(&Keyframes.back().RotationQuat);
			XMVECTOR zero = DX::XMVectorSet(0, 0, 0, 1.0f);

			M = DX::XMMatrixAffineTransformation(S, zero, Q, P);
		}
		else
		{
			for (UINT i = 0; i < Keyframes.size() - 1; ++i)
			{
				if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
				{
					float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

					XMVECTOR s0 = DX::XMLoadFloat3(&Keyframes[i].Scale);
					XMVECTOR s1 = DX::XMLoadFloat3(&Keyframes[i + 1].Scale);

					XMVECTOR p0 = DX::XMLoadFloat3(&Keyframes[i].Translation);
					XMVECTOR p1 = DX::XMLoadFloat3(&Keyframes[i + 1].Translation);

					XMVECTOR q0 = DX::XMLoadFloat4(&Keyframes[i].RotationQuat);
					XMVECTOR q1 = DX::XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

					XMVECTOR S = DX::XMVectorLerp(s0, s1, lerpPercent);
					XMVECTOR P = DX::XMVectorLerp(p0, p1, lerpPercent);
					XMVECTOR Q = DX::XMQuaternionSlerp(q0, q1, lerpPercent);

					XMVECTOR zero = DX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
					M = DX::XMMatrixAffineTransformation(S, zero, Q, P);

					break;
				}
			}
		}
	}

	std::vector<KeyFrame> Keyframes;
};

struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

	void Interpolate(float t, std::vector<DX::XMMATRIX>& boneTransforms)const;

	std::vector<BoneAnimation> BoneAnimations;
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::vector<int>& boneHierarchy,
		std::vector<DirectX::XMMATRIX>& boneOffsets,
		std::unordered_map<std::string, AnimationClip>& animations);

	// In a real project, you'd want to cache the result if there was a chance
	// that you were calling this several times with the same clipName at 
	// the same timePos.
	void GetFinalTransforms(const std::string& clipName, float timePos,
		std::vector<DirectX::XMMATRIX>& finalTransforms)const;

private:
	// Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy;

	std::vector<DirectX::XMMATRIX> mBoneOffsets;

	std::unordered_map<std::string, AnimationClip> mAnimations;
};
