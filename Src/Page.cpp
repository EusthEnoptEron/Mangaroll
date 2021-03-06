#include "Page.h"
#include <math.h>
#include "PackageFiles.h"
#include "Kernel/OVR_MemBuffer.h"
#include "stb_image.h"
#include "ImageData.h"
#include "Helpers.h"
#include "Config.h"

namespace OvrMangaroll {

	const int Page::SEGMENTS = 10;
	const float Page::HEIGHT = 1.0f;
	const float Page::RADIUS = 1.0f;

	Page::~Page(void)
	{
		if (_ATexture->GetState() >= TEXTURE_LOADED) {
			_Geometry.Free();
		}
		_ATexture->Unload();
	}

	Page *Page::GetNext() {
		return _Next;
	}

	Page *Page::GetPrev() {
		return _Prev;
	}

	void Page::SetPrev(Page *p) {
		_Prev = p;

		if (_ATexture->GetState() >= TEXTURE_LOADED) {
			p->_PixelRange.SetEnd(_PixelRange.GetStart());
			p->_AngularRange.SetEnd(_AngularRange.GetStart());
		}
	}

	bool Page::IsLoaded() {
		return _ATexture->GetState() >= TEXTURE_LOADED;
	}

	bool Page::IsVisible() {
		return _DisplayState == DisplayState::VISIBLE;
	}
	
	bool Page::IsTarget(float angle) {
		if(IsVisible()) {
			Vector3f pos = Position;
			pos.y = 0;
			float zoom = pos.Length(); // 0/0/0 is default. Moves toward the camera when selected
			float margin = zoom * _AngularRange.GetLength() * 0.5f;
			float angularOffset = _AngularRange.GetStart() - margin;
			float angularEnd = _AngularRange.GetEnd() + margin;

			return angle > angularOffset && angle < angularEnd;
		}
		return false;
	}

	bool Page::IsBefore(float angle) {
		return (_AngularRange.GetEnd() != RANGE_UNDEFINED && _AngularRange.GetEnd() < angle);
	}
	bool Page::IsAfter(float angle) {
		return (_AngularRange.GetStart() != RANGE_UNDEFINED && _AngularRange.GetStart() > angle);
	}

	void Page::SetSelected(bool state) {
		_Selected = state;
		_SelectionStart = Time::Elapsed;
	}

	void Page::UpdateStates(float angle) {
		_DisplayState = DisplayState::INVISIBLE;

		float pixelStart = angle * PIXELS_PER_DEGREE - 90 * PIXELS_PER_DEGREE;
		float pixelEnd   = angle * PIXELS_PER_DEGREE + 90 * PIXELS_PER_DEGREE;
		bool textureLoaded = _ATexture->GetState() >= TEXTURE_LOADED;

		if(IsPositionable()) {
			bool initialIsVisible = (_PixelRange.GetStart() != RANGE_UNDEFINED || textureLoaded)
				? _PixelRange.GetStart() > pixelStart && _PixelRange.GetStart() < pixelEnd // Right end within view
				: _PixelRange.GetEnd() > pixelStart && _PixelRange.GetEnd() < pixelEnd; // left end within view

			// "On-Load" if unloaded
			Load();

			// If user's view is inside the valid range...
			if (initialIsVisible || (textureLoaded && _PixelRange.GetEnd() > pixelStart && _PixelRange.GetEnd() < pixelEnd)) {

			
				_DisplayState = DisplayState::VISIBLE;

				if (_ATexture->GetState() == TEXTURE_UNLOADED) {
					_ATexture->Load();
				} else if (_ATexture->GetState() == TEXTURE_LOADED) {
					_ATexture->Display();
				}

			} else {
				// Otherwise - disappear!
				_DisplayState = DisplayState::INVISIBLE;

				_ATexture->Hide();

				if (_ATexture->GetFutureState() != TEXTURE_UNLOADED) {
					if (abs(angle - _AngularRange.GetStart()) > 720) {
						_ATexture->Unload();
						_Geometry.Free();
						_Loaded = false;
					}
				}
				//LOG("DONT DRAW %s", _Path.ToCStr());
			}
		} else {
			_DisplayState = DisplayState::INVISIBLE;
		}
	}

