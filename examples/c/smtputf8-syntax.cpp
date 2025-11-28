#include <codecvt>
#include <locale>
#include <iostream>
#include <string>

/* This large enum was created by the following command:

   (awk '/^\[/{print $4}' < unicodedata-1.h ; \
    awk '/^[^\/]/{print $1}' < unicodedata-2.h ) | \
    sort -u | fmt -70 | sed 's/^/    /'
*/
typedef enum {
    Adlam, Ahom, Anatolian_Hieroglyphs, Arabic, Armenian, Avestan,
    Balinese, Bamum, Bassa_Vah, Batak, Bengali, Beria_Erfe, Bhaiksuki,
    Bopomofo, Brahmi, Buginese, Buhid, Canadian_Aboriginal, Carian,
    Caucasian_Albanian, Chakma, Cham, Cherokee, Chorasmian, Common,
    Coptic, Cuneiform, Cypriot, Cypro_Minoan, Cyrillic, Deseret,
    Devanagari, Dives_Akuru, Dogra, Duployan, Egyptian_Hieroglyphs,
    Elbasan, Elymaic, Ethiopic, Garay, Georgian, Glagolitic, Gothic,
    Grantha, Greek, Gujarati, Gunjala_Gondi, Gurmukhi, Gurung_Khema,
    Han, Hangul, Hanifi_Rohingya, Hanunoo, Hatran, Hebrew,
    Hiragana, Imperial_Aramaic, Inherited, Inscriptional_Pahlavi,
    Inscriptional_Parthian, Javanese, Kaithi, Kannada, Katakana, Kawi,
    Kayah_Li, Kharoshthi, Khitan_Small_Script, Khmer, Khojki, Khudawadi,
    Kirat_Rai, Lao, Latin, Lepcha, Limbu, Linear_A, Linear_B, Lisu,
    Lycian, Lydian, Mahajani, Makasar, Malayalam, Mandaic, Manichaean,
    Marchen, Masaram_Gondi, Medefaidrin, Meetei_Mayek, Mende_Kikakui,
    Meroitic_Cursive, Meroitic_Hieroglyphs, Miao, Modi, Mongolian,
    Mro, Multani, Myanmar, Nabataean, Nag_Mundari, Nandinagari, Newa,
    New_Tai_Lue, Nko, None, Nushu, Nyiakeng_Puachue_Hmong, Ogham,
    Ol_Chiki, Old_Hungarian, Old_Italic, Old_North_Arabian, Old_Permic,
    Old_Persian, Old_Sogdian, Old_South_Arabian, Old_Turkic, Old_Uyghur,
    Ol_Onal, Oriya, Osage, Osmanya, Pahawh_Hmong, Palmyrene, Pau_Cin_Hau,
    Phags_Pa, Phoenician, Psalter_Pahlavi, Rejang, Runic, Samaritan,
    Saurashtra, Sharada, Shavian, Siddham, Sidetic, SignWriting, Sinhala,
    Sogdian, Sora_Sompeng, Soyombo, Sundanese, Sunuwar, Syloti_Nagri,
    Syriac, Tagalog, Tagbanwa, Tai_Le, Tai_Tham, Tai_Viet, Tai_Yo, Takri,
    Tamil, Tangsa, Tangut, Telugu, Thaana, Thai, Tibetan, Tifinagh,
    Tirhuta, Todhri, Tolong_Siki, Toto, Tulu_Tigalari, Ugaritic, Vai,
    Vithkuqi, Wancho, Warang_Citi, Yezidi, Yi, Zanabazar_Square } Script;

const struct {
    // this is just 63 bits in total, so a plain scan will be fast on
    // most CPUs made after 2005.
    unsigned int start : 21;
    unsigned int length : 20;
    Script script: 8;
    signed int index : 14;
} unicodeData[577] = {
#include "unicodedata-1.h"
};

const Script supplement[7441] = {
#include "unicodedata-2.h"
};

