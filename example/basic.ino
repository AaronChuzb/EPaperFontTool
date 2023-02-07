/*
 * @Date: 2023-02-07 11:36:22
 * @LastEditors: AaronChu
 * @LastEditTime: 2023-02-07 11:47:47
 */
#include "EPD.h"
#include "SdFat.h"
#include "FontTool.h"
SdFs sd;
FontTool fontTool;
// 文件系统挂载
void SdFat_Init()
{
  if (!sd.begin(SdSpiConfig(5, DEDICATED_SPI, 20000000)))
  {
    Serial.println("文件系统挂载失败！");
  }
  else
  {
    Serial.println("文件系统挂载成功！");
  }
}

void setup()
{
  DEV_Module_Init();
  EPD_Init();
  // 先初始化文件系统和屏幕再初始化字库
  fontTool.Init(&sd, &Paint); // Paint来自微雪墨水屏驱动全局变量
  uint16_t Imagesize = ((EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1)) * EPD_HEIGHT;
  if ((BlackImage = (uint8_t *)malloc(Imagesize)) == NULL)
  {
    printf("Failed to apply for black memory...\r\n");
    while (1)
      ;
  }
  printf("Paint_NewImage\r\n");
  Paint_NewImage(BlackImage, EPD_WIDTH, EPD_HEIGHT, 0, WHITE);
  Paint_SelectImage(BlackImage);
  Paint_Clear(WHITE);
  fontTool.DrawString(0, 0, "1234567890 Made in China, Power By: AaronChu unicode字符集，こんにちは世界", 16);
  fontTool.DrawString(0, 100, "24号字体", 24, "kaiti");
  fontTool.DrawString(0, 130, "32", 32, "caiyun");
  fontTool.DrawString(0, 180, "反相显示", 16, "songti",true);
  EPD_Display(BlackImage);
  printf("Goto Sleep...\r\n");
  EPD_Sleep();
  free(BlackImage);
  BlackImage = NULL;
}

void loop()
{

}