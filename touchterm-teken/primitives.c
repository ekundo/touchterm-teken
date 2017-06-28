#ifndef _PRIMITIVES_C_INC_
#define _PRIMITIVES_C_INC_ 1

#include <tgmath.h>

#include <stdarg.h>
#include <math.h>
#include "inkview.h"
#include <stdio.h>
#include <stdlib.h>

#include "redraw.c"
#include "font.c"

void drawText(char * s, int x, int y, int fg, int bg, int immed) {
  int sw, sh;
  sw = StringWidth(s);
  sh = TextRectHeight(sw, s, 0);
  FillArea(x, y, sw, sh, bg);
  SetFont(defaultFont, fg);
  DrawString (x, y, s);
  scheduleRedraw(x, y, x+sw, y+sh, immed);
}

void drawHairCross (x, y, d, fg, bg, immed) {
  FillArea(x-d, y-d, 2*d, 2*d, bg);
  FillArea(x-d, y, 2*d, 1, fg);
  FillArea(x, y-d, 1, 2*d, fg);
  scheduleRedraw(x-d, y-d, x+d, y+d, immed);
}

#endif 
