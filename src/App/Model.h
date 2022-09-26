#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector2D>
#include <QVector3D>

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/contrib/stb/stb_image.h>

class Model
{
public:
	Model(const std::string & path, QOpenGLShaderProgram & program, QOpenGLFunctions * functions)
		: program_(program)
		, functions_(functions)
	{
		loadModel(path);
	}

	void Draw(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection)
	{
		for (auto & mesh: meshes_)
		{
			program_.setUniformValue(program_.uniformLocation("model"), model);
			program_.setUniformValue(program_.uniformLocation("view"), view);
			program_.setUniformValue(program_.uniformLocation("projection"), projection);
			mesh->Draw(program_, *functions_);
			model.translate(2, 0);
		}
	}

private:
	std::vector<std::unique_ptr<Mesh>> meshes_;
	std::vector<Texture> textures_loaded_;
	QOpenGLShaderProgram & program_;
	QOpenGLFunctions * functions_;
	std::string directory_;

	void loadModel(const std::string & path)
	{
		Assimp::Importer import;
		const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FixInfacingNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << import.GetErrorString() << std::endl;
			exit(1);
		}
		directory_ = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode * node, const aiScene * scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
			meshes_.push_back(processMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	std::unique_ptr<Mesh> processMesh(aiMesh * mesh, const aiScene * scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position = {
				mesh->mVertices[i].x,
				mesh->mVertices[i].z,
				mesh->mVertices[i].y,
			};
			if (mesh->HasNormals())
			{
				vertex.normal = {
					mesh->mNormals[i].x,
					mesh->mNormals[i].z,
					mesh->mNormals[i].y,
				};
			}
			if (mesh->mTextureCoords[0])
			{
				vertex.tex_coords = {
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y};
				vertex.tangent = {
					mesh->mTangents[i].x,
					mesh->mTangents[i].z,
					mesh->mTangents[i].y,
				};
				vertex.bitangent = {
					mesh->mBitangents[i].x,
					mesh->mBitangents[i].z,
					mesh->mBitangents[i].y,
				};
			}

			QVector3D cross = QVector3D::crossProduct(vertex.bitangent, vertex.tangent);
			if (
				signbit(vertex.normal.x()) != signbit(cross.x()) || signbit(vertex.normal.y()) != signbit(cross.y()) || signbit(vertex.normal.z()) != signbit(cross.z()))
			{
				QVector3D tmp = vertex.tangent;
				vertex.tangent = vertex.bitangent;
				vertex.bitangent = tmp;
				vertex.normal = -cross;
			}

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::diffuse);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::specular);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, TextureType::normal);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_METALNESS, TextureType::metallic);
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		return std::make_unique<Mesh>(vertices, indices, textures, program_);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type,
											  const TextureType & texture_type)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (auto & j: textures_loaded_)
			{
				if (std::strcmp(j.path.data(), str.C_Str()) == 0)
				{
					textures.push_back(j);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str());
				texture.type = texture_type;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded_.push_back(texture);
			}
		}
		return textures;
	}
	unsigned int TextureFromFile(const char * path)
	{
		std::string filename = path;
		filename = directory_ + '/' + filename;

		unsigned int texture_id;
		functions_->glGenTextures(1, &texture_id);

		int width, height, number_of_components;
		unsigned char * data = stbi_load(filename.c_str(), &width, &height, &number_of_components, 0);
		if (data)
		{
			GLint format;
			if (number_of_components == 1)
			{
				format = GL_RED;
			}
			else if (number_of_components == 3)
			{
				format = GL_RGB;
			}
			else if (number_of_components == 4)
			{
				format = GL_RGBA;
			}
			else
			{
				std::cout << "Wrong format" << std::endl;
				exit(1);
			}

			functions_->glBindTexture(GL_TEXTURE_2D, texture_id);
			functions_->glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			functions_->glGenerateMipmap(GL_TEXTURE_2D);

			functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			functions_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return texture_id;
	}
};
