//---------------------------------------------------------------------------
#include "InkFont.h"
//---------------------------------------------------------------------------
InkFont::InkFont(SdFs *pSDCard,  TFT_eSPI *pScreen)
{
    FpSDCard = pSDCard;
    FpScreen = pScreen;
    FboolFontInit = false;
    fgColor = TFT_WHITE;
    bgColor = TFT_BLACK;
}

InkFont::InkFont()
{
  FpSDCard = NULL;
  FpScreen = NULL;
  FboolFontInit = NULL;
  fgColor = TFT_WHITE;
  bgColor = TFT_BLACK;
}

void InkFont::Init(SdFs *pSDCard,  TFT_eSPI *pScreen)
{
    FpSDCard = pSDCard;
    FpScreen = pScreen;
    FboolFontInit = false;
}

//---------------------------------------------------------------------------
int16_t InkFont::GetDisplayWidth()
{
  return FpScreen->width();
  //返回屏幕的宽度，以像素为单位
}
//---------------------------------------------------------------------------
int16_t InkFont::GetDisplayHeight()
{
    return FpScreen->height();
    //返回屏幕的高度，以像素为单位
}
//---------------------------------------------------------------------------
void InkFont::DrawPixel(const int16_t nX, const int16_t nY, const byte nColor)
{
    //画点，nColor = 0代表使用背景色画点，nColor != 0代表使用前景色画点
    if(nColor == 0)
      FpScreen->drawPixel(nX, nY, bgColor);
    else
      FpScreen->drawPixel(nX, nY, fgColor);
}
//---------------------------------------------------------------------------
boolean InkFont::GetCharBitmap(const uint32_t nCharCode, const uint8_t nBFLType, FsFile& FontFile)
{
    boolean boolBFLBitmap = false;

    //Unicode
    if (nBFLType == 1)
    {
        //获取字体文件段信息
        for (byte nSectionIndex = 0; nSectionIndex < FBFLHead.Section; nSectionIndex++)
        {
            if (! FontFile.seek(nSectionIndex * sizeof(FBFLSection) + sizeof(FBFLHead))) break;
            if (FontFile.read(&FBFLSection, sizeof(FBFLSection)) != sizeof(FBFLSection)) break;

            if ((nCharCode >= FBFLSection.First) && (nCharCode <= FBFLSection.Last))
            {
                //获取字体文件检索表
                if (! FontFile.seek((nCharCode - FBFLSection.First) * sizeof(FBFLSearch) + FBFLSection.OffAddr)) break;
                if (FontFile.read(&FBFLSearch, sizeof(FBFLSearch)) != sizeof(FBFLSearch)) break;
                boolBFLBitmap = true;
                break;
            }
        }
    }
    //MBCS等宽
    else if (nBFLType == 2)
    {
        uint32_t nEncodeIndex = nCharCode;
        byte nHigh8Bits = (nCharCode >> 8) & 0xff;
        byte nLower8Bits = nCharCode & 0xff;

        //获取字体文件检索表
        if (FstrEncodeType == "GBK")
        {
            if ((nHigh8Bits >= 0x81) && (nHigh8Bits <= 0xfe) && (((nLower8Bits >= 0x40) && (nLower8Bits <= 0x7e)) || ((nLower8Bits >= 0x80) && (nLower8Bits <= 0xfe))))
            {
                nEncodeIndex = (nHigh8Bits - 0x81) * 0xbe + (nLower8Bits - 0x40);  //0xbe = (0x7e - 0x40 + 1) + (0xfe - 0x80 + 1)
                if (nLower8Bits >= 0x80) nEncodeIndex -= 1;
                boolBFLBitmap = true;
            }
        }
        else if (FstrEncodeType == "GB2312")
        {
            if ((nHigh8Bits >= 0xa1) && (nHigh8Bits <= 0xfe) && (nLower8Bits >= 0xa1) && (nLower8Bits <= 0xfe))
            {
                nEncodeIndex = (nHigh8Bits - 0xa1) * 0x5e + (nLower8Bits - 0xa1);  //0x5e = 0xfe - 0xa1 + 1
                boolBFLBitmap = true;
            }
        }
        else if (FstrEncodeType == "GB2312-16")  //简化版的GB2312字库，不含1-15区（符号、数字区），直接从第16区（汉字区）开始
        {
            if ((nHigh8Bits >= 0xb0) && (nHigh8Bits <= 0xfe) && (nLower8Bits >= 0xa1) && (nLower8Bits <= 0xfe))
            {
                nEncodeIndex = (nHigh8Bits - 0xb0) * 0x5e + (nLower8Bits - 0xa1);  //0x5e = 0xfe - 0xa1 + 1
                boolBFLBitmap = true;
            }
        }
        else
        {
            boolBFLBitmap = true;
        }

        if (boolBFLBitmap)
        {
            FBFLSearch.Width = FBFLHead.Height;
            uint16_t nBFLBitmapSize = ((FBFLSearch.Width + 7) / 8) * FBFLHead.Height;
            FBFLSearch.OffAddr = nEncodeIndex * nBFLBitmapSize + sizeof(FBFLHead);
        }
    }
    //MBCS非等宽
    else if (nBFLType == 3)
    {
        while (true)
        {
            //获取字体文件检索表
            if (! FontFile.seek(nCharCode * sizeof(FBFLSearch) + sizeof(FBFLHead))) break;
            if (FontFile.read(&FBFLSearch, sizeof(FBFLSearch)) != sizeof(FBFLSearch)) break;
            boolBFLBitmap = true;
            break;
        }
    }

    return boolBFLBitmap;
}
//---------------------------------------------------------------------------
void InkFont::DrawCharBitmap(const int16_t nXpos, const int16_t nYpos, const byte *BFLBitmap, const uint16_t nSize, const uint16_t nWidth, const byte nHeight, const boolean boolReverse, const boolean boolTransparent, const uint8_t nDirection, const uint8_t nWidthScale, const uint8_t nHeightScale)
{
    uint16_t nByteIndex = 0;
    int8_t   nBitIndex  = 7;
    byte     nColor;
    int16_t  nX, nY;

    for (byte nHeightIndex = 0; nHeightIndex < nHeight; nHeightIndex++)
    {
        for (uint16_t nWidthIndex = 0; nWidthIndex < nWidth; nWidthIndex++)
        {
            nColor = BFLBitmap[nByteIndex];
            nColor <<= (7 - nBitIndex);
            nColor >>= 7;
            if (boolReverse) nColor = 1 - nColor;

            if ((nColor == 1) || (! boolTransparent))
            {
                for (uint8_t nWidthScaleIndex = 0; nWidthScaleIndex < nWidthScale; nWidthScaleIndex++)
                {
                    for (uint8_t nHeightScaleIndex = 0; nHeightScaleIndex < nHeightScale; nHeightScaleIndex++)
                    {
                        if (nDirection == 1)
                        {
                            nX = nXpos - nHeightIndex * nHeightScale - nHeightScaleIndex;
                            nY = nYpos + nWidthIndex * nWidthScale + nWidthScaleIndex;
                        }
                        else if (nDirection == 2)
                        {
                            nX = nXpos - nWidthIndex * nWidthScale - nWidthScaleIndex;
                            nY = nYpos - nHeightIndex * nHeightScale - nHeightScaleIndex;
                        }
                        else if (nDirection == 3)
                        {
                            nX = nXpos + nHeightIndex * nHeightScale + nHeightScaleIndex;
                            nY = nYpos - nWidthIndex * nWidthScale - nWidthScaleIndex;
                        }
                        else
                        {
                            nX = nXpos + nWidthIndex * nWidthScale + nWidthScaleIndex;
                            nY = nYpos + nHeightIndex * nHeightScale + nHeightScaleIndex;
                        }

                        DrawPixel(nX, nY, nColor);
                    }
                }
            }

            nBitIndex--;
            if (nBitIndex < 0)
            {
                nByteIndex++;
                nBitIndex = 7;
            }
        }
    }
}
//---------------------------------------------------------------------------
uint32_t InkFont::GetCharCode(const char *pcText, uint16_t &nOffset, FsFile& CPFile)
{
    if      (FnBFLType == 1) return UTF8_to_Unicode(pcText, nOffset);
    else if (FnBFLType == 2) return UTF8_to_MBCS(pcText, nOffset, CPFile);
    else if (FnBFLType == 3) return UTF8_to_MBCS(pcText, nOffset, CPFile);
    else    {nOffset = 1;  return (uint8_t)*pcText;}
}
//---------------------------------------------------------------------------
uint32_t InkFont::UTF8_to_Unicode(const char *pcText, uint16_t &nOffset)
{
    uint8_t nUtf8, nFlagBitNum, nByteNum;
    uint32_t nUnicode;

    //初始化字符串的偏移量
    nOffset = 0;

    //获取首字节
    nUtf8 = *pcText;

    //判断首字节，获取标志位占位位数、后续字节数
    if      ((nUtf8 & 0x80) == 0x00) {nFlagBitNum = 1; nByteNum = 1;}
    else if ((nUtf8 & 0xe0) == 0xc0) {nFlagBitNum = 3; nByteNum = 2;}
    else if ((nUtf8 & 0xf0) == 0xe0) {nFlagBitNum = 4; nByteNum = 3;}
    else if ((nUtf8 & 0xf8) == 0xf0) {nFlagBitNum = 5; nByteNum = 4;}
    else if ((nUtf8 & 0xfc) == 0xf8) {nFlagBitNum = 6; nByteNum = 5;}
    else if ((nUtf8 & 0xfe) == 0xfc) {nFlagBitNum = 7; nByteNum = 6;}
    else                             {nFlagBitNum = 1; nByteNum = 1;}

    //获取去除标志位的首字节
    nUtf8 <<= nFlagBitNum; nUtf8 >>= nFlagBitNum;
    nUnicode = (uint32_t)nUtf8;

    //字符串指针+1
    pcText++;
    nOffset++;

    //获取累计的去除标志位的后续字节
    for (uint8_t nByteIndex = 1; nByteIndex < nByteNum; nByteIndex++)
    {
        //获取后续字节
        nUtf8 = *pcText;

        //判断后续字节满足10xxxxxx格式
        if ((nUtf8 & 0xc0) == 0x80)
        {
            //获取累计的去除标志位的后续字节
            nUtf8 <<= 2; nUtf8 >>= 2;
            nUnicode *= 0x40;
            nUnicode += (uint32_t)nUtf8;

            //字符串指针+1
            pcText++;
            nOffset++;
        }
        else
        {
            break;
        }
    }

    return nUnicode;
}
//---------------------------------------------------------------------------
uint32_t InkFont::UTF8_to_MBCS(const char *pcText, uint16_t &nOffset, FsFile& CPFile)
{
    uint32_t nUnicode = UTF8_to_Unicode(pcText, nOffset);
    uint32_t nMBCS = nUnicode;

    if (CPFile.seek(nUnicode * 14 + 2))
    {
        char cMBCS[4];
        if (CPFile.read(cMBCS, 4) == 4) nMBCS = (uint32_t)HEXc_to_Int(cMBCS[0]) * 0x1000 + (uint32_t)HEXc_to_Int(cMBCS[1]) * 0x100 + (uint32_t)HEXc_to_Int(cMBCS[2]) * 0x10 + (uint32_t)HEXc_to_Int(cMBCS[3]);
    }

    return nMBCS;
}
//---------------------------------------------------------------------------
uint8_t InkFont::HEXc_to_Int(const char c)
{
    if (c == '0') return 0x0;
    if (c == '1') return 0x1;
    if (c == '2') return 0x2;
    if (c == '3') return 0x3;
    if (c == '4') return 0x4;
    if (c == '5') return 0x5;
    if (c == '6') return 0x6;
    if (c == '7') return 0x7;
    if (c == '8') return 0x8;
    if (c == '9') return 0x9;
    if ((c == 'a') || (c == 'A')) return 0xa;
    if ((c == 'b') || (c == 'B')) return 0xb;
    if ((c == 'c') || (c == 'C')) return 0xc;
    if ((c == 'd') || (c == 'D')) return 0xd;
    if ((c == 'e') || (c == 'E')) return 0xe;
    if ((c == 'f') || (c == 'F')) return 0xf;
    return 0x0;
}
//---------------------------------------------------------------------------
boolean InkFont::ProcessWordWrap(int16_t &nXpos, int16_t &nYpos, const int16_t nXpos0, const int16_t nYpos0, const uint8_t nAlign, const uint8_t nDirection, const uint8_t nWidthScale, const uint8_t nHeightScale, const int16_t nDisplayWidth, const int16_t nDisplayHeight, const boolean boolForce)
{
    boolean boolProcess = false;

    if (nDirection == 1)
    {
        if (boolForce || (nYpos + FBFLSearch.Width * nWidthScale > nDisplayHeight))
        {
            if (nAlign == 1) nYpos = nYpos0; else nYpos = 0;
            nXpos -= (FBFLHead.Height * nHeightScale);
            boolProcess = true;
        }
    }
    else if (nDirection == 2)
    {
        if (boolForce || (nXpos - FBFLSearch.Width * nWidthScale < -1))
        {
            if (nAlign == 1) nXpos = nXpos0; else nXpos = nDisplayWidth - 1;
            nYpos -= (FBFLHead.Height * nHeightScale);
            boolProcess = true;
        }
    }
    else if (nDirection == 3)
    {
        if (boolForce || (nYpos - FBFLSearch.Width * nWidthScale < -1))
        {
            if (nAlign == 1) nYpos = nYpos0; else nYpos = nDisplayHeight - 1;
            nXpos += (FBFLHead.Height * nHeightScale);
            boolProcess = true;
        }
    }
    else
    {
        if (boolForce || (nXpos + FBFLSearch.Width * nWidthScale > nDisplayWidth))
        {
            if (nAlign == 1) nXpos = nXpos0; else nXpos = 0;
            nYpos += (FBFLHead.Height * nHeightScale);
            boolProcess = true;
        }
    }

    return boolProcess;
}
//---------------------------------------------------------------------------
boolean InkFont::ProcessRange(int16_t &nXpos, int16_t &nYpos, const uint8_t nDirection, const uint8_t nWidthScale, const uint8_t nHeightScale, const int16_t nDisplayWidth, const int16_t nDisplayHeight, boolean &boolComeBack)
{
    boolean boolRange = false;
    boolComeBack = true;

    if (nDirection == 1)
    {
        if ((nYpos >= nDisplayHeight) || (nXpos < 0)) boolComeBack = false;
        else if ((nYpos + FBFLSearch.Width * nWidthScale > 0) && (nXpos - FBFLHead.Height * nHeightScale < nDisplayWidth - 1)) boolRange = true;
    }
    else if (nDirection == 2)
    {
        if ((nXpos < 0) || (nYpos < 0)) boolComeBack = false;
        else if ((nXpos - FBFLSearch.Width * nWidthScale < nDisplayWidth - 1) && (nYpos - FBFLHead.Height * nHeightScale < nDisplayHeight - 1)) boolRange = true;
    }
    else if (nDirection == 3)
    {
        if ((nYpos < 0) || (nXpos >= nDisplayWidth)) boolComeBack = false;
        else if ((nYpos - FBFLSearch.Width * nWidthScale < nDisplayHeight - 1) && (nXpos + FBFLHead.Height * nHeightScale > 0)) boolRange = true;
    }
    else
    {
        if ((nXpos >= nDisplayWidth) || (nYpos >= nDisplayHeight)) boolComeBack = false;
        else if ((nXpos + FBFLSearch.Width * nWidthScale > 0) && (nYpos + FBFLHead.Height * nHeightScale > 0)) boolRange = true;
    }

    return boolRange;
}
//---------------------------------------------------------------------------
void InkFont::ProcessCursorStep(int16_t &nXpos, int16_t &nYpos, const uint8_t nDirection, const uint8_t nWidthScale)
{
    if      (nDirection == 1) nYpos += (FBFLSearch.Width * nWidthScale);
    else if (nDirection == 2) nXpos -= (FBFLSearch.Width * nWidthScale);
    else if (nDirection == 3) nYpos -= (FBFLSearch.Width * nWidthScale);
    else                      nXpos += (FBFLSearch.Width * nWidthScale);
}
//---------------------------------------------------------------------------
boolean InkFont::SetFont(const String strFontFilename, const int nCodePage, const String strEncodeType, const String strAsciiFontFilename)
{
    FboolFontInit = false;
    FstrFontFilename = strFontFilename;
    FnCodePage = nCodePage;
    FstrCPFilename = "";
    FstrEncodeType = strEncodeType;
    FstrAsciiFontFilename = strAsciiFontFilename;

    //判断字体文件是否存在
    if (FpSDCard->exists(FstrFontFilename))
    {
        FsFile FontFile = FpSDCard->open(FstrFontFilename/*, FILE_READ*/);

        //获取字体文件头信息
        if (FontFile.read(&FBFLHead, sizeof(FBFLHead)) == sizeof(FBFLHead))
        {
            if ((FBFLHead.Flag[0] == 'U') && (FBFLHead.Flag[1] == 'F') && (FBFLHead.Flag[2] == 'L'))
            {
                FnBFLType = 1;
                FboolFontInit = true;
            }
            else if ((FBFLHead.Flag[0] == 'M') && (FBFLHead.Flag[1] == 'F') && (FBFLHead.Flag[2] == 'L'))
            {
                //获取代码页文件名
                int nIndex = FstrFontFilename.lastIndexOf("/");
                if (nIndex >= 0) FstrCPFilename = FstrFontFilename.substring(0, nIndex + 1);
                FstrCPFilename = FstrCPFilename + "CP" + String(FnCodePage, DEC) + ".txt";

                //判断代码页文件是否存在
                if (FpSDCard->exists(FstrCPFilename))
                {
                    if (FBFLHead.Section == 0) FnBFLType = 2; else FnBFLType = 3;
                    FboolFontInit = true;
                }
            }
        }

        FontFile.close();
    }

    return FboolFontInit;
}
//---------------------------------------------------------------------------
uint16_t InkFont::DrawText(int16_t nXpos, int16_t nYpos, const char *pcText, const boolean boolAutoWordWrap, const uint8_t nWordWrapAlign, const boolean boolReverse, const boolean boolTransparent, const uint8_t nDirection, const boolean boolPosBottom, const uint8_t nWidthScale, const uint8_t nHeightScale, const boolean boolNoDraw)
{
    uint16_t nTextWidth = 0;

    //判断是否设置过字体文件
    if (! FboolFontInit) return nTextWidth;
    FsFile FontFile = FpSDCard->open(FstrFontFilename/*, FILE_READ*/);
    uint16_t nOffset = 1;

    //打开字体文件对应的代码页文件
    boolean boolCPFile = ((FnBFLType == 2) || (FnBFLType == 3));
    FsFile CPFile;
    if (boolCPFile) CPFile = FpSDCard->open(FstrCPFilename/*, FILE_READ*/);

    //打开字体文件对应的常规Ascii字体文件，常规Ascii字体文件必须为MBCS非等宽编码、和字体文件相同的像素高度
    boolean boolAsciiFontFile = ((FnBFLType == 2) && FpSDCard->exists(FstrAsciiFontFilename));
    FsFile AsciiFontFile;
    if (boolAsciiFontFile) AsciiFontFile = FpSDCard->open(FstrAsciiFontFilename/*, FILE_READ*/);

    //获取屏幕尺寸
    int16_t nDisplayWidth = GetDisplayWidth();
    int16_t nDisplayHeight = GetDisplayHeight();

    //判断是否需要转换原点坐标
    if (boolPosBottom)
    {
        if      (nDirection == 1) nXpos += (FBFLHead.Height * nHeightScale - 1);
        else if (nDirection == 2) nYpos += (FBFLHead.Height * nHeightScale - 1);
        else if (nDirection == 3) nXpos -= (FBFLHead.Height * nHeightScale - 1);
        else                      nYpos -= (FBFLHead.Height * nHeightScale - 1);
    }

    //保存起始的原点坐标
    int16_t nXpos0 = nXpos;
    int16_t nYpos0 = nYpos;

    //循环处理每一个字符
    while (*pcText)
    {
        FsFile DrawFontFile = FontFile;
        uint32_t nCharCode = GetCharCode(pcText, nOffset, CPFile);

        //处理换行符
        if (nCharCode == '\n')
        {
            ProcessWordWrap(nXpos, nYpos, nXpos0, nYpos0, nWordWrapAlign, nDirection, nWidthScale, nHeightScale, nDisplayWidth, nDisplayHeight, true);
        }
        else
        {
            //获取字体文件点阵信息偏移地址
            boolean boolGetCharBitmap = GetCharBitmap(nCharCode, FnBFLType, DrawFontFile);

            //如果没有获取到字体文件点阵信息偏移地址，则获取字体文件对应的常规Ascii字体文件点阵信息偏移地址（仅针对MBCS等宽编码的字库文件），常规Ascii字体文件必须为MBCS非等宽编码、和字体文件相同的像素高度
            if ((! boolGetCharBitmap) && boolAsciiFontFile)
            {
                DrawFontFile = AsciiFontFile;
                boolGetCharBitmap = GetCharBitmap(nCharCode, 3, DrawFontFile);
            }

            if (boolGetCharBitmap)
            {
                nTextWidth += (FBFLSearch.Width * nWidthScale);

                if ((! boolNoDraw) && DrawFontFile.seek(FBFLSearch.OffAddr))
                {
                    uint16_t nBFLBitmapSize = ((FBFLSearch.Width + 7) / 8) * FBFLHead.Height;
                    byte BFLBitmap[nBFLBitmapSize];

                    //获取字体文件点阵信息
                    if (DrawFontFile.read(BFLBitmap, nBFLBitmapSize) == nBFLBitmapSize)
                    {
                        //判断是否需要换行
                        if (boolAutoWordWrap) ProcessWordWrap(nXpos, nYpos, nXpos0, nYpos0, nWordWrapAlign, nDirection, nWidthScale, nHeightScale, nDisplayWidth, nDisplayHeight, false);

                        //判断是否超出显示范围
                        boolean boolComeBack;
                        if (ProcessRange(nXpos, nYpos, nDirection, nWidthScale, nHeightScale, nDisplayWidth, nDisplayHeight, boolComeBack))
                        {
                            //绘制点阵
                            DrawCharBitmap(nXpos, nYpos, BFLBitmap, nBFLBitmapSize, FBFLSearch.Width, FBFLHead.Height, boolReverse, boolTransparent, nDirection, nWidthScale, nHeightScale);
                        }
                        else
                        {
                            //如果超出显示范围，并且后续字符不会再出现在显示范围内，则放弃处理后续字符
                            if (! boolComeBack) break;
                        }

                        //光标步进
                        ProcessCursorStep(nXpos, nYpos, nDirection, nWidthScale);
                    }
                }
            }
        }

        pcText += nOffset;
    }

    if (boolAsciiFontFile) AsciiFontFile.close();
    if (boolCPFile) CPFile.close();
    FontFile.close();

    return nTextWidth;
}

