#pragma once

#include "Kernel\OVR_String.h"
#include "Kernel\OVR_GlUtils.h"
#include "Kernel\OVR_Threads.h"
#include "Kernel/OVR_Types.h"
#include "GazeCursor.h"

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

	class AsyncTexture {
	public:
		AsyncTexture(String path, int mipmapCount = 1);
		virtual ~AsyncTexture();
		void Load();
		GLuint Display();
		void Hide();
		void Unload();
		void Update();
		eTextureState GetState();

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
		static void *DownloadFile(Thread *thread, void *v);

		// Load file from the file system
		static void *LoadFile(Thread *thread, void *v);

		// Upload file to the graphics card
		static void *UploadTexture(Thread *thread, void *v);

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

		Thread *_LoadThread;
		Thread *_UploadThread;
	};

}