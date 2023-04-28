#include "StarSystemSim/graphics/primitives/point_light.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

namespace graphics { namespace primitives {

    unsigned int PointLight::_VAO = 0, PointLight::_VBO = 0;
    PointLight::VertexData PointLight::_vertices[_VERTICES_COUNT * sizeof(VertexData)];


    void PointLight::draw(Shader& shader) {
        updateModelMat();

        shader.use();
        shader.setUniformMat4("_modelMat", this->modelMat);
        shader.setUniform3f("_lightCol", this->specular);

        glDrawArrays(GL_TRIANGLES, 0, _VERTICES_COUNT);
        shader.unuse();
    }

    void PointLight::bindVAO() {
        glBindVertexArray(_VAO);
    }
    void PointLight::unbindVAO() {
        glBindVertexArray(0);
    }

    PointLight::VertexData *PointLight::vertices = new PointLight::VertexData [36] {
        // positions
        {glm::vec3(-0.5f, -0.5f, -0.5f)},
        {glm::vec3( 0.5f, -0.5f, -0.5f)},
        {glm::vec3( 0.5f,  0.5f, -0.5f)},
        {glm::vec3( 0.5f,  0.5f, -0.5f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f)},
        {glm::vec3( 0.5f, -0.5f,  0.5f)},
        {glm::vec3( 0.5f,  0.5f,  0.5f)},
        {glm::vec3( 0.5f,  0.5f,  0.5f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f)},
        {glm::vec3( 0.5f,  0.5f,  0.5f)},
        {glm::vec3( 0.5f,  0.5f, -0.5f)},
        {glm::vec3( 0.5f, -0.5f, -0.5f)},
        {glm::vec3( 0.5f, -0.5f, -0.5f)},
        {glm::vec3( 0.5f, -0.5f,  0.5f)},
        {glm::vec3( 0.5f,  0.5f,  0.5f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f)},
        {glm::vec3( 0.5f, -0.5f, -0.5f)},
        {glm::vec3( 0.5f, -0.5f,  0.5f)},
        {glm::vec3( 0.5f, -0.5f,  0.5f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f)},
        {glm::vec3( 0.5f,  0.5f, -0.5f)},
        {glm::vec3( 0.5f,  0.5f,  0.5f)},
        {glm::vec3( 0.5f,  0.5f,  0.5f)},
        {glm::vec3(-0.5f,  0.5f,  0.5f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f)},
    };

    PointLight::PointLight() : Object() {
        this->scale(glm::vec3(0.1f));
        _pos = glm::vec3(0.0f, 0.0f, 0.0f);

        this->ambient = glm::vec3(0.1f);
        this->diffuse = glm::vec3(0.5f);
        this->specular = glm::vec3(1.0f);

        this->attenuation = glm::vec3(1.0f, 0.09f, 0.032f);
	}

    PointLight::~PointLight() {
	}

	void PointLight::initVBO() {
        for (int vertexInd = 0; vertexInd < _VERTICES_COUNT; ++vertexInd) {
            _vertices[vertexInd] = PointLight::vertices[vertexInd];
        }

        glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);

        glBindVertexArray(_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, VertexData::pos));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
	}

    void PointLight::destroyVBO() {
        glDeleteBuffers(1, &_VBO);
        glDeleteVertexArrays(1, &_VAO);
    }

    void PointLight::translate(const glm::vec3& trans) {
        Object::translate(trans);
        _pos += trans;
    }

    void PointLight::setPos(const glm::vec3& pos) {
        _pos = pos;
        resetTransMat();
        Object::translate(pos);
    }

} }