void InkFont::DrawGlyph(int16_t nXpos, int16_t nYpos, uint16_t hex,const boolean boolAutoWordWrap, const uint8_t nWordWrapAlign, const boolean boolReverse, const boolean boolTransparent, const uint8_t nDirection, const boolean boolPosBottom, const uint8_t nWidthScale, const uint8_t nHeightScale, const boolean boolNoDraw)
{
  uint16_t nTextWidth = 0;

  //保存起始的原点坐标
  int16_t nXpos0 = nXpos;
  int16_t nYpos0 = nYpos;

  //获取屏幕尺寸
  int16_t nDisplayWidth = GetDisplayWidth();
  int16_t nDisplayHeight = GetDisplayHeight();

  //判断是否需要转换原点坐标
  if (boolPosBottom)
  {
      if      (nDirection == 1) nXpos += (FBFLHead.Height * nHeightScale - 1);
      else if (nDirection == 2) nYpos += (FBFLHead.Height * nHeightScale - 1);
      else if (nDirection == 3) nXpos -= (FBFLHead.Height * nHeightScale - 1);
      else                      nYpos -= (FBFLHead.Height * nHeightScale - 1);
  }

  FsFile FontFile = FpSDCard->open(FstrFontFilename/*, FILE_READ*/);
  FsFile DrawFontFile = FontFile;

  //获取字体文件点阵信息偏移地址
  boolean boolGetCharBitmap = GetCharBitmap(hex, 1, DrawFontFile);

  if (boolGetCharBitmap)
  {
      nTextWidth += (FBFLSearch.Width * nWidthScale);

      if ((! boolNoDraw) && DrawFontFile.seek(FBFLSearch.OffAddr))
      {
          uint16_t nBFLBitmapSize = ((FBFLSearch.Width + 7) / 8) * FBFLHead.Height;
          byte BFLBitmap[nBFLBitmapSize];

          //获取字体文件点阵信息
          if (DrawFontFile.read(BFLBitmap, nBFLBitmapSize) == nBFLBitmapSize)
          {
              //判断是否需要换行
              if (boolAutoWordWrap) ProcessWordWrap(nXpos, nYpos, nXpos0, nYpos0, nWordWrapAlign, nDirection, nWidthScale, nHeightScale, nDisplayWidth, nDisplayHeight, false);

              //判断是否超出显示范围
              boolean boolComeBack;
              if (ProcessRange(nXpos, nYpos, nDirection, nWidthScale, nHeightScale, nDisplayWidth, nDisplayHeight, boolComeBack))
              {
                  //绘制点阵
                  DrawCharBitmap(nXpos, nYpos, BFLBitmap, nBFLBitmapSize, FBFLSearch.Width, FBFLHead.Height, boolReverse, boolTransparent, nDirection, nWidthScale, nHeightScale);
              }
              else
              {
                  //如果超出显示范围，并且后续字符不会再出现在显示范围内，则放弃处理后续字符
                  if (! boolComeBack) return;
              }

              //光标步进
              ProcessCursorStep(nXpos, nYpos, nDirection, nWidthScale);
          }
      }
  }

  FontFile.close();

}
//---------------------------------------------------------------------------
uint16_t InkFont::GetTextWidth(const char *pcText)
{
    return DrawText(0, 0, pcText, true, false, false, 0, false, 1, 1, true);
}
//---------------------------------------------------------------------------
byte InkFont::GetCharHeight()
{
    //判断是否设置过字体文件
    if (! FboolFontInit) return 0;
    return FBFLHead.Height;
}
//---------------------------------------------------------------------------
