/*
 * @Date: 2023-01-14 23:42:33
 * @LastEditors: AaronChu
 * @LastEditTime: 2023-02-07 09:15:20
 */

#ifndef FONTTOOl_H
#define FONTTOOl_H

#include <SPI.h>
#include "SdFat.h"
#include "EPD.h"
#include "GUI_Paint.h"

class FontTool
{
private:
  SdFs *FpSDCard;       // SD卡库
  PAINT *FpScreen;      // 屏幕类
public:
  FontTool();
  void Init(SdFs *pSDCard, PAINT *pScreen);
  unsigned char *GetByteDataFromBin(uint32_t addr, uint16_t size, String FontPath);
  void DrawChar(uint16_t Xstart, uint16_t Ystart, const char *text, uint16_t size,  uint16_t bytes, String type, bool inversion);
  void DrawString(uint16_t Xstart, uint16_t Ystart, const char * pString, uint16_t size, String type = "songti", bool inversion = false);

};

#endif
