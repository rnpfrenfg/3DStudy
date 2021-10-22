#include "AnimationHelper.h"

KeyFrame::KeyFrame()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

KeyFrame::~KeyFrame()
{
}

float BoneAnimation::GetStartTime()const
{
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	return Keyframes.back().TimePos;
}

float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = FLT_MAX;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, std::vector<DX::XMMATRIX>& boneTransforms)const
{
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
auto clip = mAnimations.find(clipName);
return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
auto clip = mAnimations.find(clipName);
return clip->second.GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
return mBoneHierarchy.size();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy,
	std::vector<DX::XMMATRIX>& boneOffsets,
	std::unordered_map<std::string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets = boneOffsets;
	mAnimations = animations;
}

void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<DX::XMMATRIX>& finalTransforms)const
{
	UINT numBones = mBoneOffsets.size();

	std::vector<DX::XMMATRIX> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance.
	auto clip = mAnimations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	std::vector<DX::XMMATRIX> toRootTransforms(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		int parentIndex = mBoneHierarchy[i];
		toRootTransforms[i] = DX::XMMatrixMultiply(toParentTransforms[i], toRootTransforms[parentIndex]);
	}

	// Premultiply by the bone offset transform to get the final transform.
	for (UINT i = 0; i < numBones; ++i)
	{
		finalTransforms[i] = DX::XMMatrixTranspose(mBoneOffsets[i] * toRootTransforms[i]);
	}
}