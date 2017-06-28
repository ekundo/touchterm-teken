#include <tgmath.h>

#include <stdarg.h>
#include <math.h>
#include "inkview.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "redraw.c"
#include "font.c"
#include "button.c"
#include "button-placement.c"
#include "unix.c"
#include "teken-term.c"

#ifndef FONT_SIZE
#define FONT_SIZE 16
#endif
#ifndef KB_WIDTH
#define KB_WIDTH 750
#endif
#ifndef LETTER_KEY_WIDTH
#define LETTER_KEY_WIDTH 55
#endif

#ifndef TERM_WIDTH
#define TERM_WIDTH 80
#endif
#ifndef TERM_HALF_HEIGHT
#define TERM_HALF_HEIGHT 20
#endif

#define BYE_MSG "\n\nBye\n\n\0"
#define GREET_MSG  "Да-да?\n\0"

#define SHELL "/mnt/ext2/applications/shell.sh"
#define LOGFILE "/mnt/ext2/applications/tt-log.txt"

#define OUTPUT_CHECK_DELAY_A 50
#define OUTPUT_CHECK_DELAY_P 500
#define OUTPUT_CHECK_DELAY_TO 20

#ifndef TERM_ONE_OUTPUT_STREAM
#define TERM_ONE_OUTPUT_STREAM 1
#endif

int pbSW, pbSH, pbOrient;

struct globalAppState {
  struct tekenTerm term;

  struct button mainButtons[MAX_BUTTONS];
  struct buttonSet mainButtonSet;

  int shellFD[3];
  int shellPID;

  int reverseStreamOrder;
  int noEcho;
 
  int idleCount;
  int outputCheckDelay;

  int ctrl_pressed, ctrlIdx;
};

struct globalAppState sta;

void ChangeOrientation(void) {
  SetOrientation(pbOrient);
  pbSW=ScreenWidth(); 
  pbSH=ScreenHeight();

  FullUpdate();

  rp.timer = 0;
}

void checkOneStream(int fd){
  char buffer[1032];
  char buffer2[2064];
  
  int i,j;

  int rl;

  rl=read(fd, buffer, 1024);
  
  if (rl>0) {
    sta.idleCount = 0;
    
    j=0;
    for(i=0; i<rl; i++, j++){
      buffer2[j] = buffer[i];
      if(buffer2[j]==10){
        buffer2[++j]=13;
      }
    }

    tekenTerm_handleInput(&sta.term, (void*)buffer2, j);
  }
}

void checkOutputProc(){
  sta.idleCount ++;
  if (sta.reverseStreamOrder) {
    checkOneStream(sta.shellFD[1]);
    checkOneStream(sta.shellFD[2]);
  } else {
    checkOneStream(sta.shellFD[2]);
    checkOneStream(sta.shellFD[1]);
  }
  
  if(sta.idleCount > OUTPUT_CHECK_DELAY_TO && 
     sta.outputCheckDelay < OUTPUT_CHECK_DELAY_P) {
    sta.outputCheckDelay = OUTPUT_CHECK_DELAY_P;
  }
  if(sta.idleCount == 0 && 
     sta.outputCheckDelay > OUTPUT_CHECK_DELAY_A) {
    sta.outputCheckDelay = OUTPUT_CHECK_DELAY_A;
  }

  SetWeakTimer("term-console-app-output-check", checkOutputProc, 
    sta.outputCheckDelay);
}

void handleKeypress(char * s){
  write(sta.shellFD[0], s, strlen(s));
  if(!sta.noEcho) {
    tekenTerm_handleInput(&sta.term, s, strlen(s));
  } else {
  }
  sta.idleCount = 0;
  checkOutputProc();
}

void handleKeypresses (char * s){
  char *sp, *ep, *tp;
  char c;

  ep = s + strlen(s);
  sp = s;
  while (sp < ep){
    tp = sp + utf8width(sp);
    if(tp<ep){
      c = *tp;
      *tp = 0;
    }
    handleKeypress(sp);
    if(tp<ep){
      *tp=c;
    }
    sp = tp;
  };
}

void onExit(){
  tekenTerm_handleInput(&sta.term, BYE_MSG, strlen(BYE_MSG));
  FullUpdate();
  kill(sta.shellPID, SIGCONT);
  kill(sta.shellPID, SIGHUP);
  close(sta.shellFD[0]);
  kill(sta.shellPID, SIGTERM);
  kill(sta.shellPID, SIGQUIT);
  kill(sta.shellPID, SIGKILL);
  waitpid(sta.shellPID, NULL, 0);
  kill(0, SIGKILL);
  CloseApp();
}

void exitProc(int id){
  onExit();
}

void greetProc(int id){
  tekenTerm_handleInput(&sta.term, GREET_MSG, strlen(GREET_MSG));
}

char* keyLetters[] = {
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
  "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
  "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
  "`", "-", "=", "|", ";", "[", "]",
  NULL
  };

