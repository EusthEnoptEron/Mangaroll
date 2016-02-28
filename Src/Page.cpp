#include "Page.h"
#include <math.h>
#include "PackageFiles.h"
#include "Kernel/OVR_MemBuffer.h"
#include "stb_image.h"
#include "ImageData.h"
#include "Helpers.h"

namespace OvrMangaroll {

	const int Page::SEGMENTS = 10;
	const float Page::HEIGHT = 1.0f;
	const float Page::RADIUS = 1.0f;

	Page::~Page(void)
	{
		if(_LoadState == LoadState::LOADED) {
			UnloadTexture();

			//Buffer.FreeData();
			free(Buffer);
			_Geometry.Free();
		}
		
	}

	Page *Page::GetNext() {
		return _Next;
	}

	bool Page::IsLoaded() {
		return _LoadState == LoadState::LOADED;
	}

	bool Page::IsVisible() {
		return _DisplayState == DisplayState::VISIBLE;
	}
	
	bool Page::IsTarget(float angle) {
		if(IsVisible()) {
			float angleEnd = _AngularOffset + _AngularWidth;
			return angle > _AngularOffset && angle < angleEnd;
		}
		return false;
	}


	void Page::SetSelected(bool state) {
		_Selected = state;
		_SelectionStart = Time::Elapsed;
	}

