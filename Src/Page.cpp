#include "Page.h"
#include <math.h>
#include "PackageFiles.h"
#include "Kernel/OVR_MemBuffer.h"
#include "stb_image.h"
#include "ImageData.h"

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
	
	void Page::UpdateStates(float angle) {
		_DisplayState = DisplayState::INVISIBLE;
		bool startedSelected = _Selected;
		_Selected = false;

		float pixelStart = angle * PIXELS_PER_DEGREE - 60 * PIXELS_PER_DEGREE;
		float pixelEnd   = angle * PIXELS_PER_DEGREE + 60 * PIXELS_PER_DEGREE;

		if(_Positionable) {
			int right = (_Offset + _Width);
			if((_Offset > pixelStart && _Offset < pixelEnd) || (_LoadState == LoadState::LOADED && right > pixelStart && right < pixelEnd)) {
				_DisplayState = DisplayState::VISIBLE;

				if(_LoadState == LoadState::LOADED) {
					LoadTexture();

					float degreeStart = _Offset / PIXELS_PER_DEGREE;
					float degreeEnd = degreeStart + _Width / PIXELS_PER_DEGREE;

					if(angle > degreeStart && angle < degreeEnd) {
						_Selected = true;
						if(!startedSelected) {
							_SelectionStart = vrapi_GetTimeInSeconds();
						}
					}
				}

				// Load if unloaded
				Load();
			} else {
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

	void Page::Update(float angle) {
		UpdateStates(angle);

		if(_Selected) {
			double selectionTime = vrapi_GetTimeInSeconds() - _SelectionStart;

			if(selectionTime > 2) {
				float radianOffset = Mathf::Pi / 2;// - widthInRadians / 2; // Makes sure this thing is centered
				radianOffset += DegreeToRad(_Offset / PIXELS_PER_DEGREE);
				radianOffset += DegreeToRad(_Width / PIXELS_PER_DEGREE) / 2.0f;

				float x = cos(radianOffset) * RADIUS;
				float z = -sin(radianOffset) * RADIUS;
				Vector3f dir = Vector3f(-x, 0.0f,-z);

				float progress = fmin(1, fmax(0, ((selectionTime - 2.0f) / 2.0f)));

				Position = dir * progress * 0.2f;
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

	void *DownloadImage(Thread *thread, void *v) {
		//Page *page = (Page *)v;
		
		return NULL;
	}

	void *FillBuffer(Thread *thread, void *v) {
		Page *page = (Page *)v;

		MemBufferFile bufferFile = MemBufferFile( page->GetPath().ToCStr() );
		MemBuffer fileBuffer = bufferFile.ToMemBuffer();
		
		int comp;
		page->Buffer = stbi_load_from_memory((unsigned char*)(fileBuffer.Buffer), fileBuffer.Length, &(page->_RealWidth), &(page->_RealHeight), &comp, 4);
		
		if(page->Buffer != NULL) {
			WARN("BUFFER COULD BE LOADED: %d/%d", page->_RealWidth, page->_RealHeight);
			if(page->_RealWidth > 1500) {
				unsigned char *oldBuffer = page->Buffer;
				//page->Buffer = ScaleImageRGBA(page->Buffer, page->_RealWidth, page->_RealHeight, 1024, 1024, ImageFilter::IMAGE_FILTER_CUBIC, true);
				page->Buffer = QuarterImageSize(page->Buffer, page->_RealWidth, page->_RealHeight, false);

				page->_BufferWidth = OVR::Alg::Max( 1, page->_RealWidth >> 1 );
				page->_BufferHeight = OVR::Alg::Max( 1, page->_RealHeight >> 1 );
			/*	page->_BufferWidth = 1024;
				page->_BufferHeight = 1024;*/

				free(oldBuffer);
			} else {
				page->_BufferWidth = page->_RealWidth;
				page->_BufferHeight = page->_RealHeight;
			}
		} else {
		}
		
		fileBuffer.FreeData();
		bufferFile.FreeData();

		return NULL;
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
		return &FillBuffer;
	}



	void Page::SetOffset(int offset) {
		_Offset = offset;
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

		float widthInRadians = DegreeToRad(_Width / PIXELS_PER_DEGREE);
		float radianOffset = Mathf::Pi / 2;// - widthInRadians / 2; // Makes sure this thing is centered
		radianOffset += DegreeToRad(_Offset / PIXELS_PER_DEGREE);

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


	Thread::ThreadFn RemotePage::GetWorker() {
		return &DownloadImage;
	}
}
