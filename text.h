/*
 * tab:4
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep  9 22:08:16 2004
 * Filename:      text.h
 * History:
 *    SL    1    Thu Sep  9 22:08:16 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:40:11 2009
 *        Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT  16

/* Standard VGA text font. */
extern unsigned char font_data[256][16];
//takes in the input string, status buffer, and color customizations and writes to the given buffer in the correct format
extern void Ascii_buf( char * input, int size, unsigned char * buf, unsigned char word_color, unsigned char background_color );

// generates a mask for the text with 1 indicating if text needs to be drawn
extern void  Text_Mask_Gen(char * input, int size, unsigned char * base);

/*The Ascii_buf function works by first converting the input string or char array into a 18 row by 320 column 
2d array. It does this by traversing each char in the array using a 16 by 8 double for loop. It starts writing at the second
row since we have 1 row above and below in the status bar for padding. It calls the correct char from the fonts array using
the ascii offset which is the value itself  It used an x offset variable that will be set to the center by being width/2 - fontwidth/2
 Once each char is printed, thus x offset is offset by font width so it prints all the chars side by side. Once this output array is
 created, I then add the values into the buffer plane by plane. For each plane, I traverse the first element of each column and add it, then the second
 column, and so on.*/
#endif /* TEXT_H */
