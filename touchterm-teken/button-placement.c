#ifndef _BUTTON_PLACEMENT_C_INC_
#define _BUTTON_PLACEMENT_C_INC_ 1

#include "trivial.c"

struct buttonPlacer {
  int x, w, bh, bdw, bdh, cx, cy;
};

void buttonPlacerAddButton(struct buttonPlacer * bp, 
    struct button * b, int width, char * label)
{
  int realWidth = max(width, StringWidth(label)+1);
  if(realWidth+bp->cx > bp->w){
    bp->cx = bp->x;
    bp->cy = bp->cy + bp-> bh + bp->bdh; 
  }
  b->xl = bp->cx;
  b->xr = b->xl + realWidth;
  b->yt = bp->cy;
  b->yb = b->yt + bp->bh;
  b->label = label;

  bp->cx = b->xr+bp->bdw;
}

#endif 
