/* ctype2.c -- ctype function replacements for the Latin-1 character set */

#include "hack.h"

#define CT_CTRL  0x01
#define CT_PUNCT 0x02
#define CT_DIGIT 0x04
#define CT_LOWER 0x08
#define CT_UPPER 0x10
#define CT_BLANK 0x20
#define CT_SPACE 0x40

static unsigned char ctype2[] =
{
    CT_CTRL, /* 0x00 */
    CT_CTRL, /* 0x01 */
    CT_CTRL, /* 0x02 */
    CT_CTRL, /* 0x03 */
    CT_CTRL, /* 0x04 */
    CT_CTRL, /* 0x05 */
    CT_CTRL, /* 0x06 */
    CT_CTRL, /* 0x07 */
    CT_CTRL, /* 0x08 */
    CT_CTRL | CT_BLANK, /* 0x09 */
    CT_CTRL | CT_SPACE, /* 0x0A */
    CT_CTRL | CT_SPACE, /* 0x0B */
    CT_CTRL | CT_SPACE, /* 0x0C */
    CT_CTRL | CT_SPACE, /* 0x0D */
    CT_CTRL, /* 0x0E */
    CT_CTRL, /* 0x0F */
    CT_CTRL, /* 0x10 */
    CT_CTRL, /* 0x11 */
    CT_CTRL, /* 0x12 */
    CT_CTRL, /* 0x13 */
    CT_CTRL, /* 0x14 */
    CT_CTRL, /* 0x15 */
    CT_CTRL, /* 0x16 */
    CT_CTRL, /* 0x17 */
    CT_CTRL, /* 0x18 */
    CT_CTRL, /* 0x19 */
    CT_CTRL, /* 0x1A */
    CT_CTRL, /* 0x1B */
    CT_CTRL, /* 0x1C */
    CT_CTRL, /* 0x1D */
    CT_CTRL, /* 0x1E */
    CT_CTRL, /* 0x1F */
    CT_BLANK, /* space */
    CT_PUNCT, /* ! */
    CT_PUNCT, /* " */
    CT_PUNCT, /* # */
    CT_PUNCT, /* $ */
    CT_PUNCT, /* % */
    CT_PUNCT, /* & */
    CT_PUNCT, /* ' */
    CT_PUNCT, /* ( */
    CT_PUNCT, /* ) */
    CT_PUNCT, /* * */
    CT_PUNCT, /* + */
    CT_PUNCT, /* , */
    CT_PUNCT, /* - */
    CT_PUNCT, /* . */
    CT_PUNCT, /* / */
    CT_DIGIT, /* 0 */
    CT_DIGIT, /* 1 */
    CT_DIGIT, /* 2 */
    CT_DIGIT, /* 3 */
    CT_DIGIT, /* 4 */
    CT_DIGIT, /* 5 */
    CT_DIGIT, /* 6 */
    CT_DIGIT, /* 7 */
    CT_DIGIT, /* 8 */
    CT_DIGIT, /* 9 */
    CT_PUNCT, /* : */
    CT_PUNCT, /* ; */
    CT_PUNCT, /* < */
    CT_PUNCT, /* = */
    CT_PUNCT, /* > */
    CT_PUNCT, /* ? */
    CT_PUNCT, /* @ */
    CT_UPPER, /* A */
    CT_UPPER, /* B */
    CT_UPPER, /* C */
    CT_UPPER, /* D */
    CT_UPPER, /* E */
    CT_UPPER, /* F */
    CT_UPPER, /* G */
    CT_UPPER, /* H */
    CT_UPPER, /* I */
    CT_UPPER, /* J */
    CT_UPPER, /* K */
    CT_UPPER, /* L */
    CT_UPPER, /* M */
    CT_UPPER, /* N */
    CT_UPPER, /* O */
    CT_UPPER, /* P */
    CT_UPPER, /* Q */
    CT_UPPER, /* R */
    CT_UPPER, /* S */
    CT_UPPER, /* T */
    CT_UPPER, /* U */
    CT_UPPER, /* V */
    CT_UPPER, /* W */
    CT_UPPER, /* X */
    CT_UPPER, /* Y */
    CT_UPPER, /* Z */
    CT_PUNCT, /* [ */
    CT_PUNCT, /* \ */
    CT_PUNCT, /* ] */
    CT_PUNCT, /* ^ */
    CT_PUNCT, /* _ */
    CT_PUNCT, /* ` */
    CT_LOWER, /* a */
    CT_LOWER, /* b */
    CT_LOWER, /* c */
    CT_LOWER, /* d */
    CT_LOWER, /* e */
    CT_LOWER, /* f */
    CT_LOWER, /* g */
    CT_LOWER, /* h */
    CT_LOWER, /* i */
    CT_LOWER, /* j */
    CT_LOWER, /* k */
    CT_LOWER, /* l */
    CT_LOWER, /* m */
    CT_LOWER, /* n */
    CT_LOWER, /* o */
    CT_LOWER, /* p */
    CT_LOWER, /* q */
    CT_LOWER, /* r */
    CT_LOWER, /* s */
    CT_LOWER, /* t */
    CT_LOWER, /* u */
    CT_LOWER, /* v */
    CT_LOWER, /* w */
    CT_LOWER, /* x */
    CT_LOWER, /* y */
    CT_LOWER, /* z */
    CT_PUNCT, /* { */
    CT_PUNCT, /* | */
    CT_PUNCT, /* } */
    CT_PUNCT, /* ~ */
    CT_CTRL, /* 0x7F */
    CT_CTRL, /* 0x80 */
    CT_CTRL, /* 0x81 */
    CT_CTRL, /* 0x82 */
    CT_CTRL, /* 0x83 */
    CT_CTRL, /* 0x84 */
    CT_CTRL, /* 0x85 */
    CT_CTRL, /* 0x86 */
    CT_CTRL, /* 0x87 */
    CT_CTRL, /* 0x88 */
    CT_CTRL, /* 0x89 */
    CT_CTRL, /* 0x8A */
    CT_CTRL, /* 0x8B */
    CT_CTRL, /* 0x8C */
    CT_CTRL, /* 0x8D */
    CT_CTRL, /* 0x8E */
    CT_CTRL, /* 0x8F */
    CT_CTRL, /* 0x90 */
    CT_CTRL, /* 0x91 */
    CT_CTRL, /* 0x92 */
    CT_CTRL, /* 0x93 */
    CT_CTRL, /* 0x94 */
    CT_CTRL, /* 0x95 */
    CT_CTRL, /* 0x96 */
    CT_CTRL, /* 0x97 */
    CT_CTRL, /* 0x98 */
    CT_CTRL, /* 0x99 */
    CT_CTRL, /* 0x9A */
    CT_CTRL, /* 0x9B */
    CT_CTRL, /* 0x9C */
    CT_CTRL, /* 0x9D */
    CT_CTRL, /* 0x9E */
    CT_CTRL, /* 0x9F */
    CT_BLANK, /*   */
    CT_PUNCT, /* ¡ */
    CT_PUNCT, /* ¢ */
    CT_PUNCT, /* £ */
    CT_PUNCT, /* ¤ */
    CT_PUNCT, /* ¥ */
    CT_PUNCT, /* ¦ */
    CT_PUNCT, /* § */
    CT_PUNCT, /* ¨ */
    CT_PUNCT, /* © */
    CT_PUNCT, /* ª */
    CT_PUNCT, /* « */
    CT_PUNCT, /* ¬ */
    CT_PUNCT, /* ­ */
    CT_PUNCT, /* ® */
    CT_PUNCT, /* ¯ */
    CT_PUNCT, /* ° */
    CT_PUNCT, /* ± */
    CT_PUNCT, /* ² */
    CT_PUNCT, /* ³ */
    CT_PUNCT, /* ´ */
    CT_PUNCT, /* µ */
    CT_PUNCT, /* ¶ */
    CT_PUNCT, /* · */
    CT_PUNCT, /* ¸ */
    CT_PUNCT, /* ¹ */
    CT_PUNCT, /* º */
    CT_PUNCT, /* » */
    CT_PUNCT, /* ¼ */
    CT_PUNCT, /* ½ */
    CT_PUNCT, /* ¾ */
    CT_PUNCT, /* ¿ */
    CT_UPPER, /* À */
    CT_UPPER, /* Á */
    CT_UPPER, /* Â */
    CT_UPPER, /* Ã */
    CT_UPPER, /* Ä */
    CT_UPPER, /* Å */
    CT_UPPER, /* Æ */
    CT_UPPER, /* Ç */
    CT_UPPER, /* È */
    CT_UPPER, /* É */
    CT_UPPER, /* Ê */
    CT_UPPER, /* Ë */
    CT_UPPER, /* Ì */
    CT_UPPER, /* Í */
    CT_UPPER, /* Î */
    CT_UPPER, /* Ï */
    CT_UPPER, /* Ð */
    CT_UPPER, /* Ñ */
    CT_UPPER, /* Ò */
    CT_UPPER, /* Ó */
    CT_UPPER, /* Ô */
    CT_UPPER, /* Õ */
    CT_UPPER, /* Ö */
    CT_PUNCT, /* × */
    CT_UPPER, /* Ø */
    CT_UPPER, /* Ù */
    CT_UPPER, /* Ú */
    CT_UPPER, /* Û */
    CT_UPPER, /* Ü */
    CT_UPPER, /* Ý */
    CT_UPPER, /* Þ */
    CT_UPPER | CT_LOWER, /* ß */
    CT_LOWER, /* à */
    CT_LOWER, /* á */
    CT_LOWER, /* â */
    CT_LOWER, /* ã */
    CT_LOWER, /* ä */
    CT_LOWER, /* å */
    CT_LOWER, /* æ */
    CT_LOWER, /* ç */
    CT_LOWER, /* è */
    CT_LOWER, /* é */
    CT_LOWER, /* ê */
    CT_LOWER, /* ë */
    CT_LOWER, /* ì */
    CT_LOWER, /* í */
    CT_LOWER, /* î */
    CT_LOWER, /* ï */
    CT_LOWER, /* ð */
    CT_LOWER, /* ñ */
    CT_LOWER, /* ò */
    CT_LOWER, /* ó */
    CT_LOWER, /* ô */
    CT_LOWER, /* õ */
    CT_LOWER, /* ö */
    CT_PUNCT, /* ÷ */
    CT_LOWER, /* ø */
    CT_LOWER, /* ù */
    CT_LOWER, /* ú */
    CT_LOWER, /* û */
    CT_LOWER, /* ü */
    CT_LOWER, /* ý */
    CT_LOWER, /* þ */
    CT_UPPER | CT_LOWER, /* ÿ */
};


