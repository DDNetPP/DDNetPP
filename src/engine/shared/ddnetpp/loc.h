#ifndef ENGINE_SHARED_DDNETPP_LOC_H
#define ENGINE_SHARED_DDNETPP_LOC_H

enum
{
	LANG_EN,
	LANG_RU,
};

// turns `"en"` into `0` (`LANG_EN`)
int str_to_lang_id(const char *pLanguage);

// Translate strings shown to users
// (localize)
//
// @param Language can be `LANG_EN` or `LANG_RU`
// @param pLocalized output buffer where the localized string will be written to
// @param LocalizedSize sie of the output buffer in bytes
// @param pFormat english input text that will be translated and can include format specifiers
// @param ... values for the `pFormat` format specifiers
//
// @return translated string
[[gnu::format(printf, 4, 5)]] void str_ddpp_loc_to_buf(int Language, char *pLocalized, int LocalizedSize, const char *pFormat, ...);

// Translate strings shown to users
// (localize)
//
// @param Language can be `LANG_EN` or `LANG_RU`
// @param pStr english input text that should be translated
//
// @return translated string
[[gnu::format_arg(2)]] const char *str_ddpp_loc(int Language, const char *pStr);

#endif