	void Page::Update(float angle, bool onlyVisual) {
		if (!onlyVisual) {
			UpdateStates(angle);
		}

		if(_DisplayState == DisplayState::VISIBLE) {
			if(_Selected) {
				float radianOffset = Mathf::Pi / 2;// - widthInRadians / 2; // Makes sure this thing is centered
				radianOffset += DegreeToRad(_AngularRange.GetStart());
				radianOffset += DegreeToRad(_AngularRange.GetLength()) / 2.0f;

				float x = cos(radianOffset) * RADIUS;
				float z = -sin(radianOffset) * RADIUS;

				if (AppState::Conf->LeftToRight) {
					z *= -1;
				}

				float distance = AppState::Conf->Zoom * 0.7f;

				Vector3f targetPos = Vector3f(-x, 0.0f, -z) * distance;
				if (AppState::Conf->Guided) {
					float maxAngle = atan( (HEIGHT / 2) / RADIUS );
					float verticalAngle = Acos(HMD::NormalizedDirection.ProjectToPlane(Vector3f(0.0f, 1.0f, 0.0f)).Length());
				
					if (HMD::NormalizedDirection.y < 0) verticalAngle *= -1;

					float verticalShift = fmax(-1, fmin(1, verticalAngle / maxAngle));

					targetPos += (-verticalShift * (HEIGHT * PAGE_LOOK_SENSITIVIY) * Vector3f(0, 1, 0));
				}

				/*LOG("Target Position: %.2f/%.2f/%.2f", targetPos.x, targetPos.y, targetPos.z);
				LOG("Current Position: %.2f/%.2f/%.2f", Position.x, Position.y, Position.z);
				*/
				Position = Position.Lerp(targetPos, Time::Delta * 10);

				Touch();
			} else {
				Position = Position.Lerp(Vector3f::ZERO, Time::Delta * 10);
				Touch();
			}
		}
	}

