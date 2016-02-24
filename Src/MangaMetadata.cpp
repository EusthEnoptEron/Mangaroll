#include "MangaMetadata.h"
#include "Kernel/OVR_JSON.h"
#include "VrCommon.h"

using namespace OVR;

namespace OvrMangaroll {

	const char * const TITLE_INNER = "title";
	const char * const AUTHOR_INNER = "author";
	const char * const THUMBNAIL_URL_INNER = "thumbnail_url";
	const char * const STREAMING_TYPE_INNER = "streaming_type";
	const char * const STREAMING_PROXY_INNER = "streaming_proxy";
	const char * const STREAMING_SECURITY_LEVEL_INNER = "streaming_security_level";
	const char * const DEFAULT_AUTHOR_NAME = "Unspecified Author";


	MangaMetaDatum::MangaMetaDatum(const String& url)
		: Author(DEFAULT_AUTHOR_NAME)
	{
		Title = ExtractFileBase(url);
	}

	OvrMetaDatum * MangaMetadata::CreateMetaDatum(const char* url) const
	{
		return new MangaMetaDatum(url);
	}

	void MangaMetadata::ExtractExtendedData(const JsonReader & jsonDatum, OvrMetaDatum & datum) const
	{
		MangaMetaDatum * mangaData = static_cast< MangaMetaDatum * >(&datum);
		if (mangaData)
		{
			mangaData->Title = jsonDatum.GetChildStringByName(TITLE_INNER);
			mangaData->Author = jsonDatum.GetChildStringByName(AUTHOR_INNER);
			mangaData->ThumbnailUrl = jsonDatum.GetChildStringByName(THUMBNAIL_URL_INNER);
			mangaData->StreamingType = jsonDatum.GetChildStringByName(STREAMING_TYPE_INNER);
			mangaData->StreamingProxy = jsonDatum.GetChildStringByName(STREAMING_PROXY_INNER);
			mangaData->StreamingSecurityLevel = jsonDatum.GetChildStringByName(STREAMING_SECURITY_LEVEL_INNER);

			if (mangaData->Title.IsEmpty())
			{
				mangaData->Title = ExtractFileBase(datum.Url.ToCStr());
			}

			if (mangaData->Author.IsEmpty())
			{
				mangaData->Author = DEFAULT_AUTHOR_NAME;
			}
		}
	}

	void MangaMetadata::ExtendedDataToJson(const OvrMetaDatum & datum, JSON * outDatumObject) const
	{
		if (outDatumObject)
		{
			const MangaMetaDatum * const mangaData = static_cast< const MangaMetaDatum * const >(&datum);
			if (mangaData)
			{
				outDatumObject->AddStringItem(TITLE_INNER, mangaData->Title.ToCStr());
				outDatumObject->AddStringItem(AUTHOR_INNER, mangaData->Author.ToCStr());
				outDatumObject->AddStringItem(THUMBNAIL_URL_INNER, mangaData->ThumbnailUrl.ToCStr());
				outDatumObject->AddStringItem(STREAMING_TYPE_INNER, mangaData->StreamingType.ToCStr());
				outDatumObject->AddStringItem(STREAMING_PROXY_INNER, mangaData->StreamingProxy.ToCStr());
				outDatumObject->AddStringItem(STREAMING_SECURITY_LEVEL_INNER, mangaData->StreamingSecurityLevel.ToCStr());
			}
		}
	}

	void MangaMetadata::SwapExtendedData(OvrMetaDatum * left, OvrMetaDatum * right) const
	{
		MangaMetaDatum * leftmangaData = static_cast< MangaMetaDatum * >(left);
		MangaMetaDatum * rightmangaData = static_cast< MangaMetaDatum * >(right);
		if (leftmangaData && rightmangaData)
		{
			Alg::Swap(leftmangaData->Title, rightmangaData->Title);
			Alg::Swap(leftmangaData->Author, rightmangaData->Author);
		}
	}

}