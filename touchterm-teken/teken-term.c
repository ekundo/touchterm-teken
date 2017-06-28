#ifndef _TEKEN_TERM_C_INC_
#define _TEKEN_TERM_C_INC_ 1

#include <stdlib.h>
#include <stdio.h>

#include "../teken/teken.c"

#include "utf8.c"
#include "font.c"
#include "primitives.c"
#include "redraw.c"

#ifndef TEKEN_TERM_MAX_WIDTH
#define TEKEN_TERM_MAX_WIDTH 257
#endif
#ifndef TEKEN_TERM_MAX_HEIGHT
#define TEKEN_TERM_MAX_HEIGHT 1000
#endif

struct tekenTerm {
  teken_unit_t cx, cy;
  teken_unit_t w, h;
  int x, y;
  int polarity;
  teken_char_t symbols[TEKEN_TERM_MAX_HEIGHT][TEKEN_TERM_MAX_WIDTH];
  teken_t * interpreter;
  int client_stdin_pipe;
};

void tekenTerm_drawCharacter(struct tekenTerm * tt, teken_unit_t row, teken_unit_t col, int immed){
  int sx,sy;
  int fg, bg;
  char msg[5];
  
  bg = ((tt->cx==col && tt->cy==row)^tt->polarity)?BLACK:WHITE;
  fg = ((tt->cx==col && tt->cy==row)^tt->polarity)?WHITE:BLACK;

  sx = tt->x + col * defaultFontWidth;
  sy = tt->y + row * defaultFontHeight;
  FillArea(sx,sy,defaultFontWidth, defaultFontHeight,bg);
  SetFont(defaultFont, fg); 
  memset(msg, 0, 5);
  utf8encode(tt->symbols[row][col], msg);
  if(! msg[0]) msg[0]=' ';
  DrawString(sx, sy, msg);
  scheduleRedraw(sx,sy,sx+defaultFontWidth,sy+defaultFontHeight, immed);
}

void tekenTerm_bell(void * tt){
}

void tekenTerm_setCursor(void* tt, const teken_pos_t * pos){
  struct tekenTerm * t = (struct tekenTerm * )tt;
  int ocx, ocy;
  ocx = t->cx;
  ocy = t->cy;
  t->cx = pos->tp_col;
  t->cy = pos->tp_row;
  tekenTerm_drawCharacter(t, pos->tp_row, pos->tp_col, 0);
  tekenTerm_drawCharacter(t, ocy, ocx, 0);
}

void tekenTerm_putCharacter(void * tt, const teken_pos_t * pos, 
    teken_char_t ch, const teken_attr_t * attr){
  struct tekenTerm * t = (struct tekenTerm * )tt;
  t->symbols[pos->tp_row][pos->tp_col] = ch;
  tekenTerm_drawCharacter(t, pos->tp_row, pos->tp_col, 0);
}

void tekenTerm_fill(void* tt, const teken_rect_t * area, teken_char_t ch,
    const teken_attr_t * attr){
  teken_pos_t p;
  for(p.tp_col=area->tr_begin.tp_col; p.tp_col<area->tr_end.tp_col; p.tp_col++){
    for(p.tp_row=area->tr_begin.tp_row; p.tp_row<area->tr_end.tp_row; p.tp_row++){
      tekenTerm_putCharacter(tt, &p, ch, attr);
    }
  }
}

void tekenTerm_copy(void* tt, const teken_rect_t * src, const teken_pos_t * pos){
  struct tekenTerm * t = (struct tekenTerm * )tt;
  teken_pos_t p, tp;
  int step_x, step_y, dx, dy;

  dx = pos->tp_col - src->tr_begin.tp_col;
  dy = pos->tp_row - src->tr_begin.tp_row;
  step_x = (dx > 0) ? -1 : 1;
  step_y = (dy > 0) ? -1 : 1;
  for(p.tp_col=(step_x>0)? src->tr_begin.tp_col : src->tr_end.tp_col; 
      (p.tp_col < src->tr_end.tp_col) && (p.tp_col >= src->tr_begin.tp_col);
      p.tp_col += step_x){
    for(p.tp_row=(step_y>0)? src->tr_begin.tp_row : src->tr_end.tp_row; 
        (p.tp_row < src->tr_end.tp_row) && (p.tp_row >= src->tr_begin.tp_row);
        p.tp_row += step_y){
      tp.tp_col = p.tp_col + dx;
      tp.tp_row = p.tp_row + dy;
      tekenTerm_putCharacter(tt, &tp, t->symbols[p.tp_row][p.tp_col], NULL);
    }
  }
}

void tekenTerm_param(void * tt, int kind, unsigned int value){
}

void tekenTerm_respond(void * tt, const void* reply, size_t len){
  struct tekenTerm * t = (struct tekenTerm * )tt;
  if(t->client_stdin_pipe > -1){
    write(t->client_stdin_pipe, reply, len);
  }
}

void tekenTerm_init(struct tekenTerm * t, int w, int h, int fd){
  teken_funcs_t * tf = (teken_funcs_t *) malloc(sizeof(teken_funcs_t));
  teken_pos_t pos;
  tf-> tf_bell = tekenTerm_bell;
  tf-> tf_cursor = tekenTerm_setCursor;
  tf-> tf_putchar = tekenTerm_putCharacter;
  tf-> tf_fill = tekenTerm_fill;
  tf->tf_copy = tekenTerm_copy;
  tf->tf_param = tekenTerm_param;
  tf->tf_respond = tekenTerm_respond;

  t->interpreter = (teken_t*) malloc(sizeof(teken_t));
  t->w = w;
  t->h = h;
  t->client_stdin_pipe = fd;

  teken_init(t->interpreter, tf, t);
  
  pos.tp_col = w;
  pos.tp_row = h;

  teken_set_winsize(t->interpreter, &pos);
}

void tekenTerm_handleInput(struct tekenTerm * t, void* buffer, size_t len){
  teken_input(t->interpreter, buffer, len);
}

void tekenTerm_redraw(struct tekenTerm * t){
  teken_unit_t x, y;
  for(x=0; x<t->w; x++){
    for(y=0; y<t->h; y++){
      tekenTerm_drawCharacter(t, x, y, 0);
    }
  }
}

#endif 
