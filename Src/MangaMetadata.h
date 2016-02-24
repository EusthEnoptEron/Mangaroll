#pragma once

#include "MetaDataManager.h"


using namespace OVR;

namespace OvrMangaroll {

	struct MangaMetaDatum : public OvrMetaDatum
	{
		String	Author;
		String	Title;
		String	ThumbnailUrl;
		String  StreamingType;
		String  StreamingProxy;
		String  StreamingSecurityLevel;

		MangaMetaDatum(const String& url);
	};

	class MangaMetadata : public OvrMetaData {
	public:
		virtual ~MangaMetadata() {}

	protected:
		virtual OvrMetaDatum *	CreateMetaDatum(const char* url) const;
		virtual	void			ExtractExtendedData(const JsonReader & jsonDatum, OvrMetaDatum & outDatum) const;
		virtual	void			ExtendedDataToJson(const OvrMetaDatum & datum, JSON * outDatumObject) const;
		virtual void			SwapExtendedData(OvrMetaDatum * left, OvrMetaDatum * right) const;
	};
}