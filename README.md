<!--
 * @Date: 2021-09-22 14:40:26
 * @LastEditors: AaronChu
 * @LastEditTime: 2023-02-07 11:35:39
-->

# EPaperFontTool

![](./extra/img/img.gif)

关于
基于 SdFat 和微雪墨水屏驱动的内存卡字库驱动，文件系统和屏幕的驱动都可以更换，只需替换少量代码即可。

## 使用步骤
1. PlatformIO平台只需要放入lib文件夹即可，arduino平台下放到跟.ino文件同级目录下。

2. 初始化屏幕和挂载文件系统

3. 随后初始化字体工具，需要将屏幕和文件系统的类传入。
```c++
FontTool fontTool;

void setup()
{
  fontTool.Init(&sd, &Paint);
}

```

## 已取模字体列表

| 名称      | 字体 | 大小  | unicode 编码范围 | 链接                                      |
| --------- | ---- | ----- | ---------------- | ----------------------------------------- |
| songti16  | 宋体 | 16x16 | 0x0000-0xFFFF    | [宋体 16 号](./extra/fonts/songti16.bin)  |
| songti24  | 宋体 | 24x24 | 0x0000-0xFFFF    | [宋体 24 号](./extra/fonts/songti24.bin)  |
| songti32  | 宋体 | 32x32 | 0x0000-0xFFFF    | [宋体 32 号](./extra/fonts/songti32.bin)  |
| songti64  | 宋体 | 64x64 | 0x0000-0xFFFF    | [宋体 64 号](./extra/fonts/songti64.bin)  |
| kaiti16   | 楷体 | 16x16 | 0x0000-0xFFFF    | [楷体 16 号](./extra/fonts/kaiti16.bin)   |
| kaiti24   | 楷体 | 24x24 | 0x0000-0xFFFF    | [楷体 24 号](./extra/fonts/kaiti24.bin)   |
| kaiti32   | 楷体 | 32x32 | 0x0000-0xFFFF    | [楷体 32 号](./extra/fonts/kaiti32.bin)   |
| kaiti64   | 楷体 | 64x64 | 0x0000-0xFFFF    | [楷体 64 号](./extra/fonts/kaiti64.bin)   |
| youyuan16 | 幼圆 | 16x16 | 0x0000-0xFFFF    | [幼圆 16 号](./extra/fonts/youyuan16.bin) |
| youyuan24 | 幼圆 | 24x24 | 0x0000-0xFFFF    | [幼圆 24 号](./extra/fonts/youyuan24.bin) |
| youyuan32 | 幼圆 | 32x32 | 0x0000-0xFFFF    | [幼圆 32 号](./extra/fonts/youyuan32.bin) |
| youyuan64 | 幼圆 | 64x64 | 0x0000-0xFFFF    | [幼圆 64 号](./extra/fonts/youyuan64.bin) |
| caiyun32  | 彩云 | 32x32 | 0x0000-0xFFFF    | [彩云 32 号](./extra/fonts/caiyun32.bin)  |
| caiyun64  | 彩云 | 64x64 | 0x0000-0xFFFF    | [彩云 64 号](./extra/fonts/caiyun64.bin)  |

## 维护者

[@AaronChuzb](https://github.com/AaronChuzb)。

## 使用许可

[MIT](LICENSE) © Aaron Chu
