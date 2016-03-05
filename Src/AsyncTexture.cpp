#include "AsyncTexture.h"
#include <stdio.h>
#include "Kernel\OVR_MemBuffer.h"
#include "stb_image.h"
#include "ImageData.h"
#include "Kernel\OVR_LogUtils.h"

namespace OvrMangaroll {
	PFNGLMAPBUFFEROESPROC glMapBuffer = (PFNGLMAPBUFFEROESPROC)eglGetProcAddress("glMapBufferOES");

	AsyncTexture::AsyncTexture(String path, int mipmapCount)
		: MaxHeight(2000)
		, _TID(0)
		, _BID(0)
		, _Path(path)
		, _MipmapCount(mipmapCount)
		, _State(TEXTURE_UNLOADED)
		, _TargetState(TEXTURE_UNLOADED)
		, _Width(0)
		, _Height(0)
		, _InternalWidth(0)
		, _InternalHeight(0)
		, _Buffers()
		, _BufferOffsets()
		, _BufferLengths()
		, _BufferLength(0)
		, _GPUBuffer(NULL)
		, _TextureGenerated(false)
		, _ThreadEvents(0)
		, _LoadThread(NULL)
		, _UploadThread(NULL)
	{
	}

	AsyncTexture::~AsyncTexture() {
		DeleteTexture();
	}

