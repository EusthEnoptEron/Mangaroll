#include "TranslationManager.h"
#include "Mangaroll.h"
#include "OVR_Locale.h"

namespace OvrMangaroll {

	TranslationManager::TranslationManager(Mangaroll *mangaroll) {
		mangaroll->GetLocale().GetString("@string/app_name", "@string/app_name", AppName);
		mangaroll->GetLocale().GetString("@string/action_settings", "@string/action_settings", ActionSettings);
		mangaroll->GetLocale().GetString("@string/no_local_files", "@string/no_local_files", NoLocalFiles);
		mangaroll->GetLocale().GetString("@string/no_services", "@string/no_services", NoServices);
		mangaroll->GetLocale().GetString("@string/no_results", "@string/no_results", NoResults);
		mangaroll->GetLocale().GetString("@string/label_local", "@string/label_local", LabelLocal);
		mangaroll->GetLocale().GetString("@string/label_online", "@string/label_online", LabelOnline);
		mangaroll->GetLocale().GetString("@string/page_no", "@string/page_no", PageNo);
		mangaroll->GetLocale().GetString("@string/pref_contrast", "@string/pref_contrast", PrefContrast);
		mangaroll->GetLocale().GetString("@string/pref_brightness", "@string/pref_brightness", PrefBrightness);
		mangaroll->GetLocale().GetString("@string/pref_sharpen", "@string/pref_sharpen", PrefSharpen);
		mangaroll->GetLocale().GetString("@string/pref_transparent", "@string/pref_transparent", PrefTransparent);
		mangaroll->GetLocale().GetString("@string/pref_read_dir", "@string/pref_read_dir", PrefReadDir);
		mangaroll->GetLocale().GetString("@string/pref_auto_progress", "@string/pref_auto_progress", PrefAutoProgress);
		mangaroll->GetLocale().GetString("@string/pref_head_motions", "@string/pref_head_motions", PrefHeadMotions);
	}

}