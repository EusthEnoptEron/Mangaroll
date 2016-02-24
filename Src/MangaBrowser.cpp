#include "MangaBrowser.h"
#include "Mangaroll.h"
#include "OVR_Locale.h"
#include "Kernel\OVR_File.h"
#include "PackageFiles.h"
#include "OVR_TurboJpeg.h"
#include "ImageData.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_String_Utils.h"

using namespace OVR;

namespace OvrMangaroll{

	MangaBrowser * MangaBrowser::Create(
		Mangaroll & videos,
		OvrGuiSys & guiSys,
		MangaMetadata & metaData,
		unsigned thumbWidth,
		float horizontalPadding,
		unsigned thumbHeight,
		float verticalPadding,
		unsigned 	numSwipePanels,
		float SwipeRadius)
	{
		return new MangaBrowser(videos, guiSys, metaData,
			thumbWidth + horizontalPadding, thumbHeight + verticalPadding, SwipeRadius, numSwipePanels, thumbWidth, thumbHeight);
	}

	String MangaBrowser::GetCategoryTitle(OvrGuiSys & guiSys, const char * tag, const char * key) const
	{
		OVR_UNUSED(guiSys);
		OVR_UNUSED(tag);

		String outStr;
		_Mangaroll->GetLocale().GetString(key, key, outStr);
		return outStr;
	}

	String MangaBrowser::GetPanelTitle(OvrGuiSys & guiSys, const OvrMetaDatum & panelData) const
	{
		OVR_UNUSED(guiSys);
		const MangaMetaDatum * const mangaDatum = static_cast< const MangaMetaDatum * const >(&panelData);
		if (mangaDatum != NULL)
		{
			String outStr;
			_Mangaroll->GetLocale().GetString(mangaDatum->Title.ToCStr(), mangaDatum->Title.ToCStr(), outStr);
			return outStr;
		}
		return String();
	}

	void MangaBrowser::OnPanelActivated(OvrGuiSys & guiSys, const OvrMetaDatum * panelData)
	{
		OVR_UNUSED(guiSys);
		//_Mangaroll->OnMangaActivated(panelData);
	}

	unsigned char * MangaBrowser::LoadThumbnail(const char * filename, int & width, int & height)
	{
		LOG("MangaBrowser::LoadThumbnail loading on %s", filename);
		unsigned char * orig = NULL;

		
		orig = TurboJpegLoadFromFile(filename, &width, &height);
		
		if (orig)
		{
			const int ThumbWidth = GetThumbWidth();
			const int ThumbHeight = GetThumbHeight();

			if (ThumbWidth == width && ThumbHeight == height)
			{
				LOG("MangaBrowser::LoadThumbnail skip resize on %s", filename);
				return orig;
			}

			LOG("MangaBrowser::LoadThumbnail resizing %s to %ix%i", filename, ThumbWidth, ThumbHeight);
			unsigned char * outBuffer = ScaleImageRGBA((const unsigned char *)orig, width, height, ThumbWidth, ThumbHeight, IMAGE_FILTER_CUBIC);
			free(orig);

			if (outBuffer)
			{
				width = ThumbWidth;
				height = ThumbHeight;

				return outBuffer;
			}
		}
		else
		{
			LOG("Error: MangaBrowser::LoadThumbnail failed to load %s", filename);
		}
		return NULL;
	}

	void MangaBrowser::OnMediaNotFound(OvrGuiSys & guiSys, String & title, String & imageFile, String & message)
	{
		_Mangaroll->GetLocale().GetString("@string/app_name", "@string/app_name", title);
		imageFile = "assets/sdcard.png";
		_Mangaroll->GetLocale().GetString("@string/media_not_found", "@string/media_not_found", message);

		OVR::Array< OVR::String > wholeStrs;
		wholeStrs.PushBack("Gear VR");
		guiSys.GetDefaultFont().WordWrapText(message, 1.4f, wholeStrs);
	}

	String MangaBrowser::ThumbName(const String & s)
	{
		String	ts(s);
		ts = StringUtils::SetFileExtensionString(ts.ToCStr(), ".pvr");
		return ts;
	}

}