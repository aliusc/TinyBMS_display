

/*
 *
 * webdings
 *
 * created with FontCreator
 * written by F. Maximilian Thiele
 *
 * http://www.apetech.de/fontCreator
 * me@apetech.de
 *
 * File Name           : webdings.h
 * Date                : 02.02.2017
 * Font size in bytes  : 64
 * Font width          : 10
 * Font height         : 7
 * Font first char     : 51
 * Font last char      : 53
 * Font used chars     : 2
 *
 * The font data are defined as
 *
 * struct _FONT_ {
 *     uint16_t   font_Size_in_Bytes_over_all_included_Size_it_self;
 *     uint8_t    font_Width_in_Pixel_for_fixed_drawing;
 *     uint8_t    font_Height_in_Pixel_for_all_characters;
 *     unit8_t    font_First_Char;
 *     uint8_t    font_Char_Count;
 *
 *     uint8_t    font_Char_Widths[font_Last_Char - font_First_Char +1];
 *                  // for each character the separate width in pixels,
 *                  // characters < 128 have an implicit virtual right empty row
 *
 *     uint8_t    font_data[];
 *                  // bit field of all characters
 */

#ifndef webdings_H
#define webdings_H

#define webdings_WIDTH 10
#define webdings_HEIGHT 7

GLCDFONTDECL(webdings) = {
    0x00, 0x40, // size
    0x0A, // width
    0x07, // height
    0x33, // first char
    0x02, // char count
    
    // char widths
    0x04, 0x04, 
    
    // font data
    0x10, 0x38, 0x7C, 0xFE, // 51
    0xFE, 0x7C, 0x38, 0x10 // 52
    
};

#endif