	void AsyncTexture::Update() {
		// Generate texture as fast as possible if needed
		if (_TargetState >= TEXTURE_APPLYING) {
			GenerateTexture();
		}

		// ------ SIMPLISTIC STATE MACHINE ------

		switch (_State) {
		case TEXTURE_UNLOADED:
			if (_TargetState >= TEXTURE_LOADING) {
				Unloaded2Loaded();
			}
			break;
		case TEXTURE_LOADED:
			if (_TargetState == TEXTURE_UNLOADED) {
				Loaded2Unloaded();
			}
			else if (_TargetState == TEXTURE_APPLIED) {
				Loaded2Displayed();
			}
			break;
		case TEXTURE_APPLIED:
			if (_TargetState <= TEXTURE_LOADED) {
				Displayed2Loaded();
			}
			break;
		default:
			break;
		}
		// ---------------------------
		// Handle events
		if (_ThreadEvents & BUFFER_FILLED) {
			_State = TEXTURE_LOADED;
		}

		if (_ThreadEvents & TEXTURE_UPLOADED) {
			WARN("Finished writing to PBO!");
			// Unmap the buffer
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _BID);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

			// Unpack
			int mipmapWidth = _InternalWidth;
			int mipmapHeight = _InternalHeight;
			for (int i = 0; i < _MipmapCount; i++) {
				glBindTexture(GL_TEXTURE_2D, _TID);
				glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, _InternalWidth, _InternalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)_BufferOffsets[i]);
				mipmapWidth = Alg::Max(1, mipmapWidth >> 1);
				mipmapHeight = Alg::Max(1, mipmapHeight >> 1);
			}
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glDeleteBuffers(1, &_BID);

			_State = TEXTURE_APPLIED;
			WARN("APPLIED!");

		}

		_ThreadEvents = 0;
	}

	void AsyncTexture::Unloaded2Loaded() {
		WARN("UNLOADED2LOADED");
		_State = TEXTURE_LOADING;

		// If local / remote...
		_LoadThread = new Thread(Thread::CreateParams(LoadFile, this));
		_LoadThread->Start();
	}

	void AsyncTexture::Loaded2Displayed() {
		WARN("Loaded2Displayed");

		_State = TEXTURE_APPLYING;

		// Make buffers
		glGenBuffers(1, &_BID);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _BID);

		// Resize
		glBufferData(GL_PIXEL_UNPACK_BUFFER, _BufferLength, NULL, GL_STATIC_DRAW);

		// Map buffer
		_GPUBuffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY_OES);

		// Get outta this context
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		_UploadThread = new Thread(Thread::CreateParams(UploadTexture, this));
		_UploadThread->Start();
	}

	void AsyncTexture::Displayed2Loaded() {
		WARN("Displayed2Loaded");

		_State = TEXTURE_LOADED;
		DeleteTexture();
	}

	void AsyncTexture::Loaded2Unloaded() {
		_State = TEXTURE_UNLOADED;
		WARN("Displayed2LoadedLoaded2Unloaded");


		while (_Buffers.GetSizeI() > 0) {
			free(_Buffers.Pop());
		}
		_Buffers = Array<unsigned char*>();
		_BufferOffsets.Clear();
		_BufferLengths.Clear();
	}

	void AsyncTexture::Load() {
		if (_TargetState < TEXTURE_LOADING) {
			_TargetState = TEXTURE_LOADED;
		}
	}

	GLuint AsyncTexture::Display() {
		if (_TargetState < TEXTURE_APPLYING) {
			_TargetState = TEXTURE_APPLIED;
		}

		GenerateTexture();

		return _TID;
	}

	void AsyncTexture::GenerateTexture() {
		if (_TextureGenerated)
			return;

		// Create texture
		glGenTextures(1, &_TID);

		glBindTexture(GL_TEXTURE_2D, _TID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		_TextureGenerated = true;
	}

	void AsyncTexture::DeleteTexture() {
		if (!_TextureGenerated)
			return;

		glDeleteTextures(1, &_TID);
		_TextureGenerated = false;
	}

	void AsyncTexture::Hide() {
		if (_TargetState >= TEXTURE_APPLYING) {
			_TargetState = TEXTURE_LOADED;
		}
	}

	void AsyncTexture::Unload() {
		if (_TargetState >= TEXTURE_LOADING) {
			_TargetState = TEXTURE_UNLOADED;
		}
	}



	// ------- THREAD FUNCTIONS --------
	void *AsyncTexture::LoadFile(Thread *thread, void *v) {
		AsyncTexture *tex = (AsyncTexture *)v;

		MemBufferFile bufferFile = MemBufferFile(tex->_Path.ToCStr());
		MemBuffer fileBuffer = bufferFile.ToMemBuffer();

		tex->ConsumeBuffer((unsigned char*)(fileBuffer.Buffer), fileBuffer.Length);

		fileBuffer.FreeData();
		bufferFile.FreeData();

		tex->_ThreadEvents |= BUFFER_FILLED;
		return NULL;
	}

	void *AsyncTexture::DownloadFile(Thread *thread, void *v) {
		AsyncTexture *tex = (AsyncTexture *)v;

		// JNIEnv *env;
		// ovr_AttachCurrentThread(page->_Java->Vm, &env, NULL);

		// page->_Clazz = env->GetObjectClass(page->_Java->ActivityObject);
		// page->_LoadHttpUrl = env->GetStaticMethodID( page->_Clazz, "LoadHttpUrl", "(Ljava/lang/String;)[B");		


		// // Download stuff
		// jstring jstr = env->NewStringUTF( page->_Path.ToCStr() );

		// jbyteArray arr = (jbyteArray) env->CallStaticObjectMethod(page->_Clazz, page->_LoadHttpUrl, jstr);

		// int count = env->GetArrayLength(arr);

		// // Copy array
		// void *buffer = env->GetPrimitiveArrayCritical(arr, 0);
		// page->ConsumeBuffer((unsigned char*)buffer, count);

		// // Clean up
		// env->ReleasePrimitiveArrayCritical(arr, buffer, 0);
		// env->DeleteLocalRef(jstr);
		// env->DeleteLocalRef(arr);

		// ovr_DetachCurrentThread(page->_Java->Vm);

		tex->_ThreadEvents |= BUFFER_FILLED;
		return NULL;
	}

	void *AsyncTexture::UploadTexture(Thread *thread, void *v) {
		AsyncTexture *tex = (AsyncTexture *)v;

		for (int i = 0; i < tex->_MipmapCount; i++) {
			memcpy(
				(void *)((unsigned char *)(tex->_GPUBuffer) + tex->_BufferOffsets[i]),
				(const void *)tex->_Buffers[i],
				size_t(tex->_BufferLengths[i])
			);
		}

		tex->_ThreadEvents |= TEXTURE_UPLOADED;
		return NULL;
	}

	void AsyncTexture::ConsumeBuffer(unsigned char *buffer, int length) {
		int comp;
		unsigned char *Buffer = stbi_load_from_memory(buffer, length, &(_Width), &(_Height), &comp, 4);

		_InternalWidth = _Width;
		_InternalHeight = _Height;

		if (Buffer != NULL) {
			while (_InternalHeight > MaxHeight) {
				unsigned char *oldBuffer = Buffer;
				//Buffer = ScaleImageRGBA(Buffer, _RealWidth, _RealHeight, 1024, 1024, ImageFilter::IMAGE_FILTER_CUBIC, true);
				Buffer = QuarterImageSize(Buffer, _InternalWidth, _InternalHeight, false);

				_InternalWidth = OVR::Alg::Max(1, _InternalWidth >> 1);
				_InternalHeight = OVR::Alg::Max(1, _InternalHeight >> 1);
				free(oldBuffer);
			}
		}

		_Buffers.PushBack(Buffer);
		_BufferLength = _InternalWidth * _InternalHeight * 4; // RGBA
		_BufferOffsets.PushBack(0);
		_BufferLengths.PushBack(_BufferLength);

		// Create mipmaps
		if (_MipmapCount > 1) {
			int mipmapWidth = _InternalWidth;
			int mipmapHeight = _InternalHeight;

			for (int i = 1; i < _MipmapCount; i++) {
				_Buffers.PushBack(
					QuarterImageSize(_Buffers[_Buffers.GetSizeI() - 1],
					mipmapWidth, mipmapHeight, false)
					);

				mipmapWidth = Alg::Max(1, mipmapWidth >> 1);
				mipmapHeight = Alg::Max(1, mipmapHeight >> 1);

				int length = mipmapWidth * mipmapHeight * 4;
				_BufferLengths.PushBack(length);
				_BufferOffsets.PushBack(_BufferLength);

				_BufferLength += length;
			}
		}
	}
}