const char deaccent[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    ' ',  '!',  '"',  '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
    '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
    '@',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
    'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '[',  '\\', ']',  '^',  '_',
    '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
    'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    ' ',  '!',  0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, '?',
    'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
    'd',  'n',  'o',  'o',  'o',  'o',  'o',  0xD7, 'o',  'u',  'u',  'u',  'u',  'y',  0xDE, 0xDF,
    'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
    'd',  'n',  'o',  'o',  'o',  'o',  'o',  0xF7, 'o',  'u',  'u',  'u',  'u',  'y',  0xFE, 'y'
};

int
isalnum2(c)
int c;
{
    return (ctype2[c & 0xFF] & (CT_LOWER | CT_UPPER | CT_DIGIT)) != 0;
}

int
isalpha2(c)
int c;
{
    return (ctype2[c & 0xFF] & (CT_LOWER | CT_UPPER)) != 0;
}

int
isblank2(c)
int c;
{
    return (ctype2[c & 0xFF] & CT_BLANK) != 0;
}

int
iscntrl2(c)
int c;
{
    return (ctype2[c & 0xFF] & CT_CTRL) != 0;
}

int
isgraph2(c)
int c;
{
    return (ctype2[c & 0xFF] & (CT_LOWER | CT_UPPER | CT_DIGIT | CT_PUNCT)) != 0;
}