	void Page::UpdateStates(float angle) {
		_DisplayState = DisplayState::INVISIBLE;

		float pixelStart = angle * PIXELS_PER_DEGREE - 60 * PIXELS_PER_DEGREE;
		float pixelEnd   = angle * PIXELS_PER_DEGREE + 60 * PIXELS_PER_DEGREE;

		if(_Positionable) {
			int right = (_Offset + _Width);

			// If user's view is inside the valid range...
			if((_Offset > pixelStart && _Offset < pixelEnd) || (_LoadState == LoadState::LOADED && right > pixelStart && right < pixelEnd)) {
				_DisplayState = DisplayState::VISIBLE;

				if(_LoadState == LoadState::LOADED) {
					LoadTexture();
				}

				// Load if unloaded
				Load();
			} else {
				// Otherwise - disappear!
				_DisplayState = DisplayState::INVISIBLE;

				if(_LoadState == LoadState::LOADED) {
					UnloadTexture();

					float degreeStart = _Offset / PIXELS_PER_DEGREE;
					if(abs(angle - degreeStart) > 720) {
						free(Buffer);
						_Geometry.Free();
						_LoadState = LoadState::UNLOADED;
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
			if(_Selected && AppState::Guide >= GuideType::ENLARGE) {
				float radianOffset = Mathf::Pi / 2;// - widthInRadians / 2; // Makes sure this thing is centered
				radianOffset += DegreeToRad(_AngularOffset);
				radianOffset += DegreeToRad(_AngularWidth) / 2.0f;

				float x = cos(radianOffset) * RADIUS;
				float z = -sin(radianOffset) * RADIUS;
				float distance = AppState::Guide == GuideType::ENLARGE ? 0.2f : 0.4f;

				Vector3f targetPos = Vector3f(-x, 0.0f, -z) * distance;

				Position = Position.Lerp(targetPos, Time::Delta * 10);
				
				if(AppState::Guide == GuideType::FOLLOW) {
					float maxAngle = atan( (HEIGHT / 2) / RADIUS );
					float verticalAngle = Acos(HMD::Direction.ProjectToPlane(Vector3f(0.0f, 1.0f, 0.0f)).Length());
				
					if(HMD::Direction.y < 0) verticalAngle *= -1;

					float verticalShift = fmax(-1, fmin(1, verticalAngle / maxAngle));
					Position += (-verticalShift * (HEIGHT / 24) * Vector3f(0, 1, 0));

				}
				Touch();
			} else {
				Position = Position.Lerp(Vector3f::ZERO, Time::Delta * 10);
				Touch();
			}
		}
	}

	void Page::UnloadTexture() {
		if(_TextureLoaded) {
			glDeleteTextures(1, &_Texture.texture);
			_TextureLoaded = false;
		}
	}

	void Page::LoadTexture() {
		if(!_TextureLoaded) {
			_Texture = LoadRGBATextureFromMemory(Buffer, _BufferWidth, _BufferHeight, true);
			//_Texture = LoadTextureFromBuffer( _Path.ToCStr(), Buffer, 
			//				TextureFlags_t( /*TEXTUREFLAG_NO_MIPMAPS */TEXTUREFLAG_NO_DEFAULT ), _RealWidth, _RealHeight );

			glBindTexture( _Texture.target, _Texture.texture );
			glGenerateMipmap( _Texture.target );
			glTexParameteri( _Texture.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

			_TextureLoaded = true;
		}
	}


	void *LocalPage::LoadFile(Thread *thread, void *v) {
		LocalPage *page = (LocalPage *)v;

		MemBufferFile bufferFile = MemBufferFile( page->GetPath().ToCStr() );
		MemBuffer fileBuffer = bufferFile.ToMemBuffer();
		
		page->ConsumeBuffer((unsigned char*)(fileBuffer.Buffer), fileBuffer.Length);

		fileBuffer.FreeData();
		bufferFile.FreeData();

		return NULL;
	}

	void Page::ConsumeBuffer(unsigned char* buffer, int length) {
		int comp;
		Buffer = stbi_load_from_memory(buffer, length, &(_RealWidth), &(_RealHeight), &comp, 4);
		
		if(Buffer != NULL) {
			WARN("BUFFER COULD BE LOADED: %d/%d", _RealWidth, _RealHeight);
			if(_RealWidth > 1500) {
				unsigned char *oldBuffer = Buffer;
				//Buffer = ScaleImageRGBA(Buffer, _RealWidth, _RealHeight, 1024, 1024, ImageFilter::IMAGE_FILTER_CUBIC, true);
				Buffer = QuarterImageSize(Buffer, _RealWidth, _RealHeight, false);

				_BufferWidth = OVR::Alg::Max( 1, _RealWidth >> 1 );
				_BufferHeight = OVR::Alg::Max( 1, _RealHeight >> 1 );
			/*	_BufferWidth = 1024;
				_BufferHeight = 1024;*/

				free(oldBuffer);
			} else {
				_BufferWidth = _RealWidth;
				_BufferHeight = _RealHeight;
			}
		} else {
		}
	}

	void Page::Load() {
		if(this->_LoadState == LoadState::UNLOADED) {
			LOG("LOADING %s", _Path.ToCStr());
			// Only if needed...
			this->_LoadState = LoadState::LOADING;

			_LoadThread = Thread( Thread::CreateParams( GetWorker(), this ) );
			_LoadThread.Start();
		}

		if(this->_LoadState == LoadState::LOADING) {
			if(_LoadThread.IsFinished()) {
				WARN("FINISHED!!! %s", _Path.ToCStr());

				LoadTexture();

				// Calculate real width
				_Width = REFERENCE_HEIGHT / _RealHeight * _RealWidth;
				_AngularWidth  = _Width / PIXELS_PER_DEGREE;

				if(_Next != NULL) {
					_Next->SetOffset(_Offset + _Width);
				}

				LOG("LOADED %s", _Path.ToCStr());
				CreateMesh();

				this->_LoadState = LoadState::LOADED;
			}
		}
	}


	Thread::ThreadFn Page::GetWorker() {
		_Texture = LoadTextureFromApplicationPackage(_Path.ToCStr(), TextureFlags_t( TEXTUREFLAG_NO_DEFAULT ), _RealWidth, _RealHeight);
		_TextureLoaded = true;
		return NULL;
	}

	Thread::ThreadFn LocalPage::GetWorker() {
		return &LocalPage::LoadFile;
	}



	void Page::SetOffset(int offset) {
		_Offset = offset;
		_AngularOffset = _Offset / PIXELS_PER_DEGREE;
		_Positionable = true;
	}

	void Page::Draw(const Matrix4f &m) {
		if(this->_DisplayState == DisplayState::VISIBLE && this->_LoadState == LoadState::LOADED) {
			this->UpdateModel();

			//LOG("DRAW %s", _Path.ToCStr());

			// Draw
			//glUniform1i(glGetUniformLocation( prog.program, "IsSelected" ), this->_Selected);
			glUniformMatrix4fv( _Prog.uModel, 1, GL_TRUE, (m * Mat).M[0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(_Texture.target, _Texture.texture);
			_Geometry.Draw();
			glBindVertexArray( 0 );
			glBindTexture(_Texture.target, 0);
		}
	}

	void Page::SetNext(Page *next) {
		_Next = next;
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
		
		for ( int i = 0; i < Page::SEGMENTS; i++ ) {
			float progress =  (i / float(Page::SEGMENTS - 1));
			float x = cos( progress * widthInRadians + radianOffset ) * RADIUS;
			float z = -sin( progress * widthInRadians + radianOffset) * RADIUS;

			attribs.position[i * 2] = Vector3f(x, y0, z);
			attribs.position[i * 2 + 1] = Vector3f(x, y1, z);
			LOG("V1: (%.2f, %.2f, %.2f)", x, y0, z);
			LOG("V2: (%.2f, %.2f, %.2f)", x, y1, z);

			attribs.uv0[i * 2] = Vector2f(1 - progress, 1);
			attribs.uv0[i * 2 + 1] = Vector2f(1 - progress, 0);

			if(i > 0) {
				// Add index
				// T1
                indices[index++] = (i * 2 - 1);
                indices[index++] = (i * 2 + 1);
                indices[index++] = (i * 2 - 2);

                // T2
                indices[index++] = (i * 2 + 1);
                indices[index++] = (i * 2);
                indices[index++] = (i * 2 - 2);
			}
		}


		_Geometry.Create(attribs, indices);

		LOG("CREATED MESH %s (w=%d)", _Path.ToCStr(), _RealWidth);
	}
}
