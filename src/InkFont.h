//---------------------------------------------------------------------------
#ifndef InkFont_H
#define InkFont_H
//---------------------------------------------------------------------------
#include <SPI.h>
#include "SdFat.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//字体文件头结构
typedef struct tBFLHead
{
    byte        Flag[4];        //'U'('M')、'F'、'L'、X。U(M)FL：Unicode(MBCS) Font Library，X表示版本号，分高低4位。如0x12表示Ver 1.2
    uint32_t    Size;           //文件总长度
    byte        Section;        //Unicode编码：总共有几个数据段（也就是总共有几个段信息）；MBCS编码：是否包含检索表
    byte        Height;         //字体像素高度
    uint16_t    CpFlag;         //CodePage Flag：bit0 - bit13：每个bit位代表一个CodePage标志，如果是1，则表示当前CodePage被选定，否则为非选定
                                //CodePage从低到高顺序分别为：日文、简中、韩文、繁中、泰文、中欧、西里尔、西欧、希腊、土耳其文、希伯来文、阿拉伯文、波罗的海文、越南文
    byte        reserved[4];    //预留字节
};

//字体文件段信息结构
typedef struct tBFLSection
{
    uint16_t    First;          //本信息段中的第一个字符的Unicode编码
    uint16_t    Last;           //本信息段中的最后一个字符的Unicode编码
    uint32_t    OffAddr;        //本信息段对应的检索表在整个文件中的起始地址偏移量
};

//字体文件检索表结构
typedef struct tBFLSearch
{
    uint32_t    OffAddr : 26;   //低26bit，当前字符点阵数据在整个文件中的起始地址偏移量
    uint8_t     Width : 6;      //高6bit，当前字符点阵的像素宽度
};
//---------------------------------------------------------------------------
class InkFont
{
private:
    SdFs          *FpSDCard;      //SD卡库

    TFT_eSPI      *FpScreen;      //屏幕类

    uint16_t    bgColor;          //背景色
    uint16_t    fgColor;          //前景色

    boolean     FboolFontInit;          //字体文件初始化标志
    String      FstrFontFilename;       //字体文件名
    int         FnCodePage;             //字体文件对应的代码页（仅针对MBCS编码的字库文件）
    String      FstrCPFilename;         //字体文件对应的代码页文件名（仅针对MBCS编码的字库文件）
    String      FstrEncodeType;         //字体文件对应的编码类型（仅针对MBCS等宽编码的字库文件）
    String      FstrAsciiFontFilename;  //字体文件对应的常规Ascii字体文件名（仅针对MBCS等宽编码的字库文件），常规Ascii字体文件必须为MBCS非等宽编码、和字体文件相同的像素高度

    tBFLHead    FBFLHead;               //字体文件头
    tBFLSection FBFLSection;            //字体文件段信息
    tBFLSearch  FBFLSearch;             //字体文件检索表
    uint8_t     FnBFLType;              //字体文件的编码方式，1：Unicode；2：MBCS等宽；3：MBCS非等宽

    int16_t     GetDisplayWidth();
    int16_t     GetDisplayHeight();
    void        DrawPixel(const int16_t nX, const int16_t nY, const byte nColor);

    boolean     GetCharBitmap(const uint32_t nCharCode, const uint8_t nBFLType, FsFile& FontFile);
    void        DrawCharBitmap(const int16_t nXpos, const int16_t nYpos, const byte *BFLBitmap, const uint16_t nSize, const uint16_t nWidth, const byte nHeight, const boolean boolReverse, const boolean boolTransparent, const uint8_t nDirection, const uint8_t nWidthScale, const uint8_t nHeightScale);

    uint32_t    GetCharCode(const char *pcText, uint16_t &nOffset, FsFile& CPFile);
    uint32_t    UTF8_to_Unicode(const char *pcText, uint16_t &nOffset);
    uint32_t    UTF8_to_MBCS(const char *pcText, uint16_t &nOffset, FsFile& CPFile);
    uint8_t     HEXc_to_Int(const char c);

    boolean     ProcessWordWrap(int16_t &nXpos, int16_t &nYpos, const int16_t nXpos0, const int16_t nYpos0, const uint8_t nAlign, const uint8_t nDirection, const uint8_t nWidthScale, const uint8_t nHeightScale, const int16_t nDisplayWidth, const int16_t nDisplayHeight, const boolean boolForce);
    boolean     ProcessRange(int16_t &nXpos, int16_t &nYpos, const uint8_t nDirection, const uint8_t nWidthScale, const uint8_t nHeightScale, const int16_t nDisplayWidth, const int16_t nDisplayHeight, boolean &boolComeBack);
    void        ProcessCursorStep(int16_t &nXpos, int16_t &nYpos, const uint8_t nDirection, const uint8_t nWidthScale);

    

public:

        InkFont();

        InkFont(SdFs *pSDCard, TFT_eSPI *pScreen);
        void Init(SdFs *pSDCard, TFT_eSPI *pScreen);

    boolean     SetFont(const String strFontFilename, const int nCodePage = -1, const String strEncodeType = "", const String strAsciiFontFilename = "");
    uint16_t    DrawText(int16_t        nXpos,                      //X坐标
                         int16_t        nYpos,                      //Y坐标
                         const char     *pcText,                    //显示文字的内容
                         const boolean  boolAutoWordWrap = true,    //自动换行
                         const uint8_t  nWordWrapAlign = 0,         //换行后的对齐方式，0：和屏幕边缘对齐；1：和上一排文字对齐
                         const boolean  boolReverse = false,        //反色显示
                         const boolean  boolTransparent = false,    //透明背景
                         const uint8_t  nDirection = 0,             //旋转方向，0：0度；1：90度；2：180度；3：270度
                         const boolean  boolPosBottom = false,      //以左下角为原点坐标
                         const uint8_t  nWidthScale = 1,            //横向放大倍率
                         const uint8_t  nHeightScale = 1,           //纵向放大倍率
                         const boolean  boolNoDraw = false          //不显示文字，仅用于获取横向像素
                        );                                          //返回横向像素
    
    void DrawGlyph(int16_t , int16_t y, uint16_t hex,
                         const boolean  boolAutoWordWrap = false,    //自动换行
                         const uint8_t  nWordWrapAlign = 0,         //换行后的对齐方式，0：和屏幕边缘对齐；1：和上一排文字对齐
                         const boolean  boolReverse = false,        //反色显示
                         const boolean  boolTransparent = false,    //透明背景
                         const uint8_t  nDirection = 0,             //旋转方向，0：0度；1：90度；2：180度；3：270度
                         const boolean  boolPosBottom = false,      //以左下角为原点坐标
                         const uint8_t  nWidthScale = 1,            //横向放大倍率
                         const uint8_t  nHeightScale = 1,           //纵向放大倍率
                         const boolean  boolNoDraw = false          //不显示文字，仅用于获取横向像素);
    );

    void        setColor(uint16_t fg, uint16_t bg){bgColor=bg; fgColor=fg;} //设置前景色和背景色

    uint16_t    GetTextWidth(const char *pcText);
    byte        GetCharHeight();
};
//---------------------------------------------------------------------------
#endif
