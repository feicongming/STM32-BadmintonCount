// #include "Draw8.h"


// /******************************************************************************
//       函数说明：画数码管的水平笔画
//       入口数据：x,y   起始坐标
//                 lenght   笔画长度
//                 color   线的颜色
//       返回值：  无
// ******************************************************************************/
// void DrawHorizontal(u16 x, u16 y, u8 lenght, u16 color)
// {
//     u8 height, i;
//     height = lenght / HL_RAT;
//     for(i=0; i<height; i++)
//     {
//         LCD_DrawLine(x-i, y-i, x+lenght+i, y-i, color);
//     }
//     for(i=0; i<height; i++)
//     {
//         LCD_DrawLine(x-height+i, y-height-i, x+height+lenght-i, y-height+lenght-i, color);
//     }
// }


// /******************************************************************************
//       函数说明：画数码管的竖直笔画
//       入口数据：x,y   起始坐标
//                 lenght   笔画长度
//                 color   线的颜色
//       返回值：  无
// ******************************************************************************/
// void DrawVertical(u16 x, u16 y, u8 lenght, u16 color)
// {
//     u8 height, i;
//     height = lenght / HL_RAT;
//     for(i=0; i<height; i++)
//     {
//         LCD_DrawLine(x+i, y+i, x+i, y-lenght-i, color);
//     }
//     for(i=0; i<height; i++)
//     {
//         LCD_DrawLine(x+height+i, y+height-i, x+height+i, y-height-lenght+i, color);
//     }
// }


// /******************************************************************************
//       函数说明：画数码管                                             a
//       入口数据：x,y   起始坐标                                     *****
//                 num   数码管数字                              f  *     *  b
//                 lenght   笔画长度                                *     *  
//                 color   数码管颜色                                *****              
//       返回值：  无                                            e  *  g  *  c
//                                                                 *     *
//                                                                  *****
//                                                                    d
// ******************************************************************************/
// void Draw8(u16 x, u16 y, u8 num, u8 lenght, u16 color)
// {
//     u8 i, height;
//     height = lenght / HL_RAT;
//     u8 NumArray[10] = {0xfc, 0x60, 0xda, 0xf2, 0x4e, 0xb6, 0xbe, 0xe0, 0xfe, 0xf6};   // 用8bit对应笔画 ( a b c d e f g 0 )
//     u16 DeltaXY[7][2] = {{2*height, 0}, {lenght+2*height, 2*height}, \
//         {lenght+2*height, lenght+4*height}, {2*lenght, 2*lenght+2*height}, \
//         {0, lenght+4*height}, {0, 2*height}, {2*height, lenght+2*height}};

//     for(i=0; i<7; i++)
//     {
//         if( (1<<(7-i)) & NumArray[num] != 0 )
//         {
//             if( 0x6c & (1<<(7-i)) == 0 )    // 0x6c == 0110 1100  其中1表示竖直笔画，0表示水平笔画
//                 DrawHorizontal(x+DeltaXY[i][0], y+DeltaXY[i][1], lenght, color);
//             else
//                 DrawVertical(x+DeltaXY[i][0], y+DeltaXY[i][1], lenght, color);
//         }
//     }

// }
