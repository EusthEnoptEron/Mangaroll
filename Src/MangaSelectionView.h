#pragma once

#include "View.h"
#include "UI\UIMenu.h"
#include "UI\UIContainer.h"
#include "UI\UILabel.h"
#include "UI\UIMenu.h"
#include "UI\UITexture.h"
#include "UI\UIImage.h"
#include "VRMenuComponent.h"
#include "AsyncTexture.h"
#include "MangaProvider.h"
#include "GazeUpdateComponent.h"
#include "UI\UIButton.h"

using namespace OVR;
namespace OvrMangaroll {

	// Forward declaration
	class Mangaroll;
	class Manga;
	class MangaSelectorComponent;

	class CategoryComponent : public ClickableComponent {
	public:
		CategoryComponent();
		virtual ~CategoryComponent() { }
		bool Selected;
		Vector4f ColNormal;
		Vector4f ColFocused;
		Vector4f ColHighlight;
	protected:
		virtual eMsgStatus _OnEvent(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);
	private:
		bool _Focused;
	};


	class MangaPanel : public UIObject {
	public:
		MangaPanel(OvrGuiSys &guiSys);
		virtual ~MangaPanel();
		void SetSelector(MangaSelectorComponent *selector);

		void AddToMenu(UIMenu *menu, UIObject *parent = NULL);
		void SetManga(Manga *manga);
		Manga *GetManga();
		float Position;
		int Index;
		int Width;
		int Height;
		int Border;
	private:
		MangaSelectorComponent *_Selector;
		Manga *_Manga;
		void Init(void);
		UIImage *_Background;
		UILabel *_Title;
		UIImage *_Cover;
		UITexture *_CoverTexture;
		ClickableComponent *_Component;

		static void OnClick(void *);
	};


	class MangaSelectorComponent : public VRMenuComponent {
	public:
		MangaSelectorComponent(OvrGuiSys &guiSys);
		virtual ~MangaSelectorComponent() { }

		void SetProvider(MangaProvider &provider);
		void AddToMenu(UIMenu *menu, UIObject *parent);

		void SetOnSelectManga(void(*cb)(Manga*, void*), void* obj) {
			this->_Callback = cb;
			this->_CallbackObject = obj;
		}

		void SelectManga(Manga *manga);
		bool CanSeek();
		bool CanSeekBack();

		// Constants because I'm lazy
		static int const COLS = 10;
		static int const ROWS = 2;
		static int const ANGLE = 120; // Degrees
		static int const MARGIN = 20;

		// Number of visible panels on either side
		static int const NUM_VISIBLE_PANELS = 2;
	private:
		virtual eMsgStatus OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);

		void Update(VRMenuEvent const & evt);
		void CleanPanels();
		void UpdatePanels();
		static void OnGoBackward(UIButton *, void *);
		static void OnGoForward(UIButton *, void *);

		UIMenu *_Menu;
		UIObject *_Parent;
		UIContainer *_Container;
		OvrGuiSys &_Gui;
		void(*_Callback)(Manga*, void*);
		void *_CallbackObject;
		Array<MangaPanel*> _Panels;
		Array<MangaPanel*> _UsedPanels;
		int _Index;
		float _Speed;
		float _Gravity;
		Vector2f _LastTouch;
		float _TouchStartTime;
		bool _IsTouching;
		float _PanelHeight;
		MangaProvider *_Provider;
		int _PanelCount;
		UITexture _Arrow;
		UITexture _ArrowLeftTexture;
		UITexture _Fill;

		UILabel *_Title;
		UIButton *_ArrowRight;
		UIButton *_ArrowLeft;

		VRMenuObject *_ActivePanel;
		double _FocusLostTime;
	};

	class MangaSelectionView : public View {
	public:
		MangaSelectionView(Mangaroll &app);
		virtual ~MangaSelectionView();

		virtual void 		OneTimeInit(const char * launchIntent);
		virtual void		OneTimeShutdown();

		virtual void 		OnOpen();
		virtual void 		OnClose();
		virtual bool 		OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType);
		virtual Matrix4f 	Frame(const VrFrame & vrFrame);
		virtual Matrix4f	GetEyeViewMatrix(const int eye) const;
		virtual Matrix4f	GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const;
		virtual Matrix4f 	DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms);

		void OnSelectManga(Manga *);
 
	private:
		Mangaroll &_Mangaroll;
		UIMenu *_Menu;

		UIContainer *_MainContainer;
		UILabel *_LocalSrcLabel;
		UILabel *_RemoteSrcLabel;

		MangaSelectorComponent *_Selector;
		UIContainer *_SelectorContainer;
		UITexture _FillTexture;

		LocalMangaProvider _LocalMangaProvider;
		RemoteMangaProvider _NProvider;

		CategoryComponent _LocalCategoryComponent;
		CategoryComponent _RemoteCategoryComponent;

		static void OnLocalCategory(void *p);
		static void OnRemoteCategory(void *p);
	};


}