int
islower2(c)
int c;
{
    return (ctype2[c & 0xFF] & CT_LOWER) != 0;
}

int
isprint2(c)
int c;
{
    return (ctype2[c & 0xFF] & (CT_LOWER | CT_UPPER | CT_DIGIT | CT_PUNCT | CT_SPACE | CT_BLANK)) != 0;
}

int
ispunct2(c)
int c;
{
    return (ctype2[c & 0xFF] & CT_PUNCT) != 0;
}

int
isspace2(c)
int c;
{
    return (ctype2[c & 0xFF] & (CT_SPACE | CT_BLANK)) != 0;
}

int
isupper2(c)
int c;
{
    return (ctype2[c & 0xFF] & CT_UPPER) != 0;
}

int
tolower2(c)
int c;
{
    c &= 0xFF;
    if ((ctype2[c] & (CT_UPPER | CT_LOWER)) == CT_UPPER) c += 32;
    return c;
}

int
toupper2(c)
int c;
{
    c &= 0xFF;
    if ((ctype2[c] & (CT_UPPER | CT_LOWER)) == CT_LOWER) c -= 32;
    return c;
}

/* A strcmpi that recognizes accented letters */

int
strcmpi2(str1, str2)
const char *str1;
const char *str2;
{
    size_t i;
    int d;

    for (i=0; str1[i]!=0 || str2[i]!=0; i++)
    {
        d = tolower2(str1[i]) - tolower2(str2[i]);
        if (d != 0) return d;
    }
    return 0;
}

