#pragma once

#include <unordered_map>

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

struct BoneInfo
{
	aiMatrix4x4 offsetMatrix;
	aiMatrix4x4 finalTransform;
};

struct MODEL
{
	const aiScene* AiScene = nullptr;

	ID3D11Buffer** VertexBuffer;
	ID3D11Buffer** IndexBuffer;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture;  // mapはunreal engineのやつ。配列の進化版。
	std::unordered_map<std::string, int> boneMapping;
	BoneInfo boneInfo[100];
	int numBones = 0;
};


MODEL* ModelLoad(const char* FileName);
void ModelRelease(MODEL* model);
void ModelDraw(MODEL* model);