char* keyULetters[] = {
  "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", 
  "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
  "~", "_", "+", "", ":", "{", "}",
  NULL
  };

char* keyRLetters[] = {
  "а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и",
  "й", "к", "л", "м", "н", "о", "п", "р", "с", "т", "у", "ф", "х", 
  "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я", ".", "?", "'",
  "/", "<", ">", "", "", "«", "»",
  NULL
  };

char* keyURLetters[] = {
  "А", "Б", "В", "Г", "Д", "Е", "Ё", "Ж", "З", "И",
  "Й", "К", "Л", "М", "Н", "О", "П", "Р", "С", "Т", "У", "Ф", "Х", 
  "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я", ",", "!", "\"",
  "\\", "≤", "≥", "", "", "‘", "’",
  NULL
  };

char* keyLabels[] = {
  "1/а", "2/б", "3/в", "4/г", "5/д", "6/е", "7/ё", "8/ж", "9/з", "0/и",
  "a/й", "b/к", "c/л", "d/м", "e/н", "f/о", "g/п", "h/р", "i/с", "j/т", "k/у", "l/ф", "m/х", 
  "n/ц", "o/ч", "p/ш", "q/щ", "r/ъ", "s/ы", "t/ь", "u/э", "v/ю", "w/я", "x/.", "y/?", "z/'",
  "`//", "-/<", "=/>", "|", ";", "[/«", "]/»",
  NULL
  };

void ctrlProc(int id){
  sta.ctrl_pressed ^= 1;
  buttonDraw(& sta.mainButtons[sta.ctrlIdx], 1^sta.ctrl_pressed, 0);
}

void letterKeyProc(int id){
	if(sta.ctrl_pressed){
		char msg[5];
                char initial;
		initial=(keyLetters[id])[0];
                msg[0]=0;
                if(initial >= 'a' && initial <= 'z'){
                  msg[0]=initial-'a'+1;
                }
                if(initial >= '3' && initial <= '7'){
                  msg[0]=initial-'3'+0x1b;
                }
 		if(initial=='['){
			msg[0]=0x1b;
		}
 		if(initial==']'){
			msg[0]=0x1d;
		}
 		if(initial=='`'){
			msg[0]=0x1c;
		}
 		if(initial=='-'){
			msg[0]=0x1f;
		}
 		if(initial=='2'){
			msg[0]=0x0;
		}
 		if(initial=='8'){
			msg[0]=0x7f;
		}
		msg[1]=0;
		sta.ctrl_pressed = 0;
		buttonDraw(& sta.mainButtons[sta.ctrlIdx], 1, 0);
		handleKeypresses(msg);
	}else{
		handleKeypress(keyLetters[id]);
	}
}

void letterKeyDragProc(int id, int xsign, int ysign){
  if(xsign==1 && ysign == 0){
    handleKeypress(" ");
  }
  if(xsign==-1 && ysign == 0){
    handleKeypress("\177");
  }
  if(xsign==0 && ysign == 1){
    handleKeypress("\n");
  }
  if(xsign==0 && ysign == -1){
    handleKeypress(keyULetters[id]);
  }
  if(xsign==1 && ysign == -1){
    handleKeypress(keyRLetters[id]);
  }
  if(xsign==-1 && ysign == -1){
    handleKeypress(keyURLetters[id]);
  }
}

void dPadDragProc (int id, int xsig, int ysig){
  if(xsig==0 && ysig==-1){
    write(sta.shellFD[0], "\033[B", 3);
  }
}

void fullRedrawProc(){
  FillArea(0,0,pbSW,pbSH,WHITE);
  tekenTerm_redraw(&sta.term);
  redrawButtonSet(&sta.mainButtonSet);
  FullUpdate();
}

void blackProc(int id){
  FillArea(0,0,pbSW,pbSH,BLACK);
  FullUpdate();
  SetWeakTimer("blackproctimer", fullRedrawProc, 10000);
}

void blackProcDrag(int id, int xsig, int ysig){
  fullRedrawProc();
}

void swapStreamProc(int id){
  sta.reverseStreamOrder = ! sta.reverseStreamOrder;
  if (sta.reverseStreamOrder) {
    sta.mainButtons[id].label = "1,2"; 
  } else {
    sta.mainButtons[id].label = "2,1"; 
  }
  buttonDraw(&sta.mainButtons[id], 1, 0);
}

void noEchoProc(int id){
  sta.noEcho = ! sta.noEcho;
  if(sta.noEcho){
    sta.mainButtons[id].label = "@-"; 
  } else {
    sta.mainButtons[id].label = "@+"; 
  }
  buttonDraw(&sta.mainButtons[id], 1, 0);
}

#define BUTTON_SET_CALLBACKS(but, _id, _callback, _drag) but.id = _id; but.callback = _callback; but.drag = _drag;

