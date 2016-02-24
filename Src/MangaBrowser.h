#pragma once

#include "FolderBrowser.h"
#include "MangaMetadata.h"
#include "BitmapFont.h"

using namespace OVR;

namespace OvrMangaroll {

	class Mangaroll;

	class MangaBrowser : public OvrFolderBrowser {
	public:
		// only one of these every needs to be created
		static  MangaBrowser*  Create(
			Mangaroll&	app,
			OvrGuiSys &			guiSys,
			MangaMetadata &	metaData,
			unsigned			thumbWidth,
			float				horizontalPadding,
			unsigned			thumbHeight,
			float				verticalPadding,
			unsigned 			numSwipePanels,
			float				SwipeRadius);

		// Called when a panel is activated
		virtual void OnPanelActivated(OvrGuiSys & guiSys, const OvrMetaDatum * panelData);
		// Called on a background thread to load thumbnail
		virtual	unsigned char * LoadThumbnail(const char * filename, int & width, int & height);
		
		// Display appropriate info if we fail to find media
		virtual void OnMediaNotFound(OvrGuiSys & guiSys, String & title, String & imageFile, String & message);
		
		// Adds thumbnail extension to a file to find/create its thumbnail
		virtual String	ThumbName(const String & s);

	protected:
		// Called from the base class when building a cateory.
		virtual String				GetCategoryTitle(OvrGuiSys & guiSys, const char * tag, const char * key) const;

		// Called from the base class when building a panel
		virtual String				GetPanelTitle(OvrGuiSys & guiSys, const OvrMetaDatum & panelData) const;

	private:
		MangaBrowser(
			Mangaroll &	app,
			OvrGuiSys & guiSys,
			MangaMetadata & metaData,
			float panelWidth,
			float panelHeight,
			float radius,
			unsigned numSwipePanels,
			unsigned thumbWidth,
			unsigned thumbHeight)
			: OvrFolderBrowser(guiSys, metaData, panelWidth, panelHeight, radius, numSwipePanels, thumbWidth, thumbHeight)
			, _Mangaroll(&app)
		{
		}
		Mangaroll *_Mangaroll;
	};
}