/* A strcmpi that considers accented letters equal to unaccented */

int
strcmpi2a(str1, str2)
const char *str1;
const char *str2;
{
    size_t i;
    int d;

    for (i=0; str1[i]!=0 || str2[i]!=0; i++)
    {
        d = deaccent[str1[i] & 0xFF] - deaccent[str2[i] & 0xFF];
        if (d != 0) return d;
    }
    return 0;
}

/* A strncmpi that recognizes accented letters */

int
strncmpi2(str1, str2, len)
const char *str1;
const char *str2;
size_t len;
{
    size_t i;
    int d;

    for (i=0; (str1[i]!=0 || str2[i]!=0) && i<len; i++)
    {
        d = tolower2(str1[i]) - tolower2(str2[i]);
        if (d != 0) return d;
    }
    return 0;
}

/* A strncmpi that considers accented letters equal to unaccented */

int
strncmpi2a(str1, str2, len)
const char *str1;
const char *str2;
size_t len;
{
    size_t i;
    int d;

    for (i=0; (str1[i]!=0 || str2[i]!=0) && i<len; i++)
    {
        d = deaccent[str1[i] & 0xFF] - deaccent[str2[i] & 0xFF];
        if (d != 0) return d;
    }
    return 0;
}


/* Translation table from ISO 8859-1 to 7 bit ASCII */
static const unsigned char asciiconvert[] =
{
    /* Identity mapping for the low characters */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,

    /* The control codes are transformed in a manner paralleling ibm850convert */
    ' ',  ' ',  ' ',  '-',  ' ',  '+',  '+',  '+',
    ' ',  '+',  '+',  '+',  '|',  '+',  '+',  '+',
    ' ',  ' ',  ' ',  '-',  ' ',  '+',  '+',  '+',
    ' ',  '+',  '+',  '+',  '|',  '+',  '+',  '+',
    /* Transform for the others; where ibmconvert uses an ASCII character, so
       should asciiconvert */
    ' ',  '!',  'L',  'c',  '$',  'Y',  '|',  'S',
    '"',  'C',  'a',  '"',  '-',  '-',  'R',  '-',
    'o',  '+',  '2',  '3',  '\'', 'u',  'P',  '.',
    ',',  '1',  'o',  '"',  '4',  '2',  '3',  '?',
    'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',
    'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
    'D',  'N',  'O',  'O',  'O',  'O',  'O',  'x',
    'O',  'U',  'U',  'U',  'U',  'Y',  'T',  'S',
    'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',
    'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
    'd',  'n',  'o',  'o',  'o',  'o',  'o',  '/',
    'o',  'u',  'u',  'u',  'u',  'y',  't',  'y',
};

