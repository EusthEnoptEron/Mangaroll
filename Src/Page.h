#pragma once

#include "App.h"
#include "GlObject.h"
#include "ShaderManager.h"
#include "AsyncTexture.h"

using namespace OVR;
namespace OvrMangaroll {

	const float PIXELS_PER_DEGREE = 20.0f;
	const float REFERENCE_HEIGHT = 1000.0f;
	const float REFERENCE_MAX_WIDTH = 180.0f * PIXELS_PER_DEGREE;
	const float REFERENCE_ANGLE_WIDTH = 15.0f;
	const float PAGE_LOOK_SENSITIVIY = 0.5f;
	enum DisplayState { VISIBLE, INVISIBLE, LIMBO };
	enum PlacingOrigin { PLACING_NONE, PLACING_BOTTOM, PLACING_TOP };

	const float RANGE_UNDEFINED = FLT_MIN;

	class Range {
	public:
		Range() : _Start(RANGE_UNDEFINED), _End(RANGE_UNDEFINED), _Length(RANGE_UNDEFINED) {}

		Range(float start) : _Start(start), _End(RANGE_UNDEFINED), _Length(RANGE_UNDEFINED) {
		}
		Range(float start, float end) : _Start(start), _End(end), _Length(RANGE_UNDEFINED) {
			Calculate();
		}

		float GetStart() { return _Start; }
		float GetEnd() { return _End; }
		float GetLength() { return _Length; }

		void SetStart(float val) { _Start = val; Calculate(); }
		void SetEnd(float val) { _End = val; Calculate(); }
		void SetLength(float length) {
			_Length = length;
			Calculate();
		}

		void Reset() { _Start = RANGE_UNDEFINED; _End = RANGE_UNDEFINED; _Length = RANGE_UNDEFINED; }
	private:
		void Calculate() { 
			if (_Start != RANGE_UNDEFINED && _End != RANGE_UNDEFINED) {
				_Length = _End - _Start;
			}
			else if (_Length != RANGE_UNDEFINED && _Start != RANGE_UNDEFINED && _End == RANGE_UNDEFINED) {
				_End = _Start + _Length;
			}
			else if (_Length != RANGE_UNDEFINED && _Start == RANGE_UNDEFINED && _End != RANGE_UNDEFINED) {
				_Start = _End - _Length;
			}
		}
		float _Start;
		float _End;
		float _Length;
	};

	class Page : public GlObject {
	public:
		Page(String _path) :
			GlObject(),
			_Path(_path),
			_DisplayState(INVISIBLE),
			_Next(NULL),
			_Prev(NULL),
			_Geometry(),
			_Selected(false),
			_Model(ovrMatrix4f_CreateIdentity()),
			_DisplayTime(0),
			_AngularRange(),
			_PixelRange(),
			_Progs(),
			_uDisplayTime(),
			_uContrast(),
			_uBrightness(),
			_Loaded(false),
			_Initialized(false),
			_ATexture(NULL)
		{
			// NOTE: this constructor may be called on a thread
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
		Page *GetNext();
		Page *GetPrev();
		void Load(void);
		virtual void Draw(const Matrix4f &m, const Matrix4f &proj);
		String GetPath() { return _Path; }
		bool IsVisible();
		bool IsLoaded();
		bool IsLoading() {
			return _ATexture->GetState() == TEXTURE_LOADING;
		}
		bool IsTarget(float angle);
		// Does this page appear before the given angle?
		bool IsBefore(float angle);
		// Does this page appear after the given angle?
		bool IsAfter(float angle);
		void SetSelected(bool state);
		void Reset(void);
		bool IsValid() { return _ATexture->IsValid(); }
		float GetStartAngle() {
			return _AngularRange.GetStart();
		}
		float GetEndAngle() {
			return _AngularRange.GetEnd();
		}
		void SetRangeStart(float pxls) {
			_PixelRange.SetStart(pxls);
			_AngularRange.SetStart(pxls / PIXELS_PER_DEGREE);
		}
		void SetRangeEnd(float pxls) {
			_PixelRange.SetEnd(pxls);
			_AngularRange.SetEnd(pxls / PIXELS_PER_DEGREE);
		}

		float GetAngle() {
			return _AngularRange.GetLength();
		}
	protected:
		String _Path;
		DisplayState _DisplayState;
		Page *_Next;
		Page *_Prev;
		GlGeometry _Geometry;
		bool _Selected;
		double _SelectionStart;
		Matrix4f _Model;
		float _ChordLength;
		float _DisplayTime;
		Range _AngularRange;
		Range _PixelRange;
		bool IsPositionable() {
			return _PixelRange.GetStart() != RANGE_UNDEFINED ||
				_PixelRange.GetEnd() != RANGE_UNDEFINED;
		}
	private:
		void UpdateStates(float);

		GlProgram *_Progs[2];
		int _uDisplayTime[2];
		int _uContrast[2];
		int _uBrightness[2];

		bool _Loaded;
		bool _Initialized;

	protected:

		void CreateMesh(void);
		AsyncTexture *_ATexture;
	};


	class LocalPage : public Page {
	public:
		LocalPage(String path) : Page(path) {}
	};

}
