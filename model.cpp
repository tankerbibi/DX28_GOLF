#include "directX.h"

#include "DirectXTex.h"  // DirectXTexというライブラリを使う。

#include "texture.h"
#include "model.h"

MODEL* ModelLoad( const char *FileName)
{
	MODEL* model = new MODEL;


	const std::string modelPath( FileName );  // stringに変換

	model->AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);  // asimpの真骨頂 directXの座標に変換
	assert(model->AiScene);  // 読み込めたかどうか確認している。

	model->VertexBuffer = new ID3D11Buffer*[model->AiScene->mNumMeshes];  // パーツ（メッシュ）の数を取得
	model->IndexBuffer = new ID3D11Buffer*[model->AiScene->mNumMeshes];


	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)  // ループで数を取得
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// 頂点バッファ生成
		{
			Vertex* vertex = new Vertex[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].position = XMFLOAT3(mesh->mVertices[v].x, -mesh->mVertices[v].z, mesh->mVertices[v].y);

				if (mesh->HasTextureCoords(0))
				{
					vertex[v].texcoord = XMFLOAT2( mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				}
				else
				{
					vertex[v].texcoord = XMFLOAT2(0.0f, 0.0f);
				}
				// vertex[v].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, -mesh->mNormals[v].z, mesh->mNormals[v].y);
			
				//for (int i = 0; i < 4; i++)  // 頂点に対するボーンの情報を初期化
				//{
				//	vertex[v].bone_indices[i] = 0;
				//	vertex[v].bone_weights[i] = 0.0f;
				//}
			}

			/*for (unsigned int b = 0; b < mesh->mNumBones; b++)
			{
				aiBone* bone = mesh->mBones[b];
				std::string boneName(bone->mName.data);
				int boneIndex = 0;

				if (model->boneMapping.find(boneName) == model->boneMapping.end())
				{
					boneIndex = model->numBones;
					model->numBones++;
					BoneInfo bi;
					bi.offsetMatrix = bone->mOffsetMatrix;
					
				}
			}*/

			// 頂点バッファ作成。それにデータも入れてしまう。
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(Vertex) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex;

			DirectXGetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer[m]);

			delete[] vertex;
		}


		// インデックスバッファ生成
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				//assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			DirectXGetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer[m]);

			delete[] index;
		}

	}

	//テクスチャ読み込み fbxにテクスチャ内蔵方式
	for(int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = model->AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICMemory(aitexture->pcData, aitexture->mWidth, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(DirectXGetDevice(), image.GetImages(), image.GetImageCount(), metadata, &texture);
		assert(texture);

		model->Texture[aitexture->mFilename.data] = texture;
	}

	return model;
}


// メモリ全部解放
void ModelRelease(MODEL* model)
{
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		model->VertexBuffer[m]->Release();
		model->IndexBuffer[m]->Release();
	}

	delete[] model->VertexBuffer;
	delete[] model->IndexBuffer;

	for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : model->Texture)
	{
		pair.second->Release();
	}

	aiReleaseImport(model->AiScene);

	delete model;
}



void ModelDraw(MODEL* model)
{
	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // assimpはTriangleListに対応している


	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// テクスチャ設定
		aiString texture;
		aiMaterial* aimaterial = model->AiScene->mMaterials[mesh->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture != aiString(""))
			DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &model->Texture[texture.data]);

		// 頂点バッファ設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// インデックスバッファ設定
		DirectXGetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// ポリゴン描画
		DirectXGetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}

void ModelDrawInstanced(MODEL* model, ID3D11Buffer* pInstanceBuffer,UINT instanceCount)
{
	if (model == nullptr || instanceCount == 0) return;

	ID3D11DeviceContext* context = DirectXGetDeviceContext();

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// テクスチャ設定
		aiString texture;
		aiMaterial* aimaterial = model->AiScene->mMaterials[mesh->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture != aiString(""))
			context->PSSetShaderResources(0, 1, &model->Texture[texture.data]);

		// スロット０に頂点バッファを設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// インデックスバッファ設定
		context->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// スロット1にインスタンスバッファを設定
		//ID3D11Buffer* pInstanceBuffer = GetInstanceBuffer();
		UINT instanceStride = sizeof(InstanceData);
		UINT instanceOffset = 0;
		context->IASetVertexBuffers(1, 1, &pInstanceBuffer, &instanceStride, &instanceOffset);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexedInstanced(
			mesh->mNumFaces * 3,  // このメッシュのインデックス数
			instanceCount,  // 描画するインスタンスの総数
			0, 0, 0
		);
	}
	

	// スロット１をnullptrで上書きして、インスタンスバッファの設定を解除する
	ID3D11Buffer* nullBuffer = nullptr;
	UINT zeroStride = 0;
	UINT zeroOffset = 0;
	context->IASetVertexBuffers(1, 1, &nullBuffer, &zeroStride, &zeroOffset);

	context->IASetVertexBuffers(0, 1, &nullBuffer, &zeroStride, &zeroOffset);  // スロット１の初期化はあってもなくても変わらない。
}
