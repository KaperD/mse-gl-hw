#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QVector2D>
#include <QVector3D>

#include <string>
#include <utility>
#include <vector>

struct Vertex {
	QVector3D position;
	QVector3D normal;
	QVector2D tex_coords;
	QVector3D tangent;
	QVector3D bitangent;
};

enum class TextureType
{
	diffuse,
	specular,
	normal,
	metallic
};

struct Texture {
	unsigned int id;
	TextureType type;
	std::string path;
};

enum class LightType {
	vertex = 1,
	fragment = 2,
	map = 3
};

class Mesh
{
	std::vector<Vertex> vertices_;
	std::vector<unsigned int> indices_;
	std::vector<Texture> textures_;
	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLBuffer ibo_{QOpenGLBuffer::Type::IndexBuffer};
	QOpenGLVertexArrayObject vao_;
	int light_type_ = static_cast<int>(LightType::map);

public:
	Mesh(
		const std::vector<Vertex> & vertices,
		const std::vector<unsigned int> & indices,
		const std::vector<Texture> & textures,
		QOpenGLShaderProgram & program)
	{
		this->vertices_ = vertices;
		this->indices_ = indices;
		this->textures_ = textures;

		setupMesh(program);
	}

	void Draw(QOpenGLShaderProgram & program, QOpenGLFunctions & functions, int order)
	{
		unsigned int diffuse_number = 1;
		unsigned int specular_number = 1;
		unsigned int normal_number = 1;
		unsigned int metallic_number = 1;
		for (unsigned int i = 0; i < textures_.size(); i++)
		{
			functions.glActiveTexture(GL_TEXTURE0 + i);
			std::string uniform;
			TextureType type = textures_[i].type;
			if (type == TextureType::diffuse)
			{
				uniform = "texture_diffuse" + std::to_string(diffuse_number++);
			}
			else if (type == TextureType::specular)
			{
				uniform = "texture_specular" + std::to_string(specular_number++);
			}
			else if (type == TextureType::normal)
			{
				uniform = "texture_normal" + std::to_string(normal_number++);
			}
			else if (type == TextureType::metallic)
			{
				uniform = "texture_metallic" + std::to_string(metallic_number++);
			}

			program.setUniformValue(program.uniformLocation(uniform.c_str()), i);
			functions.glBindTexture(GL_TEXTURE_2D, textures_[i].id);
		}
		program.setUniformValue(program.uniformLocation("lightType"), light_type_ + 1);
		program.setUniformValue(program.uniformLocation("id"), order);

		vao_.bind();
		functions.glDrawElements(GL_TRIANGLES, static_cast<int>(indices_.size()), GL_UNSIGNED_INT, nullptr);
		vao_.release();

		functions.glActiveTexture(GL_TEXTURE0);
	}

	void Click() {
		light_type_ = (light_type_ + 1) % 3;
	}

private:
	void setupMesh(QOpenGLShaderProgram & program)
	{
		vao_.create();
		vao_.bind();

		vbo_.create();
		vbo_.bind();
		vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
		vbo_.allocate(vertices_.data(), static_cast<int>(vertices_.size() * sizeof(Vertex)));

		ibo_.create();
		ibo_.bind();
		ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
		ibo_.allocate(indices_.data(), static_cast<int>(indices_.size() * sizeof(unsigned int)));

		program.enableAttributeArray(0);
		program.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));

		program.enableAttributeArray(1);
		program.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

		program.enableAttributeArray(2);
		program.setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, tex_coords), 2, sizeof(Vertex));

		program.enableAttributeArray(3);
		program.setAttributeBuffer(3, GL_FLOAT, offsetof(Vertex, tangent), 3, sizeof(Vertex));

		program.enableAttributeArray(4);
		program.setAttributeBuffer(4, GL_FLOAT, offsetof(Vertex, bitangent), 3, sizeof(Vertex));

		vao_.release();
	}
};
