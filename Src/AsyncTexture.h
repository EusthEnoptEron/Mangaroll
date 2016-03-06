#pragma once

#include "Kernel\OVR_String.h"
#include "Kernel\OVR_GlUtils.h"
#include "Kernel\OVR_Threads.h"
#include "Kernel/OVR_Types.h"
#include "GazeCursor.h"
#include "Kernel\OVR_Array.h"
#include "MessageQueue.h"
#include "Kernel\OVR_StringHash.h"

using namespace OVR;

namespace OvrMangaroll {

	enum eTextureState {
		TEXTURE_UNLOADED,
		TEXTURE_LOADING,
		TEXTURE_LOADED,
		TEXTURE_APPLYING,
		TEXTURE_APPLIED
	};

	enum eThreadEvents {
		BUFFER_FILLED = 1 << 0,
		TEXTURE_UPLOADED = 1 << 1
	};

	typedef void(*QueueCb)(void*);


	class AsyncTexture {
	public:
		AsyncTexture(String path, int mipmapCount = 1);
		virtual ~AsyncTexture();
		void Load();
		GLuint Display();
		void Hide();
		void Unload();
		void Update();
		eTextureState GetState() { return _State; }
		eTextureState GetFutureState() { return _TargetState; }
		int GetWidth() { return _Width; }
		int GetHeight() { return _Height; }
		GLenum GetTarget() { return GL_TEXTURE_2D; }
		int MaxHeight;
	private:
		void GenerateTexture();
		void DeleteTexture();
		void Unloaded2Loaded();
		void Loaded2Displayed();
		void Displayed2Loaded();
		void Loaded2Unloaded();


		// # THREADS

		// Load file from the Internet
		static void DownloadFile(void *v);

		// Load file from the file system
		static void LoadFile(void *v);

		// Upload file to the graphics card
		static void UploadTexture(void *v);

		// Prepare a buffer for upload to the graphics card
		void ConsumeBuffer(unsigned char *buffer, int length);

		// Texture ID
		GLuint _TID;
		// Buffer ID
		GLuint _BID;

		String _Path;
		int _MipmapCount;
		eTextureState _State;
		eTextureState _TargetState;
		int _Width;
		int _Height;
		int _InternalWidth;
		int _InternalHeight;
		Array<unsigned char*> _Buffers;
		Array<int> _BufferOffsets;
		Array<int> _BufferLengths;

		int _BufferLength;
		void *_GPUBuffer;
		bool _TextureGenerated;
		int _ThreadEvents;

		static ovrMessageQueue *S_Queue;
		static Thread *S_WorkerThread;
		static void *S_WorkerFn(Thread *, void *);
	};

	class BufferManager {
	public:
		static BufferManager &Instance() {
			if (S_Instance == NULL) {
				S_Instance = new BufferManager();
			}
			return *S_Instance;
		}

		GLuint GetBuffer();
		GLuint GetTexture(int width, int height, int mipCount);
		void ReleaseBuffer(GLuint);
		void ReleaseTexture(GLuint, int width, int height, int mipCount);
	private:
		BufferManager();
		BufferManager(const BufferManager &);

		static BufferManager *S_Instance;
		static Array<GLuint> *S_Buffers;
		static Hash<String, Array<GLuint>> *S_Textures;
		static GLuint *S_Buffers_Arr;
	};

}