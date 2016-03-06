#include "AsyncTexture.h"
#include <stdio.h>
#include "Kernel\OVR_MemBuffer.h"
#include "stb_image.h"
#include "ImageData.h"
#include "Kernel\OVR_LogUtils.h"
#include <OVR_Capture.h>
#include "Kernel\OVR_String_Utils.h"

using namespace OVR;

namespace OvrMangaroll {
	//typedef unsigned int GLuint;

	PFNGLMAPBUFFEROESPROC glMapBuffer = (PFNGLMAPBUFFEROESPROC)eglGetProcAddress("glMapBufferOES");

	void *AsyncTexture::S_WorkerFn(Thread *thread, void *) {
		thread->SetThreadName("Worker Thread");
		
		// Create buffers
		
		while (!thread->GetExitFlag()) {
			S_Queue->SleepUntilMessage();
			const char *msg = S_Queue->GetNextMessage();
			
			QueueCb cb;
			void *obj;
			sscanf(msg, "call %p %p", &cb, &obj);

			cb(obj);
		}
		return NULL;
	}


	ovrMessageQueue *AsyncTexture::S_Queue = new ovrMessageQueue(20);
	Thread *AsyncTexture::S_WorkerThread = new Thread(Thread::CreateParams(AsyncTexture::S_WorkerFn, NULL, 128 * 1024, -1, Thread::ThreadState::Running, Thread::BelowNormalPriority));
	
	Array<GLuint> *BufferManager::S_Buffers = NULL;
	Hash<String, Array<GLuint>> *BufferManager::S_Textures = NULL;
	GLuint *BufferManager::S_Buffers_Arr = NULL;
	BufferManager *BufferManager::S_Instance = NULL;

