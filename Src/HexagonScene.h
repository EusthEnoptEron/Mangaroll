#pragma once

#include "ShaderManager.h"
#include "VrApi.h"
#include "View.h"
#include "IScene.h"
//#include "Sparkle.h"
#include "ParticleSystem.h"

using namespace OVR;

namespace OvrMangaroll {
	class HexagonScene : public IScene {
	public:
		HexagonScene() 
			: IScene()
			, _Particles()
			, _Color(0, 0.5f, 1.0f)
		{
			//_Particles.p_size_min = 1;
			//_Particles.p_size_max = 10;

			_Shader = ShaderManager::Instance()->Get(SHADER_VERT_FLOOR);
			glUseProgram(_Shader->program);
			uFalloffDistance = glGetUniformLocation(_Shader->program, "uFalloffDistance");
			uTint = glGetUniformLocation(_Shader->program, "uTint");
			uTime = glGetUniformLocation(_Shader->program, "uTime");
			glUniform3f(uTint, 1, 1.5f, 1.5f);

			LOG("my shader: %p", _Shader);

			// Create geometry
			VertexAttribs attribs;
			Array< TriangleIndex > indices;

			attribs.position.Resize(4);
			attribs.uv0.Resize(4);
			
			attribs.position[0] = Vector3f(-FLOOR_WIDTH * 0.5f, FLOOR_HEIGHT, -FLOOR_DEPTH * 0.5f); //TL
			attribs.position[1] = Vector3f(FLOOR_WIDTH * 0.5f, FLOOR_HEIGHT, -FLOOR_DEPTH * 0.5f); //TR
			attribs.position[2] = Vector3f(FLOOR_WIDTH * 0.5f, FLOOR_HEIGHT, FLOOR_DEPTH * 0.5f); //BR
			attribs.position[3] = Vector3f(-FLOOR_WIDTH * 0.5f, FLOOR_HEIGHT, FLOOR_DEPTH * 0.5f); //BL
			//LOG("RECT: [%.2f, %.2f, %.2f]", attribs.position[0].x, attribs.position[0].y, attribs.position[0].z);

			attribs.uv0[0] = Vector2f(0, 1);
			attribs.uv0[1] = Vector2f(1, 1);
			attribs.uv0[2] = Vector2f(1, 0);
			attribs.uv0[3] = Vector2f(0, 0);

			indices.Resize(6);
			indices[0] = 0;
			indices[1] = 2;
			indices[2] = 1;
			indices[3] = 0;
			indices[4] = 3;
			indices[5] = 2;
			//LOG("FLOOR CREATED (%d, %d, %d)", FLOOR_WIDTH, FLOOR_HEIGHT, FLOOR_DEPTH);
			_Floor.Create(attribs, indices);

			_Dome = BuildDome(2 * Mathf::Pi, 1.0f, 1.0f);
			_Dome.primitiveType = GL_LINE_LOOP;

			// Load texture
			int width, height;
			_Texture = LoadTextureFromApplicationPackage("assets/hexagon_psd.png", TextureFlags_t(), width, height);
		}

		virtual ~HexagonScene() {}

		virtual void OnOpen() {
			for (int i = 0; i < 100; i++) {
				SpawnParticle(i, 1);
			}
		}


		virtual void OnClose() {

		}

		virtual bool OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) { return false; }
		virtual void Frame(const VrFrame & vrFrame) {
			for (int i = 0; i < vrFrame.DeltaSeconds * 70; i++) {
				SpawnParticle(i, 0);
			}

			_Particles.Simulate(vrFrame.DeltaSeconds);
		}
		

		virtual void DrawEyeView(const Matrix4f &eyeViewMatrix, const Matrix4f &eyeProjectionMatrix, const int eye) {
			glUseProgram(_Shader->program);
			glUniform3f(uTime, Time::Elapsed, sinf(Time::Elapsed), cosf(Time::Elapsed));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(_Texture.target, _Texture.texture);
			// Set matrices
			glUniformMatrix4fv(_Shader->uView, 1, GL_TRUE, eyeViewMatrix.M[0]);
			glUniformMatrix4fv(_Shader->uProjection, 1, GL_TRUE, eyeProjectionMatrix.M[0]);

			Vector3f col = Color::HSL2RGB(Vector3f(fmod(Time::Elapsed * 0.01f, 1.0f), 0.25f, .4f));
			glUniform3f(uTint, col.x, col.y, col.z);

			glUniform3f(uFalloffDistance, 6.0f, 0.1f, 0.0f);
			_Floor.Draw();
			glUniform3f(uFalloffDistance, 100.0f, 20.0f, 10.0f);
			glBindTexture(GL_TEXTURE_2D, Assets::Instance().Fill.Texture);

			_Dome.Draw();

			glBindVertexArray(0);
			glBindTexture(_Texture.target, 0);

			glUseProgram(0);
			//glDepthMask(0);
			_Particles.Render(eyeViewMatrix, eyeProjectionMatrix);
			//glDepthMask(1);

		}

	private:
		void SpawnParticle(int i, float a) {
			Particle p;
			p.pos = Vector3f(rand() % 1000 * 0.01f - 5, 
				rand() % 100 * 0.03f - 1.0f,
				rand() % 1000 * 0.01f - 5);
			p.r = (rand() % 100) + 155;
			p.g = (rand() % 100) + 155;
			p.b = (rand() % 100) + 155;
			p.a = a;
			p.life = 10;
			p.speed = Vector3f(0, 0.05f, 0);
			p.size = 0.01f;

			if (Vector2f(p.pos.x, p.pos.z).Length() > 3.0f)
				_Particles.Spawn(p);
		}

		GlProgram *_Shader;
		GlGeometry _Floor;
		GlGeometry _Dome;
		GlTexture _Texture;
		ParticleSystem _Particles;
		GLint uFalloffDistance;
		Vector3f _Color;
		GLint uTint;
		GLint uTime;
		static const int FLOOR_WIDTH = 174;
		static const int FLOOR_DEPTH = 100;
		static const int FLOOR_HEIGHT = -2;

	};
}