	void Page::Load() {
		if (!_Loaded && _ATexture->GetState() == TEXTURE_LOADED) {
			_Loaded = true;

			// Calculate real width
			_PixelRange.SetLength(Alg::Min(REFERENCE_MAX_WIDTH, REFERENCE_HEIGHT / _ATexture->GetHeight() * _ATexture->GetWidth()));
			_AngularRange.SetLength(_PixelRange.GetLength() / PIXELS_PER_DEGREE);

			if(_Next != NULL) {
				_Next->_PixelRange.SetStart(_PixelRange.GetEnd());
				_Next->_AngularRange.SetStart(_AngularRange.GetEnd());
			}

			if (_Prev != NULL) {
				_Prev->_PixelRange.SetEnd(_PixelRange.GetStart());
				_Prev->_AngularRange.SetEnd(_AngularRange.GetStart());
			}

			LOG("LOADED %s", _Path.ToCStr());
			CreateMesh();
			
			if (!_Initialized) {
				// Only do this once

				_Progs[0] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
				_Progs[1] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME, PAGE_TRANSPARENT_FRAG_NAME);

				_uDisplayTime[0] = glGetUniformLocation(_Progs[0]->program, "DisplayTime");
				_uDisplayTime[1] = glGetUniformLocation(_Progs[1]->program, "DisplayTime");


				_uContrast[0] = glGetUniformLocation(_Progs[0]->program, "Contrast");
				_uContrast[1] = glGetUniformLocation(_Progs[1]->program, "Contrast");
				_uBrightness[0] = glGetUniformLocation(_Progs[0]->program, "Brightness");
				_uBrightness[1] = glGetUniformLocation(_Progs[1]->program, "Brightness");

				_Initialized = true;
			}

			_DisplayTime = Time::Elapsed;
		}
	}

	void Page::Reset(void) {
		float width = _PixelRange.GetLength();
		float aWidth = _AngularRange.GetLength();
		_AngularRange.Reset();
		_PixelRange.Reset();
		_AngularRange.SetLength(aWidth);
		_PixelRange.SetLength(width);



		_Loaded = false;

		if (_ATexture->GetState() > TEXTURE_LOADED) {
			_Geometry.Free();
		}
		_ATexture->Unload();

		_DisplayState = DisplayState::INVISIBLE;
	}


	void Page::Draw(const Matrix4f &view, const Matrix4f &proj) {
		if (this->_DisplayState == DisplayState::VISIBLE) {
			this->UpdateModel();

			if (_ATexture->GetState() == TEXTURE_APPLIED) {
				int index = AppState::Conf->Transparent ? 1 : 0;
				// Draw
				glUseProgram(_Progs[index]->program);

				glUniformMatrix4fv(_Progs[index]->uView, 1, GL_TRUE, view.M[0]);
				glUniformMatrix4fv(_Progs[index]->uProjection, 1, GL_TRUE, proj.M[0]);
				glUniform1f(_uContrast[index], AppState::Conf->Contrast);
				glUniform1f(_uBrightness[index], AppState::Conf->Brightness);

				glUniform1f(_uDisplayTime[index], Time::Elapsed - this->_DisplayTime);
				glUniformMatrix4fv(_Progs[index]->uModel, 1, GL_TRUE, Mat.M[0]);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(_ATexture->GetTarget(), _ATexture->Display());
				_Geometry.Draw();
				glBindVertexArray(0);
				glBindTexture(_ATexture->GetTarget(), 0);

				glUseProgram(0);
			}
		}
		
	}

	void Page::SetNext(Page *next) {
		_Next = next;

		if (_ATexture->GetState() >= TEXTURE_LOADED) {
			next->_PixelRange.SetStart(_PixelRange.GetEnd());
			next->_AngularRange.SetStart(_AngularRange.GetEnd());
		}
	}

	
	// Creates a mesh
	// Precondition: Width is known
	void Page::CreateMesh(void)
	{
		LOG("CREATE MESH %s", _Path.ToCStr());
		//_Geometry = GlGeometry::Create();

		// Create the quad.
		VertexAttribs attribs;
		Array< TriangleIndex > indices;
		attribs.position.Resize( Page::SEGMENTS * 2 );
		attribs.uv0.Resize( Page::SEGMENTS * 2 );
		indices.Resize( (Page::SEGMENTS - 1) * 2 * 3 ); // Number of faces * triangles per face * vertices per triangle

		float widthInRadians = DegreeToRad(_AngularRange.GetLength());
		float radianOffset = Mathf::Pi / 2;// - widthInRadians / 2; // Makes sure this thing is centered
		radianOffset += DegreeToRad(_AngularRange.GetStart());

		float y0 = -Page::HEIGHT / 2.0f;
		float y1 = +Page::HEIGHT / 2.0f;
		int index = 0;
		
		int off1 = 0;
		int off2 = AppState::Conf->LeftToRight ? 2 : 1;
		int off3 = AppState::Conf->LeftToRight ? 1 : 2;

		for ( int i = 0; i < Page::SEGMENTS; i++ ) {
			float progress =  (i / float(Page::SEGMENTS - 1));
			float x = cos( progress * widthInRadians + radianOffset ) * RADIUS;
			float z = -sin( progress * widthInRadians + radianOffset) * RADIUS;

			if (AppState::Conf->LeftToRight) {
				z *= -1;
			}

			attribs.position[i * 2] = Vector3f(x, y0, z);
			attribs.position[i * 2 + 1] = Vector3f(x, y1, z);
			//LOG("V1: (%.2f, %.2f, %.2f)", x, y0, z);
			//LOG("V2: (%.2f, %.2f, %.2f)", x, y1, z);

			attribs.uv0[i * 2] = Vector2f(1 - progress, 1);
			attribs.uv0[i * 2 + 1] = Vector2f(1 - progress, 0);
			if (AppState::Conf->LeftToRight) {
				attribs.uv0[i * 2].x = 1 - attribs.uv0[i * 2].x;
				attribs.uv0[i * 2+1].x = 1 - attribs.uv0[i * 2+1].x;
			}

			if(i > 0) {
				// Add index
				// T1
				indices[index + off1] = (i * 2 - 1);
				indices[index + off2] = (i * 2 + 1);
				indices[index + off3] = (i * 2 - 2);
				index += 3;

                // T2
				indices[index + off1] = (i * 2 + 1);
				indices[index + off2] = (i * 2);
				indices[index + off3] = (i * 2 - 2);
				index += 3;
			}
		}


		_Geometry.Create(attribs, indices);
	}
}
