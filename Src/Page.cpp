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

	void Page::SetPrev(Page *p) {
		_Prev = p;

		if (_ATexture->GetState() >= TEXTURE_LOADED) {
			p->SetHighOffset(_Offset);
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
			float margin = zoom * _AngularWidth * 0.5f;
			float angularOffset = _AngularOffset - margin;
			float angularEnd = _AngularOffset + _AngularWidth + margin;

			//// Calculate the angle of the isosceles triangle
			//float x = Alg::Clamp(-(_ChordLength * _ChordLength) / (2 * distance * distance), -1.0f, 1.0f);
			//float angularWidth = acosf(x) * Mathf::RadToDegreeFactor;
			//if (_Selected) {
			//	LOG("Distance: %.2f Angular Width: %.2f Chord: %.2f, Cos: %.2f", distance, angularWidth, _ChordLength, 1 - (_ChordLength * _ChordLength) / (2 * distance * distance));
			//}

			//if (angularWidth == NAN) return true;

			//float angularOffset = _AngularOffset - (angularWidth - _AngularWidth) / 2;
			//float angleEnd = angularOffset + angularWidth;

			return angle > angularOffset && angle < angularEnd;
		}
		return false;
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

		if(_Positionable) {
			int left = (_Offset + _Width);

			bool initialIsVisible = (_Origin == PLACING_BOTTOM || textureLoaded)
				? _Offset > pixelStart && _Offset < pixelEnd // Right end within view
				: _HighOffset > pixelStart && _HighOffset < pixelEnd; // left end within view

			// "On-Load" if unloaded
			Load();

			// If user's view is inside the valid range...
			if (initialIsVisible || (textureLoaded && left > pixelStart && left < pixelEnd)) {

			
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
					float degreeStart = _Offset / PIXELS_PER_DEGREE;
					if (abs(angle - degreeStart) > 720) {
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
				radianOffset += DegreeToRad(_AngularOffset);
				radianOffset += DegreeToRad(_AngularWidth) / 2.0f;

				float x = cos(radianOffset) * RADIUS;
				float z = -sin(radianOffset) * RADIUS;

				if (AppState::Conf->LeftToRight) {
					z *= -1;
				}

				float distance = AppState::Conf->Zoom * 0.7f;

				Vector3f targetPos = Vector3f(-x, 0.0f, -z) * distance;
				if (AppState::Conf->Guided) {
					float maxAngle = atan( (HEIGHT / 2) / RADIUS );
					float verticalAngle = Acos(HMD::Direction.ProjectToPlane(Vector3f(0.0f, 1.0f, 0.0f)).Length());
				
					if(HMD::Direction.y < 0) verticalAngle *= -1;

					float verticalShift = fmax(-1, fmin(1, verticalAngle / maxAngle));

					targetPos += (-verticalShift * (HEIGHT / 3) * Vector3f(0, 1, 0));
				}

				LOG("Target Position: %.2f/%.2f/%.2f", targetPos.x, targetPos.y, targetPos.z);
				LOG("Current Position: %.2f/%.2f/%.2f", Position.x, Position.y, Position.z);
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
			_Width = REFERENCE_HEIGHT / _ATexture->GetHeight() * _ATexture->GetWidth();
			_AngularWidth  = _Width / PIXELS_PER_DEGREE;
			_ChordLength = 2 * RADIUS * sinf(_AngularWidth * Mathf::DegreeToRadFactor / 2);

			if (_Origin == PLACING_TOP) {
				// Coming from the top!
				_Offset = _HighOffset - _Width;
				_AngularOffset = _Offset / PIXELS_PER_DEGREE;
			}

			if(_Next != NULL) {
				_Next->SetOffset(_Offset + _Width);
			}

			if (_Prev != NULL) {
				_Prev->SetHighOffset(_Offset);
			}

			LOG("LOADED %s", _Path.ToCStr());
			CreateMesh();
			
			if (!_Initialized) {
				// Only do this once

				_Progs[0] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
				_Progs[1] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME, PAGE_TRANSPARENT_FRAG_NAME);

				_uDisplayTime[0] = glGetUniformLocation(_Progs[0]->program, "DisplayTime");
				_uDisplayTime[1] = glGetUniformLocation(_Progs[1]->program, "DisplayTime");

				_Initialized = true;
			}

			_DisplayTime = Time::Elapsed;
		}
	}

	void Page::Reset(void) {
		_Positionable = false;
		_Origin = PLACING_NONE;
		_Offset = 0;
		_AngularOffset = 0;
		_HighOffset = 0;
		_Loaded = false;

		if (_ATexture->GetState() > TEXTURE_LOADED) {
			_Geometry.Free();
		}
		_ATexture->Unload();

		_DisplayState = DisplayState::INVISIBLE;
	}

	void Page::SetOffset(int offset) {
		if (_Origin == PLACING_NONE) {
			_Offset = offset;
			_AngularOffset = _Offset / PIXELS_PER_DEGREE;
			_Positionable = true;
			_Origin = PLACING_BOTTOM;
		}
	}

	void Page::SetHighOffset(int offset) {
		if (_Origin == PLACING_NONE) {
			_HighOffset = offset;
			_Positionable = true;
			_Origin = PLACING_TOP;
		}
		
	}

	void Page::Draw(const Matrix4f &m) {
		if(this->_DisplayState == DisplayState::VISIBLE && _ATexture->GetState() == TEXTURE_APPLIED) {
			this->UpdateModel();

			int index = AppState::Conf->Transparent ? 1 : 0;
			// Draw
			
			glUniform1f(_uDisplayTime[index], Time::Elapsed - this->_DisplayTime);
			//glUniform1f(_uDisplayTime[index], Time::Elapsed - this->_DisplayTime);
			// ^--- WHY IN THE NAME OF FUCKING JESUS CHRIST DOES THE UNIFORM LOCATION CHANGE HERE?!
			glUniformMatrix4fv(_Progs[index]->uModel, 1, GL_TRUE, (m * Mat).M[0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(_ATexture->GetTarget(), _ATexture->Display());
			_Geometry.Draw();
			glBindVertexArray( 0 );
			glBindTexture(_ATexture->GetTarget(), 0);
		}
	}

	void Page::SetNext(Page *next) {
		_Next = next;

		if (_ATexture->GetState() >= TEXTURE_LOADED) {
			next->SetOffset(_Offset + _Width);
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

		float widthInRadians = DegreeToRad(_AngularWidth);
		float radianOffset = Mathf::Pi / 2;// - widthInRadians / 2; // Makes sure this thing is centered
		radianOffset += DegreeToRad(_AngularOffset);

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
