#pragma once

#include "ShaderManager.h"
#include "VrApi.h"
#include "View.h"
#include "IScene.h"

using namespace OVR;

namespace OvrMangaroll {
	class HexagonScene : public IScene {
	public:
		HexagonScene() 
			: IScene()
		{
			_Shader = ShaderManager::Instance()->Get(SHADER_VERT_FLOOR);
			glUniform3f(glGetUniformLocation(_Shader->program, "uTint"), 0, 0.5f, 0.5f);
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

			// Load texture
			int width, height;
			_Texture = LoadTextureFromApplicationPackage("assets/hexagon_psd.png", TextureFlags_t(), width, height);
		}

		virtual ~HexagonScene() {}

		virtual void OnOpen() {

		}

		virtual void OnClose() {

		}

		virtual bool OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) { return false; }
		virtual void Frame(const VrFrame & vrFrame) {
		
		}
		
		virtual void DrawEyeView(const Matrix4f &eyeViewMatrix, const Matrix4f &eyeProjectionMatrix, const int eye) {
			LOG("Set shader: %d", _Shader->program);
			glUseProgram(_Shader->program);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(_Texture.target, _Texture.texture);
			// Set matrices
			glUniformMatrix4fv(_Shader->uView, 1, GL_TRUE, eyeViewMatrix.M[0]);
			glUniformMatrix4fv(_Shader->uProjection, 1, GL_TRUE, eyeProjectionMatrix.M[0]);
			
			_Floor.Draw();

			glBindTexture(_Texture.target, 0);

		}

	private:
		GlProgram *_Shader;
		GlGeometry _Floor;
		GlTexture _Texture;
		static const int FLOOR_WIDTH = 174;
		static const int FLOOR_DEPTH = 100;
		static const int FLOOR_HEIGHT = -2;

	};
}