#pragma once

#include "App.h"
#include "GlObject.h"
#include "ShaderManager.h"
#include "AsyncTexture.h"

using namespace OVR;
namespace OvrMangaroll {

	const float PIXELS_PER_DEGREE = 20.0f;
	const float REFERENCE_HEIGHT = 1000.0f;

	enum DisplayState { VISIBLE, INVISIBLE, LIMBO };
	enum PlacingOrigin { PLACING_NONE, PLACING_BOTTOM, PLACING_TOP };

	class Page : public GlObject {
	public:
		Page(String _path) :
			GlObject(),
			_Path(_path),
			_Offset(0),
			_DisplayState(INVISIBLE),
			_Next(NULL),
			_Prev(NULL),
			_Geometry(),
			_Width(0),
			_Positionable(false),
			_Selected(false),
			_Model(ovrMatrix4f_CreateIdentity()),
			_HighOffset(0),
			_Origin(PLACING_NONE),
			_DisplayTime(0),
			_Progs(),
			_uDisplayTime(),
			_Loaded(false),
			_ATexture(NULL)
		{
			_Progs[0] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
			_Progs[1] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME, PAGE_TRANSPARENT_FRAG_NAME);

			// v----- defunct
			_uDisplayTime[0] = glGetUniformLocation(_Progs[0]->program, "DisplayTime");
			_uDisplayTime[1] = glGetUniformLocation(_Progs[1]->program, "DisplayTime");

			WARN("UNIFORM, DISPLAYTIME: %d (%d [%p])", _uDisplayTime[1], _Progs[1]->program, _Progs[1]);
			_ATexture = new AsyncTexture(_path, 3);
			_ATexture->MaxHeight = 1500;
		};

		virtual ~Page(void);

		static const int SEGMENTS;
		static const float HEIGHT;
		static const float RADIUS;
		void Update(float angle, bool onlyVisual = false);
		void SetNext(Page *next);
		void SetPrev(Page *prev);
		Page *GetNext(void);
		void Load(void);
		void Draw(const Matrix4f &m);
		void SetOffset(int offset);
		void SetHighOffset(int offset);
		String GetPath() { return _Path; }
		bool IsVisible();
		bool IsLoaded();
		bool IsTarget(float angle);
		void SetSelected(bool state);
		void Reset(void);
		bool IsValid() { return _ATexture->IsValid(); }
	protected:
		String _Path;
		int _Offset;
		DisplayState _DisplayState;
		Page *_Next;
		Page *_Prev;
		GlGeometry _Geometry;
		int _Width;
		bool _Positionable;
		bool _Selected;
		double _SelectionStart;
		Matrix4f _Model;
		float _AngularOffset;
		float _AngularWidth;
		float _ChordLength;
		int _HighOffset;
		PlacingOrigin _Origin;
		float _DisplayTime;
	private:
		void UpdateStates(float);

		GlProgram *_Progs[2];
		int _uDisplayTime[2];

		bool _Loaded;
	protected:

		void CreateMesh(void);
		AsyncTexture *_ATexture;
	};


	class LocalPage : public Page {
	public:
		LocalPage(String path) : Page(path) {}
	};

}
