// #include "lcd.h"

// namespace LCD {
//     void lcd::lcd(  u_int8_t rs, u_int8_t rw, u_int8_t en, 
//                     u_int8_t d0, u_int8_t d1, u_int8_t d2, u_int8_t d3, 
//                     u_int8_t d4, u_int8_t d5, u_int8_t d6, u_int8_t d7) {
//         return init(rs, rw, en, d0, d1, d2, d3, d4, d5, d6, d7);
//     }

//     void lcd::init( u_int8_t rs, u_int8_t rw, u_int8_t en, 
//                     u_int8_t d0, u_int8_t d1, u_int8_t d2, u_int8_t d3, 
//                     u_int8_t d4, u_int8_t d5, u_int8_t d6, u_int8_t d7) {
//         _rs_pin = rs;
//         _rw_pin = rw;
//         _en_pin = en;
//         _data_pins[0] = d0;
//         _data_pins[1] = d1;
//         _data_pins[2] = d2;
//         _data_pins[3] = d3; 
//         _data_pins[4] = d4;
//         _data_pins[5] = d5;
//         _data_pins[6] = d6;
//         _data_pins[7] = d7; 

//         if(d4 && d5 && d6 && d7)
//             _displayfunction = LCD_8BIT_MODE | LCD_1LINE | LCD_5x8DOTS;
//         else
//             _displayfunction = LCD_4BIT_MODE | LCD_1LINE | LCD_5x8DOTS;
//     }

//     void lcd::begin(u_int8_t cols, u_int8_t lines, u_int8_t dotsize) {
//         if (lines > 1)
//             _displayfunction |= LCD_2LINE;
//         _numlines = lines;

//         setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

//         if (dotsize != LCD_5x8DOTS && lines == 1)
//             _displayfunction != LCD_5x10DOTS;

        
//     }
// }