#ifndef _BUTTON_C_INC_
#define _BUTTON_C_INC_ 1

#define MAX_BUTTONS 1024

#include "primitives.c"

#include "time.h"

typedef void (*buttonCallback)(int id);
typedef  void (*buttonDragCallback)(int id, int xsign, int ysign);

struct button {
  int xl, xr, yt, yb;
  int id;
  buttonCallback callback;
  buttonDragCallback drag;
  char* label;

  int down;
};

struct buttonSet {
  int fill;
  struct button * content[MAX_BUTTONS];
};

void buttonDraw(struct button * b, int polarity, int immed){
  int bg = polarity?WHITE:BLACK;
  int fg = polarity?BLACK:WHITE;
  DrawRect(b->xl-1, b->yt-1, b->xr-b->xl+2, b->yb-b->yt+2, fg);
  FillArea(b->xl, b->yt, b->xr-b->xl, b->yb-b->yt, bg);
  drawText(b-> label, b->xl+1, b->yt+1, fg, bg, 0);
  scheduleRedraw(b->xl-1, b->yt-1, b->xr+1, b-> yb+1, immed);
}

void addButton(struct button * b, struct buttonSet * bs, int draw){
  if(bs->fill >= MAX_BUTTONS) return;
  bs->content[bs->fill ++] = b;
  if(draw) buttonDraw(b, 1, 0);
}

int inside(struct button * b, int x, int y){
  return 
    (x >= b->xl) &&
    (x <= b->xr) &&
    (y >= b->yt) &&
    (y <= b->yb) ;
}

int buttonDown(struct button * b, int x, int y){
  if(inside(b, x, y)){
    b-> down = 1;
    buttonDraw(b, 0, 0);
    return 1;
  }
  return 0;
}

int buttonUp(struct button * b, int x, int y){
	if(b->down){
		buttonDraw(b, 1, 0);
		b->down = 0;
		if(inside(b, x, y)){
			if(b->callback) (b->callback)(b->id);
		} else {
			if(b->drag){
				int xsig, ysig;
				xsig = (x < b->xl)? -1 : (x > b->xr)? 1: 0;
				ysig = (y < b->yt)? -1 : (y > b->yb)? 1: 0;
                                (b->drag)(b->id, xsig, ysig);
			}
		}
		return 1;
	}
	return 0;
}

int buttonSetDown(struct buttonSet * bs, int x, int y){
  int i;
  for (i=0; i<bs->fill; i++){
    if(buttonDown(bs->content[i], x, y)) return 1;
  }
  return 0;
};

int buttonSetUp(struct buttonSet * bs, int x, int y){
  int i;
  for (i=0; i<bs->fill; i++) {
    if(buttonUp(bs->content[i], x, y)) return 1;
  }
  return 0;
};

void redrawButtonSet(struct buttonSet * bs){
  int i;
  for (i=0; i<bs->fill; i++){
    buttonDraw(bs->content[i], 1, 0);
  }
}

#endif 
