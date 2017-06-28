#ifndef _UTF8_C_INC_
#define _UTF8_C_INC_ 1
int utf8width(char * s){
	if (s[0]) {
		if (s[0] & 0x80) {
			if(s[0] & 0x40) {
				if (s[0] & 0x20) {
					if (s[0] & 0x10) {
						return 4;
					} else {
						return 3;
					}
				} else {
					return 2;
				}
			} else {
				return 0;
			}
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

void utf8Cut(char * s, int width) {
  int i;
  char * sp;
  sp = s;
  for (i=0; i<width; i++){sp+=max(1,utf8width(sp));};
  *sp = 0;
}

int utf8decode(char * s){
  switch(utf8width(s)){
    case 0:
      return 0; 
      break;
    case 1:
      return s[0];
      break;
    case 2:
      return ((s[0] & 0x1F)<<6) | (s[1] & 0x3F);
      break;
    case 3:
      return ((s[0] & 0x0F)<<12) | ((s[1] & 0x3F) << 6) | ((s[2] & 0x3F) << 0);
      break;
    case 4:
      return ((s[0] & 0x07)<<18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | ((s[3] & 0x3F) << 0);
      break;
    default:
      return 0;
  }
}

#define UTF8_BLOCK(var, mask, shift, prefix) (((1 << prefix)-1) << (8-prefix)) | ((var & (((1 << mask) - 1) << shift)) >> shift)

void utf8encode(int sym, char * s){
  if(sym<=0x7f){
    s[0] = UTF8_BLOCK(sym, 7, 0, 0);
  } else if(sym<=0x7ff){
    s[0] = UTF8_BLOCK(sym, 5, 6, 2);
    s[1] = UTF8_BLOCK(sym, 6, 0, 1);
  } else if(sym<=0xffff) {
    s[0] = UTF8_BLOCK(sym, 4, 12, 3);
    s[1] = UTF8_BLOCK(sym, 6, 6, 1);
    s[2] = UTF8_BLOCK(sym, 6, 0, 1);
  } else if(sym<=0x10FFFF) {
    s[0] = UTF8_BLOCK(sym, 3, 18, 4);
    s[1] = UTF8_BLOCK(sym, 6, 12, 1);
    s[2] = UTF8_BLOCK(sym, 6, 6, 1);
    s[3] = UTF8_BLOCK(sym, 6, 0, 1);
  } else {
    s[0] = 0;
  }
}

int utf8cutPoint(char *s){
  int l = strlen(s);
  int j = 0;
  int w = utf8width(s);
  
  while(j+w<l) {
    j+=w;
    w = max(1,utf8width(s+j));
  }

  return j+w;
}

#endif 
