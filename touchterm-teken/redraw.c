#ifndef _REDRAW_C_INC_
#define _REDRAW_C_INC_ 1

#include <tgmath.h>

#include <stdarg.h>
#include <math.h>
#include "inkview.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef REDRAW_DELAY
#define REDRAW_DELAY 300
#endif

#include "trivial.c"

struct redrawPlan {
  int xl, xr, yt, yb;
  int timer;
};

struct redrawPlan rp;

int updateProcType;

void redrawProc() {
  switch (updateProcType) {
    case 0:
      PartialUpdate(rp.xl, rp.yt, rp.xr-rp.xl , rp.yb-rp.yt );
      break;
    case 1:
      PartialUpdateBW(rp.xl, rp.yt, rp.xr-rp.xl , rp.yb-rp.yt );
      break;
    case 2:
      PartialUpdateBlack(rp.xl, rp.yt, rp.xr-rp.xl , rp.yb-rp.yt );
      break;
    case 3:
      DynamicUpdateBW(rp.xl, rp.yt, rp.xr-rp.xl , rp.yb-rp.yt );
      break;
    case 4:
      SoftUpdate();
      break;
    case 5:
      FineUpdate();
      break;
  }
  rp.timer = 0;
}

void scheduleRedraw (int xl, int yt, int xr, int yb, int immed) {
	if (immed){ 
		PartialUpdate(xl, yt, xr-xl, yb-yt);
	} else {
		if (rp.timer) {
			rp.xl = min(xl, rp.xl );    
			rp.xr = max(xr, rp.xr );    
			rp.yt = min(yt, rp.yt );    
			rp.yb = max(yb, rp.yb );    
		} else {
			rp.xl = xl;    
			rp.xr = xr;    
			rp.yt = yt;    
			rp.yb = yb;    

			rp.timer = 1;
			SetWeakTimer ("redrawTimer", redrawProc, REDRAW_DELAY);
		}
	}
}

#endif