static bool single_char_helper(unsigned int cp,
			       Script s,
			       unsigned int & c,
			       bool (& used)[256]) {
    if(s == None) {
        return false;
    } else if(s == Common) {
        // Common can be combined with others
    } else if(!used[s]) {
        if(++c > 1)
            return false;
        used[s] = true;
    }
    return true;
}

static Script script_of(unsigned int cp) {
    int n = 0;
    while(n < 576 && unicodeData[n].start < cp)
	n++;
    if(unicodeData[n].start > cp)
	n--;
    if(unicodeData[n].index < 0) {
	return unicodeData[n].script;
    } else {
	int i = cp - unicodeData[n].start + unicodeData[n].index;
	return supplement[i];
    }
}

bool smtputf8_syntax_valid(const std::wstring s) {
    int pos = 0;
    bool used[256];
    unsigned int scripts = 0;
    bool hiraganaKatakanaHanNeeded = false;
    bool arabicIndicDigitsUsed = false;
    bool extendedArabicIndicDigitsUsed = false;
    while(pos < s.length()) {
        unsigned int cp = s[pos];
        if(cp >= 0x110000)
            return false; // beyond unicode, cannot be valid
        if(cp < 128) {
            // it's ASCII
	} else if(cp == 0xB7) {
	    // RFC 5892 Appendix A.3
	    if(pos == 0 || pos+1 >= s.length())
		return false; // not between two letters
	    if(s[pos-1] == 'l' && s[pos+1] == 'l')
		; // lower case ela geminada
	    else if(s[pos-1] == 'L' && s[pos+1] == 'L')
		;// upper case ela geminada
	    else
		return false; // some other middle dot
	} else if(cp == 0x375) {
	    // RFC 5892 Appendix A.4
	    if(pos+1 >= s.length())
		return false; // no following character
	    if(script_of(s[pos+1]) != Greek)
		return false; // not followed by a Greek character
	} else if(cp == 0x5F3 || cp == 0x5F4) {
	    // RFC 5892 Appendix A.5 and A.6
	    if(pos < 1)
		return false; // no preceding character
	    if(script_of(s[pos-1]) != Hebrew)
		return false; // not preceded by a Hebrew letter
	} else if(cp == 0x30FB) {
	    // RFC 5892 Appendix A.7
	    hiraganaKatakanaHanNeeded = true;
	} else if(cp >= 0x0660 && cp <= 0x0669) {
	    // RFC 5892 Appendix A.8
	    arabicIndicDigitsUsed = true;
	} else if(cp >= 0x06f0 && cp <= 0x06f9) {
	    // RFC 5892 Appendix A.9
	    extendedArabicIndicDigitsUsed = true;
        } else {
	    Script s = script_of(cp);
	    if(!single_char_helper(cp, s, scripts, used))
		return false;
        }
	pos++;
    }
    if(hiraganaKatakanaHanNeeded &&
       !used[Hiragana] && !used[Katakana] && !used[Han])
	return false;
    if(arabicIndicDigitsUsed && extendedArabicIndicDigitsUsed)
	return false;
    return true;
}

static const char * should_pass[] = {
    "test",
    "naïve",
    "dømi",
    "阿Q正传@阿Q正传.example",
    "",
    "राजीव",
    "cel·la"
};

static const char * should_fail[] = {
    "cex·xa",
    "cel·"
};

int main(int argc, char ** argv) {
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    // command-line fun:
    int i = 1;
    while(i < argc) {
        std::wstring us = converter.from_bytes(argv[i]);
        std::cout << argv[i] << ": "
                  << ( smtputf8_syntax_valid(us) ? "ok" : "nah" )
                  << std::endl;
        i++;
    }

    // tests:
    for(const char * t : should_pass)
	if(!smtputf8_syntax_valid(converter.from_bytes(t)))
	    std::cout << "Test failed: " << t << std::endl;
    for(const char * t : should_fail)
	if(smtputf8_syntax_valid(converter.from_bytes(t)))
	    std::cout << "Test failed: " << t << std::endl;
}
