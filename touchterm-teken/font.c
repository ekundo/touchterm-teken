#ifndef _FONT_C_INC_
#define _FONT_C_INC_ 1

ifont* defaultFont;
int defaultFontHeight;
int defaultFontWidth;

void initDefaultFont(char* fn, int fs){
  defaultFont = OpenFont(fn, fs, 0);
  SetFont(defaultFont, BLACK);
  defaultFontWidth = StringWidth("W");
  defaultFontHeight = TextRectHeight(StringWidth("ghTWy"), "ghTWy", 0);
}

#endif 
