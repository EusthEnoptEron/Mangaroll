#include "AsyncTexture.h"
#include <stdio.h>
#include "Kernel\OVR_MemBuffer.h"
#include "stb_image.h"
#include "ImageData.h"
#include "Kernel\OVR_LogUtils.h"
#include <ctime>
#include <OVR_Capture.h>
#include "Kernel\OVR_String_Utils.h"
#include "Helpers.h"
#include "Web.h"

using namespace OVR;

namespace OvrMangaroll {

	PFNGLMAPBUFFEROESPROC glMapBuffer = (PFNGLMAPBUFFEROESPROC)eglGetProcAddress("glMapBufferOES");

	void *AsyncTexture::S_WorkerFn(Thread *thread, void *) {
		thread->SetThreadName("Worker Thread");
		
		// Create buffers
		
		while (!thread->GetExitFlag()) {
			S_Queue->SleepUntilMessage();
			const char *msg = S_Queue->GetNextMessage();
			
			if (msg != NULL) {
				QueueCb cb;
				void *obj;
				sscanf(msg, "call %p %p", &cb, &obj);

				cb(thread, obj);
			}
		}
		return NULL;
	}


	ovrMessageQueue *AsyncTexture::S_Queue = new ovrMessageQueue(100);
	Thread *AsyncTexture::S_WorkerThread = new Thread(Thread::CreateParams(AsyncTexture::S_WorkerFn, NULL, 128 * 1024, -1, Thread::ThreadState::Running, Thread::BelowNormalPriority));
	Thread *AsyncTexture::S_WorkerThread2 = new Thread(Thread::CreateParams(AsyncTexture::S_WorkerFn, NULL, 128 * 1024, -1, Thread::ThreadState::Running, Thread::BelowNormalPriority));


	// #################### BUFFER MANAGER #######################
	Array<GLuint> *BufferManager::S_Buffers = NULL;
	GLuint *BufferManager::S_Buffers_Arr = NULL;
	BufferManager *BufferManager::S_Instance = NULL;

	BufferManager::BufferManager() {
		S_Buffers = new Array<GLuint>();
		S_Buffers_Arr = new GLuint[5];

#ifdef USE_PBO
		// Create buffers
		glGenBuffers(5, S_Buffers_Arr);
		int bufferLength = 2000 * 4000 * 4;
		for (int i = 0; i < 5; i++) {
			GLuint buffer = S_Buffers_Arr[i];
			S_Buffers->PushBack(buffer);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, bufferLength, NULL, GL_DYNAMIC_DRAW);
		}
#endif
	}

	GLuint BufferManager::GetBuffer() {
		return S_Buffers->Pop();

	}

	void BufferManager::ReleaseBuffer(GLuint buffer) {
		S_Buffers->InsertAt(0, buffer);
	}


	// ###########################################################
	// #################### ASYNC TEXTURE MANAGER ################
	AsyncTextureManager *AsyncTextureManager::S_Instance = NULL;
	Array<AsyncTexture*> *AsyncTextureManager::S_Textures = NULL;

	void AsyncTextureManager::Update() {
		for (int i = S_Textures->GetSizeI() - 1; i >= 0; --i) {
			
			AsyncTexture *texture = S_Textures->At(i);
			texture->Update();
		
			if (texture->GetState() == texture->GetFutureState()) {
				S_Textures->RemoveAt(i);
			}
		}
	}

	void AsyncTextureManager::Register(AsyncTexture *texture) {
		S_Textures->PushBack(texture);
	}

	// ###########################################################


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
		, _Valid(true)
		, _Moot(false)
	{
		if (path.IsEmpty()) {
			_Valid = false;
			_Moot = true;
		}
	}

	AsyncTexture::~AsyncTexture() {
		DeleteTexture();
	}
	void AsyncTexture::SetTarget(eTextureState state) {
		_TargetState = state;
		if (state != _State) {
			AsyncTextureManager::Instance().Register(this);
		}
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

#ifdef USE_PBO
		if (_ThreadEvents & TEXTURE_UPLOADED) {
			// Not currently used
			OVR_CAPTURE_CPU_ZONE(Texture_Upload);
			WARN("Finished writing to PBO!");
			// Unmap the buffer
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _BID);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			glBindTexture(GL_TEXTURE_2D, _TID);

			// Unpack
			int mipmapWidth = _InternalWidth;
			int mipmapHeight = _InternalHeight;
			
			for (int i = 0; i < _MipmapCount; i++) {
				glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, mipmapWidth, mipmapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)_BufferOffsets[i]);
				mipmapWidth = Alg::Max(1, mipmapWidth >> 1);
				mipmapHeight = Alg::Max(1, mipmapHeight >> 1);
			}

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			BufferManager::Instance().ReleaseBuffer(_BID);

			_State = TEXTURE_APPLIED;
			WARN("APPLIED!");

		}
