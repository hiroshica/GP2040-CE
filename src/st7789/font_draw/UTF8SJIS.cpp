#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

// 参考サイト
// http://f4.aaacafe.ne.jp/~pointc/log1243.html
//
#define IS_SJIS(c) (((c) ^ 0x20) - 0xa1u < 60)
#define MB_LEN_MAX (5)
void UTF8ToSJIS(uint8_t *indata, uint8_t *outdata, int32_t outsize)
{
    int c;
    char mb[MB_LEN_MAX];
    wchar_t wc;
    int32_t oiI = 0;
    setlocale(LC_CTYPE, "jpn");
    *(outdata + oiI) = 0x00;
    for (int iI = 0; iI < MB_LEN_MAX; ++iI)
    {
        mb[iI] = 0;
    }

    while (1)
    {
        c = *indata++;
        if (c == 0)
        {
            break;
        }
        else if (c < 0x80)
        {
            wc = c;
        }
        else if (c < 0xc0)
        {
            continue;
        }
        else if (c < 0xe0)
        {
            wc = (c & 0x1f) << 6;
            c = *indata++;
            if (c == 0)
            {
                break;
            }
            wc |= c & 0x3f;
        }
        else if (c < 0xf0)
        {
            wc = (c & 0x0f) << 12;
            c = *indata++;
            if (c == 0)
            {
                break;
            }
            wc |= (c & 0x3f) << 6;
            c = *indata++;
            if (c == 0)
            {
                break;
            }
            wc |= c & 0x3f;
        }
        else
        {
            continue;
        }
        c = wctomb(mb, wc);
        if ((oiI + c + 1) >= outsize)
        {
            break;
        }
        for (int iJ = 0; iJ < c; ++iJ)
        {
            *(outdata + oiI++) = mb[iJ];
            *(outdata + oiI) = 0x00;
        }
    }
}

void SJIStoUTF8(FILE *fin, FILE *fout)
{
    int c;
    char mb[2];
    wchar_t wc;

    while ((c = getc(fin)) != EOF)
    {
        mb[0] = c;
        if (IS_SJIS(c))
        {
            if ((c = getc(fin)) == EOF)
                break;
            mb[1] = c;
            if (mbtowc(&wc, mb, 2) != 2)
                continue;
        }
        else if (mbtowc(&wc, mb, 1) != 1)
            continue;

        if (wc <= 0x7f)
            putc(wc, fout);
        else if (wc <= 0x7ff)
        {
            putc(wc >> 6 | 0xc0, fout);
            putc(wc & 0x3f | 0x80, fout);
        }
        else
        {
            putc(wc >> 12 | 0xe0, fout);
            putc(wc >> 6 & 0x3f | 0x80, fout);
            putc(wc & 0x3f | 0x80, fout);
        }
    }
}
