#include "FbxLoader.h"

class TestClass
{
public:
	TestClass(SkinnedData& data) :data(data) {}

	SkinnedData& data;
	std::vector<int> boneHierarchy;
	std::vector<DirectX::XMMATRIX> boneOffsets;
	std::vector<Mesh> meshes;
	int polygonCount;

	std::unordered_map<std::string, AnimationClip> animations;

	std::vector<std::string> indexName;

	Vertex* vertexes;
	UINT32* indexes;

	FbxScene* scene;

	void Setup(FbxNode* inRootNode, FbxScene* scene)
	{
		this->scene = scene;

		for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); childIndex++)
		{
			FbxNode* currNode = inRootNode->GetChild(childIndex);
			SetupNode(currNode, 0, 0, -1);
		}
	}

	void SetupNode(FbxNode* node, int inDepth, int myIndex, int inParentIndex)
	{
		if (!(node->GetNodeAttribute() && (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)))
		{
			return;
		}


		FbxNodeAttribute::EType b;
		bool c;
		auto a = node->GetNodeAttribute();
		if (a)
		{
			b = node->GetNodeAttribute()->GetAttributeType();
			c = node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton;
		}

		boneHierarchy.push_back(inParentIndex);
		indexName.push_back(node->GetName());

		if (FbxMesh* fbxMesh = node->GetMesh())
		{
			CopyVertexData(fbxMesh);

			int skins = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			for (int i = 0; i < skins; i++)
			{
				FbxSkin* skin = (FbxSkin*)fbxMesh->GetDeformer(i, FbxDeformer::eSkin);

				int clusters = skin->GetClusterCount();
				for (int clusterIndex = 0; clusterIndex < clusters; clusterIndex++)
				{
					FbxCluster* cluster = skin->GetCluster(clusterIndex);
					cluster->GetLink()->GetName();//NAME::::::::::::::::TODO

				}
			}
		}

		//todo node->GetMaterialCount, node->GetMaterial

		{
			FbxAnimEvaluator* evaluator = scene->GetAnimationEvaluator();
			FbxMatrix matrixData;
			matrixData.SetIdentity();

			DX::XMFLOAT4X4 matrix;

			if (node != scene->GetRootNode())
			{
				matrixData = evaluator->GetNodeGlobalTransform(node);
			}

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					matrix.m[i][j] = static_cast<float>(matrixData.Get(i, j));
				}
			}

			DX::XMMATRIX mat = DX::XMLoadFloat4x4(&matrix);
			boneOffsets.push_back(mat);
		}

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			SetupNode(node->GetChild(i), inDepth + 1, boneHierarchy.size(), myIndex);
		}
	}

	void TriangulateRecursive(FbxNode* node)
	{
		FbxNodeAttribute* lNodeAttribute = node->GetNodeAttribute();
		if (nullptr == lNodeAttribute)
			return;
		auto type = lNodeAttribute->GetAttributeType();
		if (type == FbxNodeAttribute::eMesh ||
			type == FbxNodeAttribute::eNurbs ||
			type == FbxNodeAttribute::eNurbsSurface ||
			type == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter lConverter(node->GetFbxManager());
			lConverter.Triangulate(scene, true);
		}

		int childs = node->GetChildCount();
		for (int i = 0; i < childs; i++)
		{
			TriangulateRecursive(node->GetChild(i));
		}
	}

	void CopyVertexData(FbxMesh* fbxMeshData)
	{
		int polygonCount = fbxMeshData->GetPolygonCount();
		this->polygonCount = polygonCount;
		vertexes = new Vertex[polygonCount];


		FbxStringList uvsetName;
		fbxMeshData->GetUVSetNames(uvsetName);
		int numUVSet = uvsetName.GetCount();
		//elements.numUVSet = numUVSet;
		bool unmapped = false;
		
		for (int i = 0; i < polygonCount; i++)
		{

			for (int j = 0; j < 3; j++)
			{
				int idx = fbxMeshData->GetPolygonVertex(i, j);

				vertexes[idx].position.x = fbxMeshData->GetControlPointAt(idx).mData[0];
				vertexes[idx].position.y = fbxMeshData->GetControlPointAt(idx).mData[1];
				vertexes[idx].position.z = fbxMeshData->GetControlPointAt(idx).mData[2];


				FbxVector4 normal = fbxMeshData->GetElementNormal(0)->GetDirectArray().GetAt(idx);
				vertexes[idx].normal.x = normal.mData[0];
				vertexes[idx].normal.y = normal.mData[1];
				vertexes[idx].normal.z = normal.mData[2];

				for (int uv = 0; uv < numUVSet; uv++)
				{
					//TODO
					FbxString name = uvsetName.GetStringAt(uv);
					FbxVector2 texCoord;
					fbxMeshData->GetPolygonVertexUV(i, j, name, texCoord, unmapped);
					//mesh->m_texcoordArray.push_back(texCoord);
				}
			}
		}
	}
};

HRESULT FbxLoader::LoadFBX(const wchar_t* p, SkinnedData& skinned)
{
	FbxScene* scene = FbxScene::Create(manager, "Scene");
	if (nullptr == scene)
		return E_FAIL;

	FbxImporter* importer = FbxImporter::Create(manager, "Importer");
	if (importer == nullptr)
		return E_FAIL;

	if (importer->Initialize("dragon.fbx", -1, manager->GetIOSettings()) == false)
		return E_FAIL;

	if (importer->Import(scene, fbxNonblocking) == false)
		return E_FAIL;


	TestClass data(skinned);
	data.TriangulateRecursive(scene->GetRootNode());
	data.Setup(scene->GetRootNode(), scene);


	scene->Destroy();
	importer->Destroy();

	return S_OK;
}


HRESULT FbxLoader::Init()
{
	bool fbxNonblocking = false;

	manager = FbxManager::Create();
	if (nullptr == manager)
		return E_FAIL;

	ioSet = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ioSet);

	return S_OK;
}