#endif

		_ThreadEvents = 0;
	}

	void AsyncTexture::Unloaded2Loaded() {
		WARN("%s UNLOADED2LOADED", _Path.ToCStr());
		_State = TEXTURE_LOADING;

		// If local / remote...
		if (!_Valid) {
			// Let's not even try
			_State = TEXTURE_LOADED;
		}
		else {
			if (_Path.GetLengthI() > 7 && (_Path.Substring(0, 7).CompareNoCase("http://") == 0 || _Path.Substring(0, 8).CompareNoCase("https://") == 0)) {
				LOG("TEXTURE: %s is a URL", _Path.ToCStr());
				S_Queue->PostPrintf("call %p %p", DownloadFile, this);
			}
			else {
				LOG("TEXTURE: %s is a local resource", _Path.ToCStr());
				S_Queue->PostPrintf("call %p %p", LoadFile, this);
			}
		}
	}

	void AsyncTexture::Loaded2Displayed() {
		WARN("%s Loaded2Displayed", _Path.ToCStr());
		OVR_CAPTURE_CPU_ZONE(CreateBuffer);

		_State = TEXTURE_APPLYING;

		if (_Valid) {
#ifdef USE_PBO
			// Make buffers
			_BID = BufferManager::Instance().GetBuffer();
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _BID);

			// Map buffer
			_GPUBuffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY_OES);

			// Get outta this context
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

			S_Queue->PostPrintf("call %p %p", UploadTexture, this);
#else
			glBindTexture(GL_TEXTURE_2D, _TID);

			// Unpack
			int mipmapWidth = _InternalWidth;
			int mipmapHeight = _InternalHeight;

			for (int i = 0; i < _Buffers.GetSizeI(); i++) {
				glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, mipmapWidth, mipmapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)_Buffers[i]);
				mipmapWidth = Alg::Max(1, mipmapWidth >> 1);
				mipmapHeight = Alg::Max(1, mipmapHeight >> 1);
			}
		}

		_State = TEXTURE_APPLIED;
#endif

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
			SetTarget(TEXTURE_LOADED);
		}
	}

	GLuint AsyncTexture::Display() {
		if (_TargetState < TEXTURE_APPLYING) {
			SetTarget(TEXTURE_APPLIED);
		}

		GenerateTexture();

		return _TID;
	}

	void AsyncTexture::GenerateTexture() {
		if (_TextureGenerated)
			return;
		WARN("%s GENERATE", _Path.ToCStr());

		// Create texture
		glGenTextures(1, &_TID);

		glBindTexture(GL_TEXTURE_2D, _TID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, _MipmapCount-1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		_TextureGenerated = true;
	}

	void AsyncTexture::DeleteTexture() {
		if (!_TextureGenerated)
			return;

		WARN("%s DELETE", _Path.ToCStr());

		glDeleteTextures(1, &_TID);
		_TextureGenerated = false;
	}

	void AsyncTexture::Hide() {
		if (_TargetState >= TEXTURE_APPLYING) {
			SetTarget(TEXTURE_LOADED);
		}
	}

	void AsyncTexture::Unload() {
		if (_TargetState >= TEXTURE_LOADING) {
			SetTarget(TEXTURE_UNLOADED);
		}
	}



	// ------- THREAD FUNCTIONS --------
	void AsyncTexture::LoadFile(Thread *thread, void *v) {

		AsyncTexture *tex = (AsyncTexture *)v;
		WARN("%s LOAD FILE", tex->_Path.ToCStr());

		MemBufferFile bufferFile = MemBufferFile(tex->_Path.ToCStr());
		MemBuffer fileBuffer = bufferFile.ToMemBuffer();


		tex->ConsumeBuffer((unsigned char*)(fileBuffer.Buffer), fileBuffer.Length);

		fileBuffer.FreeData();
		bufferFile.FreeData();

		tex->_ThreadEvents |= BUFFER_FILLED;
	}

	void AsyncTexture::DownloadFile(Thread *thread, void *v) {
		AsyncTexture *tex = (AsyncTexture *)v;

		DownloadMeta *meta = new DownloadMeta();
		meta->callback = DownloadFileCallback;
		meta->target = v;
		meta->url = tex->_Path;
		Web::DownloadFn(thread, meta);
		tex->_ThreadEvents |= BUFFER_FILLED;
	}

	void AsyncTexture::DownloadFileCallback(void *buffer, int length, void *p) {
		AsyncTexture *tex = (AsyncTexture *)p;
		tex->ConsumeBuffer((unsigned char *)buffer, length);
	}

	void AsyncTexture::UploadTexture(Thread *, void *v) {
		OVR_CAPTURE_CPU_ZONE(UploadTexture);

		AsyncTexture *tex = (AsyncTexture *)v;
		WARN("%s UPLOAD FILE", tex->_Path.ToCStr());

		for (int i = 0; i < tex->_Buffers.GetSizeI(); i++) {
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
				/*Buffer = ScaleImageRGBA(Buffer, _Width, _Height, MaxHeight, MaxHeight, ImageFilter::IMAGE_FILTER_LINEAR, true);
				_InternalWidth = MaxHeight;
				_InternalHeight = MaxHeight;*/
				Buffer = QuarterImageSize(Buffer, _InternalWidth, _InternalHeight, false);
				_InternalWidth = OVR::Alg::Max(1, _InternalWidth >> 1);
				_InternalHeight = OVR::Alg::Max(1, _InternalHeight >> 1);

				free(oldBuffer);
			}
		}
		else {
			_Valid = false;
		}

		_Buffers.PushBack(Buffer);
		_BufferLength = _InternalWidth * _InternalHeight * 4; // RGBA
		_BufferOffsets.PushBack(0);
		_BufferLengths.PushBack(_BufferLength);

		// Create mipmaps
		if (_MipmapCount > 1 && Buffer != NULL && length > 0) {
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