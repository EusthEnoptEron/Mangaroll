#ifndef OVRPAGE_H
#define OVRPAGE_H

#include "App.h"

using namespace OVR;
namespace OvrMangaroll {
	const float PIXELS_PER_DEGREE = 20.0f;
	const float REFERENCE_HEIGHT = 1000.0f;

	enum LoadState { UNLOADED, LOADING, LOADED };
	enum DisplayState { VISIBLE, INVISIBLE, LIMBO };

	class Page {
	public:
		Page(String _path) : 
			_Path(_path),
			_Offset(0), 
			_LoadState(UNLOADED), 
			_DisplayState(INVISIBLE), 
			_Next(NULL), 
			_Texture(),
			_Geometry(),
			_Width(0), 
			//_RealWidth(0),
			//_RealHeight(0),
			_Positionable(false),
			_TextureLoaded(false),
			_LoadThread(),
			_Selected(false),
			_Model(ovrMatrix4f_CreateIdentity())
		{
			
		};

		virtual ~Page(void);

		static const int SEGMENTS;
		static const float HEIGHT;
		static const float RADIUS;
		void Update(float angle);
		void SetNext(Page *next);
		Page *GetNext(void);
		void Load(void);
		void Draw(const GlProgram &prog);
		void SetOffset(int offset);
		String GetPath() { return _Path; }
		//MemBuffer Buffer;
		unsigned char* Buffer;
		void LoadTexture();
		int _RealWidth;
		int _RealHeight;
		int _BufferWidth;
		int _BufferHeight;
	protected:
		void UnloadTexture();
		Thread::ThreadFn virtual GetWorker();
		String _Path;
		int _Offset;
		LoadState _LoadState;
		DisplayState _DisplayState;
		Page *_Next;
		GlTexture _Texture;
		GlGeometry _Geometry;
		int _Width;
		
		bool _Positionable;
		bool _TextureLoaded;
		Thread _LoadThread;
		bool _Selected;
		double _SelectionStart;
		Matrix4f _Model;
		//void FillBuffer();
	private:
		
	protected:

		void CreateMesh(void);
	};


	class LocalPage : public Page {
	public:
		LocalPage(String path) : Page(path) {}
	protected:
		Thread::ThreadFn virtual GetWorker();
	};

	class RemotePage : public Page {
	public:
		RemotePage(String path) : Page(path) {}

	protected:
		Thread::ThreadFn virtual GetWorker();
	};
}

#endif