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
    } else if(s == Common || s == Inherited) {
        // Common and Inherited (combining marks) go with any script
    } else if(!used[s]) {
        if(++c > 1)
            return false;
        used[s] = true;
    }
    return true;
}

// Rule 1. The ACE prefix is case-insensitive, so XN-- is an a-label
// too. Atoms are delimited by '.' and '@' here; a quoted string isn't
// an atom, but no a-label hides in one either.
static bool contains_a_label(const std::wstring & s) {
    std::wstring::size_type atom = 0;
    while(atom < s.length()) {
	if(atom + 4 <= s.length() &&
	   (s[atom] == 'x' || s[atom] == 'X') &&
	   (s[atom+1] == 'n' || s[atom+1] == 'N') &&
	   s[atom+2] == '-' && s[atom+3] == '-')
	    return true;
	std::wstring::size_type next = s.find_first_of(L".@", atom);
	if(next == std::wstring::npos)
	    return false;
	atom = next + 1;
    }
    return false;
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
    if(contains_a_label(s))
	return false;
    std::wstring::size_type pos = 0;
    bool used[256] = {};
    unsigned int scripts = 0;
    bool hiraganaKatakanaHanNeeded = false;
    bool arabicIndicDigitsUsed = false;
    bool extendedArabicIndicDigitsUsed = false;
    while(pos < s.length()) {
        unsigned int cp = s[pos];
        if(cp >= 0x110000)
            return false; // beyond unicode, cannot be valid
        if(cp < 128) {
	    // ASCII, and rule 3 disregards it. Class K is %x21-7E and
	    // rule 2 adds SPACE, so DEL and the C0 controls are out.
	    if(cp < 0x20 || cp == 0x7f)
		return false;
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

// The tests from tests.json in this draft's repository, which the
// draft reproduces in its Testing appendix. The invisible code
// points are written as escapes, as they are there.
static const struct {
    const char * address;
    bool valid;
} tests[] = {
    { "example@example.com", true },
    { "Example@example.com", true },
    { "dømi@dømi.fo", true },
    { "example@xn--gr-zia.example.com", false },
    { "xn--gr-zia@example.com", false },
    { "com.xn--gr-zia@example.com", false },
    { "XN--GR-ZIA@example.com", false },
    { "xn-gr@example.com", true },
    { "\u200E4@example.com", false },
    { "\u061C@example.com", false },
    { "\u200E@\u200F.\u200E", false },
    { "ali\u202Emoc.elpmaxe@kravdraa.ec", false },
    { "🐪@example.com", false },
    { "IВM@dømi.fo", false },
    { "阿Q正传@dømi.fo", false },
    { "Толстой@example.com", true },
    { "名字@example.com", true },
    { "阿Q正传@阿Q正传.example", true },
    { "名字@例子.中国", true },
    { "info@例子.中国", true },
    { "उदाहरण@उदाहरण.भारत", true },
    { "gøril@example.com", true },
    { "ا@2ا.ا", true },
    { "ا@example.com", true },
    { "café@example.com", true },
    { "cafe\u0301@example.com", true },
    { "..@example.com", true },
    { "\"john.doe\"@example.com", true },
    { "\"john doe\"@example.com", true },
    { "john\u0009doe@example.com", false },
    { "john\u00A0doe@example.com", false },
    { "grå\u200B\uFEFF\u00AD\u00A0\u2003\u3000@grå.org", false },
    { "cel·la@example.com", true },
    { "cex·xa@example.com", false },
    { "\u0660\u06F0@example.com", false },
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
    for(auto t : tests) {
	std::wstring a = converter.from_bytes(t.address);
	if(smtputf8_syntax_valid(a) != t.valid)
	    std::cout << "Test failed: " << t.address
		      << " (expected " << (t.valid ? "ok" : "nah")
		      << ")" << std::endl;
    }

    // The table holds café twice, in NFC and in NFD. If some tool ever
    // normalizes this source, the two become the same string and the
    // NFD test stops testing anything.
    if(converter.from_bytes("cafe\u0301").length() !=
       converter.from_bytes("café").length() + 1)
	std::cout << "Test failed: café is no longer both NFC and NFD"
		  << std::endl;
}
