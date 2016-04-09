#pragma once

#include "VrApi.h"
#include "ShaderManager.h"
#include "GlTexture.h"
#include "Kernel\OVR_Math.h"

using namespace OVR;

namespace OvrMangaroll {
	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};

	// CPU representation of a particle
	struct Particle{
		Vector3f pos, speed;
		unsigned char r, g, b, a; // Color
		float size, angle, weight;
		float life; // Remaining life of the particle. if < 0 : dead and unused.
		float cameradistance;
/*
		Particle()
		{}*/
	};

	class ParticleSystem {
	public:
		ParticleSystem()
			: _LastUsedParticle(0)
			, _ParticlesCount(0)
		{
			// --- INITIALIZE ---

			glGenBuffers(1, &_BillboardVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _BillboardVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

			// The VBO containing the positions and sizes of the particles
			glGenBuffers(1, &_ParticlePosVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _ParticlePosVBO);
			// Initialize with empty (NULL) buffer : it will be updated later, each frame.
			glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

			// The VBO containing the colors of the particles
			glGenBuffers(1, &_ParticleColorVBO);
			glBindBuffer(GL_ARRAY_BUFFER, _ParticleColorVBO);
			// Initialize with empty (NULL) buffer : it will be updated later, each frame.
			glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

			_Shader = ShaderManager::Instance()->Get("assets/shaders/particle2.vert");
		}

		void Spawn(const Particle &particle) {
			int i = FindUnusedParticle();
			_Particles[i] = particle;
		}

		void Simulate(float delta) {

			// --- SIMULATE
			// Simulate all particles
			_ParticlesCount = 0;
			for (int i = 0; i<MAX_PARTICLES; i++){

				Particle& p = _Particles[i]; // shortcut

				if (p.life > 0.0f){

					// Decrease life
					p.life -= delta;
					if (p.life > 0.0f){

						// Simulate simple physics : gravity only, no collisions
						//p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
						p.pos += p.speed * delta;
						p.cameradistance = p.pos.Length();

						// Fill the GPU buffer
						g_particule_position_size_data[4 * _ParticlesCount + 0] = p.pos.x;
						g_particule_position_size_data[4 * _ParticlesCount + 1] = p.pos.y;
						g_particule_position_size_data[4 * _ParticlesCount + 2] = p.pos.z;

						g_particule_position_size_data[4 * _ParticlesCount + 3] = p.size;

						g_particule_color_data[4 * _ParticlesCount + 0] = p.r;
						g_particule_color_data[4 * _ParticlesCount + 1] = p.g;
						g_particule_color_data[4 * _ParticlesCount + 2] = p.b;
						g_particule_color_data[4 * _ParticlesCount + 3] = p.a;

					}
					else{
						// Particles that just died will be put at the end of the buffer in SortParticles();
						p.cameradistance = -1.0f;
					}

					_ParticlesCount++;

				}
			}

			// --- COPY DATA

			// Update the buffers that OpenGL uses for rendering.
			// There are much more sophisticated means to stream data from the CPU to the GPU,
			// but this is outside the scope of this tutorial.
			// http://www.opengl.org/wiki/Buffer_Object_Streaming

			glBindBuffer(GL_ARRAY_BUFFER, _ParticlePosVBO);
			glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
			glBufferSubData(GL_ARRAY_BUFFER, 0, _ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

			glBindBuffer(GL_ARRAY_BUFFER, _ParticleColorVBO);
			glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
			glBufferSubData(GL_ARRAY_BUFFER, 0, _ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

		}

		void Render(const Matrix4f &viewMat, const Matrix4f &projMat) {
			LOG("%d", _ParticlesCount);

			glUseProgram(_Shader->program);
			glUniformMatrix4fv(_Shader->uView, 1, GL_TRUE, viewMat.M[0]);
			glUniformMatrix4fv(_Shader->uProjection, 1, GL_TRUE, projMat.M[0]);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_POSITION);
			glBindBuffer(GL_ARRAY_BUFFER, _BillboardVBO);
			glVertexAttribPointer(
				VERTEX_ATTRIBUTE_LOCATION_POSITION, // attribute. No particular reason for 0, but must match the layout in the shader.
				3, // size
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0, // stride
				(void*)0 // array buffer offset
				);

			// 2nd attribute buffer : positions of particles' centers
			glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_NORMAL);
			glBindBuffer(GL_ARRAY_BUFFER, _ParticlePosVBO);
			glVertexAttribPointer(
				VERTEX_ATTRIBUTE_LOCATION_NORMAL, // attribute. No particular reason for 1, but must match the layout in the shader.
				4, // size : x + y + z + size => 4
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0, // stride
				(void*)0 // array buffer offset
				);

			// 3rd attribute buffer : particles' colors
			glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_COLOR);
			glBindBuffer(GL_ARRAY_BUFFER, _ParticleColorVBO);
			glVertexAttribPointer(
				VERTEX_ATTRIBUTE_LOCATION_COLOR, // attribute. No particular reason for 2, but must match the layout in the shader.
				4, // size : r + g + b + a => 4
				GL_UNSIGNED_BYTE, // type
				GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
				0, // stride
				(void*)0 // array buffer offset
				);

			// These functions are specific to glDrawArrays*Instanced*.
			// The first parameter is the attribute buffer we're talking about.
			// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
			// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
			glVertexAttribDivisor(VERTEX_ATTRIBUTE_LOCATION_POSITION, 0); // particles vertices : always reuse the same 4 vertices -> 0
			glVertexAttribDivisor(VERTEX_ATTRIBUTE_LOCATION_NORMAL, 1); // positions : one per quad (its center) -> 1
			glVertexAttribDivisor(VERTEX_ATTRIBUTE_LOCATION_COLOR, 1); // color : one per quad -> 1

			// Draw the particules !
			// This draws many times a small triangle_strip (which looks like a quad).
			// This is equivalent to :
			// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
			// but faster.
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, _ParticlesCount);
		}

	private:
		// Finds a Particle in ParticlesContainer which isn't used yet.
		// (i.e. life < 0);
		int FindUnusedParticle(){
			for (int i = _LastUsedParticle; i<MAX_PARTICLES; i++){
				if (_Particles[i].life <= 0){
					_LastUsedParticle = i;
					return i;
				}
			}

			for (int i = 0; i<_LastUsedParticle; i++){
				if (_Particles[i].life <= 0){
					_LastUsedParticle = i;
					return i;
				}
			}

			return 0; // All particles are taken, override the first one
		}

		static const int MAX_PARTICLES = 10000;
		Particle _Particles[MAX_PARTICLES];
		GLuint _BillboardVBO;
		GLuint _ParticlePosVBO;
		GLuint _ParticleColorVBO;
		int _LastUsedParticle;
		int _ParticlesCount;
		float g_particule_position_size_data[MAX_PARTICLES * 4];
		float g_particule_color_data[MAX_PARTICLES * 4];
		GlProgram *_Shader;
	};
}