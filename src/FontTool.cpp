/*
 * @Date: 2023-01-14 23:42:24
 * @LastEditors: AaronChu
 * @LastEditTime: 2023-02-07 09:20:48
 */
#include "Arduino.h"
#include "FontTool.h"

unsigned char fontbuff[512];

FontTool::FontTool()
{
  FpSDCard = NULL;
  FpScreen = NULL;
}

/**
 * @description: 文字工具初始化
 * @param {SdFs} *pSDCard 文件系统实例
 * @param {PAINT} *pScreen 屏幕实例
 */
void FontTool::Init(SdFs *pSDCard, PAINT *pScreen)
{
  FpSDCard = pSDCard;
  FpScreen = pScreen;
}

/**
 * @description: 字符类型转换 utf8转换成unicode
 * @param {char} *pText 单个字符
 * @return {uint32_t} unicode unicode编码
 */
uint32_t UTF82Unicode(const char *pText)
{
  uint8_t utf8, bitNum, byteNum;
  uint32_t unicode;
  // 初始化字符串的偏移量
  uint16_t offset = 0;
  // 获取首字节
  utf8 = *pText;
  // 判断首字节，获取标志位占位位数、后续字节数
  if ((utf8 & 0x80) == 0x00)
  {
    bitNum = 1;
    byteNum = 1;
  }
  else if ((utf8 & 0xe0) == 0xc0)
  {
    bitNum = 3;
    byteNum = 2;
  }
  else if ((utf8 & 0xf0) == 0xe0)
  {
    bitNum = 4;
    byteNum = 3;
  }
  else if ((utf8 & 0xf8) == 0xf0)
  {
    bitNum = 5;
    byteNum = 4;
  }
  else if ((utf8 & 0xfc) == 0xf8)
  {
    bitNum = 6;
    byteNum = 5;
  }
  else if ((utf8 & 0xfe) == 0xfc)
  {
    bitNum = 7;
    byteNum = 6;
  }
  else
  {
    bitNum = 1;
    byteNum = 1;
  }
  // 获取去除标志位的首字节
  utf8 <<= bitNum;
  utf8 >>= bitNum;
  unicode = (uint32_t)utf8;
  // 字符串指针+1
  pText++;
  offset++;
  // 获取累计的去除标志位的后续字节
  for (uint8_t byteIndex = 1; byteIndex < byteNum; byteIndex++)
  {
    // 获取后续字节
    utf8 = *pText;
    // 判断后续字节满足10xxxxxx格式
    if ((utf8 & 0xc0) == 0x80)
    {
      // 获取累计的去除标志位的后续字节
      utf8 <<= 2;
      utf8 >>= 2;
      unicode *= 0x40;
      unicode += (uint32_t)utf8;
      // 字符串指针+1
      pText++;
      offset++;
    }
    else
    {
      break;
    }
  }
  return unicode;
}

/**
 * @description: 从bin文件中读取N个字节的数据
 * @param {uint8_t} addr 偏移地址
 * @param {uint8_t} bytes 字节数
 * @param {FsFile} file bin文件
 * @return {unsigned char} buff
 */
unsigned char *FontTool::GetByteDataFromBin(uint32_t addr, uint16_t bytes, String FontPath)
{
  FsFile file = FpSDCard->open(FontPath /*, FILE_READ*/); // 打开文件
  file.seek(addr);                                        // 设置读取位置
  file.read(fontbuff, bytes);                             // 读取指定长度的数据到buff中
  file.close();
  return fontbuff;
}

/**
 * @description: 显示一个字符
 * @param {uint16_t} Xstart 字符x坐标
 * @param {uint16_t} Ystart 字符y坐标
 * @param {char} *text 字符内容
 * @param {uint16_t} size 字符大小
 * @param {uint16_t} bytes 字符所占字节数
 * @param {bool} inversion 是否反相
 */
void FontTool::DrawChar(uint16_t Xstart, uint16_t Ystart, const char *text, uint16_t size, uint16_t bytes, String type, bool inversion)
{
  String path = "/fonts/";
  String FontPath = path + type + String(size) + ".bin"; // 字体文件路径拼接
  // 完整路径 "/fonts/songti16.bin"，此处文件地址根据实际文件路径拼接好
  // Serial.print("FontPath:");
  // Serial.println(FontPath);
  uint32_t unicode_letter = UTF82Unicode(text);                // utf8转unicode
  uint32_t unicode_offset = (unicode_letter - 0x0000) * bytes; // 该文字的偏移地址，减去0x0000为取字模时定义的最小unicode范围
  // Serial.println(unicode_offset);
  const unsigned char *ptr = GetByteDataFromBin(unicode_offset, bytes, FontPath); // 获取点阵数据
  for (int Page = 0; Page < size; Page++)
  {
    for (int Column = 0; Column < size; Column++)
    {
      if (inversion == true)
      {
        if (*ptr & (0x80 >> (Column % 8)))
        {
          // 更换屏幕驱动需要替换此处的打点函数
          Paint_SetPixel(Xstart + Column, Ystart + Page, WHITE);
        }
        else
        {
          Paint_SetPixel(Xstart + Column, Ystart + Page, BLACK);
        }
        // Serial.println("反相");
      }
      else
      {
        if (*ptr & (0x80 >> (Column % 8)))
        {
          Paint_SetPixel(Xstart + Column, Ystart + Page, BLACK);
        }
      }
      if (Column % 8 == 7)
        ptr++;
    }
    if (size % 8 != 0)
      ptr++;
  }
}

/**
 * @description: 显示字符串
 * @param {uint16_t} Xstart 字符x坐标
 * @param {uint16_t} Ystart 字符y坐标
 * @param {char} *pString 字符串
 * @param {uint16_t} size 字符大小
 * @param {bool} inversion 是否反相
 */
void FontTool::DrawString(uint16_t Xstart, uint16_t Ystart, const char *pString, uint16_t size, String type, bool inversion)
{
  uint16_t Xpoint = Xstart;
  uint16_t Ypoint = Ystart;
  uint16_t bytes;                                         // 字节数
  uint8_t width;                                          // 宽度
  uint8_t height = (size / 8) % 2 != 0 ? size + 2 : size; // 高度(24号字体高度不够)
  uint8_t len;                                            // unicode跳过的字节数
  if (Xstart > FpScreen->Width || Ystart > FpScreen->Height)
  {
    Serial.print("Paint_DrawString_EN Input exceeds the normal display range\r\n");
    return;
  }

  while (*pString != 0)
  {
    // 字符类型判断
    // ASCII字符
    if (*pString <= 0x007f)
    {
      // 由于取的字模全是等宽等高的数据，所以字符数量跟中文保持一致，偏移地址的计算也一致
      len = 1;
      bytes = size * size / 8;
      width = size / 2;
    }
    // 全角字符
    else
    {
      len = 3;
      bytes = size * size / 8;
      width = size;
    }
    // 超出区域换行
    if ((Xpoint + width) > Paint.Width)
    {
      Xpoint = Xstart;
      Ypoint += height + 4;
    }
    if ((Ypoint + height) > Paint.Height)
    {
      Xpoint = Xstart;
      Ypoint = Ystart;
    }

    DrawChar(Xpoint, Ypoint, pString, size, bytes, type, inversion);

    // The next character of the address
    pString += len;

    // The next word of the abscissa increases the font of the broadband
    Xpoint += width;
  }
}
