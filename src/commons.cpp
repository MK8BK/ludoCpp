#include "commons.h"
#include <algorithm>

namespace gamespace {

int WINDOW_SIZE = 810; // is divisible by 15 ... :) 
int TS = WINDOW_SIZE/15;
int TSmall = TS-2;

void syncSizes(int w, int h){
  WINDOW_SIZE = std::min(w,h);
  WINDOW_SIZE -= WINDOW_SIZE%15;
  TS = WINDOW_SIZE/15;
  TSmall = TS-2;
}

}