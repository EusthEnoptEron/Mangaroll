#pragma once

#include "App.h"
#include "GlObject.h"
#include "ShaderManager.h"

using namespace OVR;
namespace OvrMangaroll {

	const float PIXELS_PER_DEGREE = 20.0f;
	const float REFERENCE_HEIGHT = 1000.0f;

	enum LoadState { UNLOADED, LOADING, LOADED };
	enum DisplayState { VISIBLE, INVISIBLE, LIMBO };
	enum PlacingOrigin { PLACING_NONE, PLACING_BOTTOM, PLACING_TOP };

	class Page : public GlObject {
	public:
		Page(String _path) : 
			GlObject(),
			_Path(_path),
			_Offset(0), 
			_LoadState(UNLOADED), 
			_DisplayState(INVISIBLE), 
			_Next(NULL), 
			_Prev(NULL),
			_Texture(),
			_Geometry(),
			_Width(0), 
			_RealWidth(0),
			_RealHeight(0),
			_BufferWidth(0),
			_BufferHeight(0),
			_Positionable(false),
			_TextureLoaded(false),
			_LoadThread(),
			_Selected(false),
			_Model(ovrMatrix4f_CreateIdentity()),
			_HighOffset(0),
			_Origin(PLACING_NONE)
			
		{
			_Prog = *ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
		};

		virtual ~Page(void);

		static const int SEGMENTS;
		static const float HEIGHT;
		static const float RADIUS;
		void Update(float angle, bool onlyVisual = false);
		void SetNext(Page *next);
		void SetPrev(Page *prev);
		void SetProgram(GlProgram &prog) { _Prog = prog; }
		Page *GetNext(void);
		void Load(void);
		void Draw(const Matrix4f &m);
		void SetOffset(int offset);
		void SetHighOffset(int offset);
		String GetPath() { return _Path; }
		//MemBuffer Buffer;
		unsigned char* Buffer;
		void LoadTexture();
		bool IsVisible();
		bool IsLoaded();
		bool IsTarget(float angle);
		void SetSelected(bool state);
		void Reset(void);
	protected:
		void UnloadTexture();
		Thread::ThreadFn virtual GetWorker();
		String _Path;
		int _Offset;
		LoadState _LoadState;
		DisplayState _DisplayState;
		Page *_Next;
		Page *_Prev;
		GlTexture _Texture;
		GlGeometry _Geometry;
		int _Width;
		int _RealWidth;
		int _RealHeight;
		int _BufferWidth;
		int _BufferHeight;
		bool _Positionable;
		bool _TextureLoaded;
		Thread _LoadThread;
		bool _Selected;
		double _SelectionStart;
		Matrix4f _Model;
		float _AngularOffset;
		float _AngularWidth;
		int _HighOffset;
		PlacingOrigin _Origin;
		void ConsumeBuffer(unsigned char* buffer, int length);
		//void FillBuffer();
	private:
		void UpdateStates(float);
		
		GlProgram _Prog;
	protected:

		void CreateMesh(void);
	};


	class LocalPage : public Page {
	public:
		LocalPage(String path) : Page(path) {}
	protected:
		Thread::ThreadFn virtual GetWorker();
	private:
		static void *LoadFile(Thread *thread, void *v);
	};

}