void initButtons(){
  int i;  
  int lc;
  struct buttonPlacer bp;

  i = 0;

  bp.x = 10;
  bp.cx = bp.x;
  bp.cy = sta.term.y + defaultFontHeight * 2 * TERM_HALF_HEIGHT + 5;
  bp.w = KB_WIDTH;
  bp.bh = 2*defaultFontHeight+1;
  bp.bdw = 5;
  bp.bdh = 5;
  
  buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "Bye");
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], 0, exitProc, 0);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;
 
  buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "Black");
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], 0, blackProc, blackProcDrag);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;
 
  /*
  * buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "Greet");
  * BUTTON_SET_CALLBACKS(sta.mainButtons[i], 0, greetProc, 0);
  * addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  * i++;
  */
 
  lc=0;
  while(keyLetters[lc]){

  buttonPlacerAddButton(&bp, &sta.mainButtons[i], LETTER_KEY_WIDTH, keyLabels[lc]);
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], lc, letterKeyProc, letterKeyDragProc);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;

    lc++;
  }

  buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "<^>");
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], 0, 0, dPadDragProc);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;
 
  buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "2,1");
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], i, swapStreamProc, 0);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;
 
  buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "@-");
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], i, noEchoProc, 0);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;
 
  sta.ctrlIdx = i; 
  buttonPlacerAddButton(&bp, &sta.mainButtons[i], 0, "Ctrl");
  BUTTON_SET_CALLBACKS(sta.mainButtons[i], i, ctrlProc, 0);
  addButton(&sta.mainButtons[i], &sta.mainButtonSet, 1);
  i++;
}

int tmsx, tmsy, tmsa;
int termMouseDown(x,y){
  if (x>=sta.term.x && x<sta.term.x+defaultFontWidth*TERM_WIDTH &&
         y>=sta.term.y && y<sta.term.y+defaultFontHeight*2*TERM_HALF_HEIGHT){
    tmsa = 1;
    tmsx = (x-sta.term.x)/defaultFontWidth;
    tmsy = (y-sta.term.y)/defaultFontHeight;
    return 1;
  }
  return 0;
}
int termMouseUp(x,y){
	int nx, ny;
	if (tmsa) {
		if(
				x>=sta.term.x && x<sta.term.x+defaultFontWidth*TERM_WIDTH &&
				y>=sta.term.y && y<sta.term.y+defaultFontHeight*2*TERM_HALF_HEIGHT){
			tmsa = 0;
			nx = (x-sta.term.x)/defaultFontWidth;
			ny = (y-sta.term.y)/defaultFontHeight;
			if(nx>=tmsx && ny==tmsy){
				char msg[5*TERM_WIDTH];
				int i;
				char *sp;
				sp=msg;
				for(i=tmsx;i<=nx;i++){
					utf8encode(sta.term.symbols[tmsy][i], sp);
					sp+=utf8width(sp);
				}
				*sp=0;
				handleKeypresses(msg);
			}
		}
		return 1;
	}
	return 0;
}

int main_handler(int type, int par1, int par2) {
  char msg[65535];
  switch (type) {
    case EVT_INIT:
      {
        int fd;
        int fdo;

        fd = open("/dev/null", O_RDWR);
        fdo = open(LOGFILE, O_RDWR | O_CREAT);

        close(0);
        close(1);
        close(2);

        dup2(fd,0);
        dup2(fdo,1);
        dup2(fdo,2);
      }

      initDefaultFont("LiberationMono", FONT_SIZE);
      tekenTerm_init(&sta.term, TERM_WIDTH, 2*TERM_HALF_HEIGHT, -1);

      pbSW = ScreenWidth ();
      pbSH = ScreenHeight ();
      pbOrient = GetOrientation ();

      FillArea(1,1,1,1,WHITE);
      scheduleRedraw(1,1,2,2,0);

      sta.term.x = 5;
      sta.term.y = 5;

      greetProc(0);

      initButtons();

      tekenTerm_redraw(&sta.term);

      bindProcess(SHELL, sta.shellFD, &sta.shellPID, TERM_ONE_OUTPUT_STREAM);

      sta.term.client_stdin_pipe = sta.shellFD[0];

      checkOutputProc();
      
      sta.noEcho = 1;

      fullRedrawProc();
      break; 
    case EVT_POINTERDOWN:
      if(buttonSetDown(&sta.mainButtonSet, par1, par2)) break;
      if(termMouseDown(par1, par2)) break;
      break; 
    case EVT_POINTERMOVE:
      break;
    case EVT_POINTERUP:
      if(buttonSetUp(&sta.mainButtonSet, par1, par2)) break;
      if(termMouseUp(par1, par2)) break;
      break;
    case EVT_KEYPRESS:
      switch (par1) {
        case KEY_BACK:
          onExit();
          break;
      }
      break;
    case EVT_EXIT:
      break;
  }
  return 0;
}

int main(int argc, char **argv) {
  InkViewMain(main_handler);
  return 0;
}