/* Translation table from ISO 8859-1 to IBM code page 437 */
static const unsigned char ibmconvert[] =
{
    /* Identity mapping for the low characters */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,

    /* This transform is not reversible:  missing accented letters are mapped
       to the equivalent unaccented letter */
    /* For the Unicode-based systems, this now transforms as an identity map
       for characters that can be printed; it must be run through ibm850convert
       to get true code page 437. */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, '$' , 0xA5, '|' , 0xA7,
    '"' , 'C' , 0xAA, 0xAB, 0xAC, '-' , 'R' , '-' ,
    0xB0, 0xB1, 0xB2, '3' , '\'', 0xB5, 0xB6, 0xB7,
    ',' , '1' , 0xBA, 0xBB, 0xBC, 0xBD, '3' , 0xBF,
    'A' , 'A' , 'A' , 'A' , 0xC4, 0xC5, 0xC6, 0xC7,
    'E' , 0xC9, 'E' , 'E' , 'I' , 'I' , 'I' , 'I' ,
    'D' , 0xD1, 'O' , 'O' , 'O' , 'O' , 0xD6, 'x' ,
    'O' , 'U' , 'U' , 'U' , 0xDC, 'Y' , 'T' , 0xDF,
    0xE0, 0xE1, 0xE2, 'a' , 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 'o' , 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 'y' , 't' , 0xFF
};

#if defined(MSDOS) || defined(WIN32) || defined(OS2)
/* Translation table from ISO 8859-1 to IBM code page 850 */
static const unsigned char ibm850convert[] =
{
    /* Identity mapping for the low characters */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,

    /* The mappings for codes 80 through 9F are chosen to place the line
       drawing characters in a logical order.  The other characters are
       placed to make a reversible transform, and are otherwise arbitrary. */
/*                    ..lr        .d.r  .dl.  .dlr        u..r  u.l.  u.lr  ud..  ud.r  udl.  udlr */
    0xB0, 0xB1, 0xB2, 0xC4, 0xFE, 0xDA, 0xBF, 0xC2, 0x9F, 0xC0, 0xD9, 0xC1, 0xB3, 0xC3, 0xB4, 0xC5,
    0xDC, 0xDF, 0xDB, 0xCD, 0xF2, 0xC9, 0xBB, 0xCB, 0xD5, 0xC8, 0xBC, 0xCA, 0xBA, 0xCC, 0xB9, 0xCE,
    /* The mappings for codes A0 through FF convert ISO 8859-1 characters to
       the equivalent code page 850 characters */
    0xFF, 0xAD, 0xBD, 0x9C, 0xCF, 0xBE, 0xDD, 0xF5,
    0xF9, 0xB8, 0xA6, 0xAE, 0xAA, 0xF0, 0xA9, 0xEE,
    0xF8, 0xF1, 0xFD, 0xFC, 0xEF, 0xE6, 0xF4, 0xFA,
    0xF7, 0xFB, 0xA7, 0xAF, 0xAC, 0xAB, 0xF3, 0xA8,
    0xB7, 0xB5, 0xB6, 0xC7, 0x8E, 0x8F, 0x92, 0x80,
    0xD4, 0x90, 0xD2, 0xD3, 0xDE, 0xD6, 0xD7, 0xD8,
    0xD1, 0xA5, 0xE3, 0xE0, 0xE2, 0xE5, 0x99, 0x9E,
    0x9D, 0xEB, 0xE9, 0xEA, 0x9A, 0xED, 0xE7, 0xE1,
    0x85, 0xA0, 0x83, 0xC6, 0x84, 0x86, 0x91, 0x87,
    0x8A, 0x82, 0x88, 0x89, 0x8D, 0xA1, 0x8C, 0x8B,
    0xD0, 0xA4, 0x95, 0xA2, 0x93, 0xE4, 0x94, 0xF6,
    0x9B, 0x97, 0xA3, 0x96, 0x81, 0xEC, 0xE8, 0x98
};
#endif

/* Translate a character to the assumed character set */

int
translate_ch(ch)
int ch;
{
    ch &= 0xFF;
    /* For displays with only ASCII */
    if (iflags.accents == 0)
        ch = asciiconvert[ch];
    /* For displays running the default IBM character set */
    else if (iflags.accents == 1)
        ch = ibmconvert[ch];
    /* MSDOS, WIN32, and OS/2 use a permuted version of ISO-8859-1 */
#if defined(WIN32)
    if (!iflags.unicode)
	ch = ibm850convert[ch];
#endif
#if defined(MSDOS) || defined(OS2)
    ch = ibm850convert[ch];
#endif
    return ch;
}