	BufferManager::BufferManager() {
		S_Buffers = new Array<GLuint>();
		S_Textures = new Hash<String, Array<GLuint>>();

		S_Buffers_Arr = new GLuint[5];
		// Create buffers
		glGenBuffers(5, S_Buffers_Arr);
		int bufferLength = 2000 * 4000 * 4;
		for (int i = 0; i < 5; i++) {
			GLuint buffer = S_Buffers_Arr[i];
			S_Buffers->PushBack(buffer);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferLength, NULL, GL_DYNAMIC_DRAW);
		}
	}

	GLuint BufferManager::GetBuffer() {
		return S_Buffers->Pop();
	}
	int COUNTER = 0;
	GLuint BufferManager::GetTexture(int width, int height, int mipCount) {
		String key = String::Format("%d-%d@%d", width, height, mipCount);
		Array<GLuint> *arr = S_Textures->Get(key);
		if (arr == NULL) {
			arr = new Array<GLuint>();
			S_Textures->Set(key, *arr);
		}
		
		if (arr->GetSizeI() == 0) {
			// Drained...
			WARN("CREATE TEXTURE");
			GLuint buf = GetBuffer();
			GLuint *tex = new GLuint();

			glGenTextures(1, tex);
			glBindTexture(GL_TEXTURE_2D, *tex);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buf);


			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipCount - 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			for (int i = 0; i < mipCount; i++) {
				int w = Alg::Max(1, width >> i);
				int h = Alg::Max(1, height >> i);
				glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}

			ReleaseBuffer(buf);
			WARN("CREATED TEXTURE %d", COUNTER++);
			return *tex;
		}
		else {
			return arr->Pop();
		}
	}

	void BufferManager::ReleaseBuffer(GLuint buffer) {
		S_Buffers->PushBack(buffer);
	}

	void BufferManager::ReleaseTexture(GLuint tex, int width, int height, int mipCount) {
		String key = String::Format("%d-%d@%d", width, height, mipCount);

		S_Textures->Get(key)->PushBack(tex);
	}




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
			OVR_CAPTURE_CPU_ZONE(Texture_Upload);
			WARN("Finished writing to PBO!");
			// Unmap the buffer
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _BID);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			glBindTexture(GL_TEXTURE_2D, _TID);

			// Unpack
			int mipmapWidth = _InternalWidth;
			int mipmapHeight = _InternalHeight;
			
			WARN("START CREATION...");
			for (int i = 0; i < _MipmapCount; i++) {
				WARN("MAKE MIP %d", i);
				glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, mipmapWidth, mipmapHeight, GL_RGBA, GL_UNSIGNED_BYTE, (void *)_BufferOffsets[i]);
				mipmapWidth = Alg::Max(1, mipmapWidth >> 1);
				mipmapHeight = Alg::Max(1, mipmapHeight >> 1);
			}
			WARN("END CREATION...");


			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			BufferManager::Instance().ReleaseBuffer(_BID);

			_State = TEXTURE_APPLIED;
			WARN("APPLIED!");

		}

		_ThreadEvents = 0;
	}

	void AsyncTexture::Unloaded2Loaded() {
		WARN("%s UNLOADED2LOADED", _Path.ToCStr());
		_State = TEXTURE_LOADING;

		// If local / remote...
		S_Queue->PostPrintf("call %p %p", LoadFile, this);
	}

	void AsyncTexture::Loaded2Displayed() {
		WARN("%s Loaded2Displayed", _Path.ToCStr());
		OVR_CAPTURE_CPU_ZONE(CreateBuffer);

		_State = TEXTURE_APPLYING;

		// Make buffers
		_BID = BufferManager::Instance().GetBuffer();

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _BID);

		// Map buffer
		_GPUBuffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY_OES);

		// Get outta this context
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		WARN("%s CALLING THREAD, Dude!", _Path.ToCStr());

		S_Queue->PostPrintf("call %p %p", UploadTexture, this);
	}

	void AsyncTexture::Displayed2Loaded() {
		WARN("%s Displayed2Loaded", _Path.ToCStr());

		_State = TEXTURE_LOADED;
		DeleteTexture();
	}

	void AsyncTexture::Loaded2Unloaded() {
		_State = TEXTURE_UNLOADED;
		WARN("%s Displayed2LoadedLoaded2Unloaded", _Path.ToCStr());


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
		if (_TextureGenerated || _State < TEXTURE_LOADED)
			return;
		WARN("%s GENERATE", _Path.ToCStr());

		// Create texture

	/*	glGenTextures(1, &_TID); */
		_TID = BufferManager::Instance().GetTexture(_InternalWidth, _InternalHeight, _MipmapCount);
		
		WARN("GOT MY TEXTURE");
		_TextureGenerated = true;
	}

	void AsyncTexture::DeleteTexture() {
		if (!_TextureGenerated)
			return;

		WARN("%s DELETE", _Path.ToCStr());

		BufferManager::Instance().ReleaseTexture(_TID, _InternalWidth, _InternalHeight, _MipmapCount);
		//glDeleteTextures(1, &_TID);
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
	void AsyncTexture::LoadFile(void *v) {

		AsyncTexture *tex = (AsyncTexture *)v;
		WARN("%s LOAD FILE", tex->_Path.ToCStr());

		MemBufferFile bufferFile = MemBufferFile(tex->_Path.ToCStr());
		MemBuffer fileBuffer = bufferFile.ToMemBuffer();


		tex->ConsumeBuffer((unsigned char*)(fileBuffer.Buffer), fileBuffer.Length);

		fileBuffer.FreeData();
		bufferFile.FreeData();

		tex->_ThreadEvents |= BUFFER_FILLED;
	}

	void AsyncTexture::DownloadFile(void *v) {
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
	}

	void AsyncTexture::UploadTexture(void *v) {
		OVR_CAPTURE_CPU_ZONE(UploadTexture);

		AsyncTexture *tex = (AsyncTexture *)v;
		WARN("%s UPLOAD FILE", tex->_Path.ToCStr());

		for (int i = 0; i < tex->_MipmapCount; i++) {
			memcpy(
				(void *)((unsigned char *)(tex->_GPUBuffer) + tex->_BufferOffsets[i]),
				(const void *)tex->_Buffers[i],
				size_t(tex->_BufferLengths[i])
			);
		}

		tex->_ThreadEvents |= TEXTURE_UPLOADED;
	}

	void AsyncTexture::ConsumeBuffer(unsigned char *buffer, int length) {
		WARN("%s CONSUME", this->_Path.ToCStr());
		OVR_CAPTURE_CPU_ZONE(ConsumeBuffer);

		int comp;
		unsigned char *Buffer = stbi_load_from_memory(buffer, length, &(_Width), &(_Height), &comp, 4);

		_InternalWidth = _Width;
		_InternalHeight = _Height;

		if (Buffer != NULL) {
			while (_InternalHeight > MaxHeight) {
				unsigned char *oldBuffer = Buffer;
				Buffer = ScaleImageRGBA(Buffer, _Width, _Height, MaxHeight, MaxHeight, ImageFilter::IMAGE_FILTER_LINEAR, true);
				_InternalWidth = MaxHeight;
				_InternalHeight = MaxHeight;

				//Buffer = QuarterImageSize(Buffer, _InternalWidth, _InternalHeight, false);
				//_InternalWidth = OVR::Alg::Max(1, _InternalWidth >> 1);
				//_InternalHeight = OVR::Alg::Max(1, _InternalHeight >> 1);

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