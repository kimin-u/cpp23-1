﻿#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "colors.h"
#include "Matrix.h"

using namespace std;


/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

char saved_key = 0;
int tty_raw(int fd);	/* put terminal into a raw mode */
int tty_reset(int fd);	/* restore terminal's mode */
  
/* Read 1 character - echo defines echo mode */
char getch() {
  char ch;
  int n;
  while (1) {
    tty_raw(0);
    n = read(0, &ch, 1);
    tty_reset(0);
    if (n > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
        if (saved_key != 0) {
          ch = saved_key;
          saved_key = 0;
          break;
        }
      }
    }
  }
  return ch;
}

void sigint_handler(int signo) {
  // cout << "SIGINT received!" << endl;
  // do nothing;
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void registerInterrupt() {
  struct sigaction act, oact;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGINT, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

int T0D0[] = { 1, 1, 1, 1, -1 };
int T0D1[] = { 1, 1, 1, 1, -1 };
int T0D2[] = { 1, 1, 1, 1, -1 };
int T0D3[] = { 1, 1, 1, 1, -1 };

int T1D0[] = { 0, 1, 0, 1, 1, 1, 0, 0, 0, -1 };
int T1D1[] = { 0, 1, 0, 0, 1, 1, 0, 1, 0, -1 };
int T1D2[] = { 0, 0, 0, 1, 1, 1, 0, 1, 0, -1 };
int T1D3[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, -1 };

int T2D0[] = { 1, 0, 0, 1, 1, 1, 0, 0, 0, -1 };
int T2D1[] = { 0, 1, 1, 0, 1, 0, 0, 1, 0, -1 };
int T2D2[] = { 0, 0, 0, 1, 1, 1, 0, 0, 1, -1 };
int T2D3[] = { 0, 1, 0, 0, 1, 0, 1, 1, 0, -1 };

int T3D0[] = { 0, 0, 1, 1, 1, 1, 0, 0, 0, -1 };
int T3D1[] = { 0, 1, 0, 0, 1, 0, 0, 1, 1, -1 };
int T3D2[] = { 0, 0, 0, 1, 1, 1, 1, 0, 0, -1 };
int T3D3[] = { 1, 1, 0, 0, 1, 0, 0, 1, 0, -1 };

int T4D0[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D1[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };
int T4D2[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D3[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };

int T5D0[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D1[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };
int T5D2[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D3[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };

int T6D0[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D1[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
int T6D2[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D3[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
  
int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

void drawScreen(Matrix *screen, int wall_depth)
{
  int dy = screen->get_dy();
  int dx = screen->get_dx();
  int dw = wall_depth;
  int **array = screen->get_array();

  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	      cout << "□ ";
      else if (array[y][x] == 1)
	      cout << "■ ";
      else if (array[y][x] == 10)
	      cout << "◈ ";
      else if (array[y][x] == 20)
	      cout << "★ ";
      else if (array[y][x] == 30)
	      cout << "● ";
      else if (array[y][x] == 40)
	      cout << "◆ ";
      else if (array[y][x] == 50)
	      cout << "▲ ";
      else if (array[y][x] == 60)
	      cout << "♣ ";
      else if (array[y][x] == 70)
	      cout << "♥ ";
      else
	      cout << "XX ";
    }
    cout << endl;
  }
}
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

#define SCREEN_DY  10
#define SCREEN_DX  10
#define SCREEN_DW  3

#define ARRAY_DY (SCREEN_DY + SCREEN_DW)
#define ARRAY_DX (SCREEN_DX + 2*SCREEN_DW)

int arrayScreen[ARRAY_DY][ARRAY_DX] = {
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },  
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },  
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },  
};

int arrayBlk[3][3] = {
  { 0, 1, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 },
};

Matrix *deleteFullLines(Matrix *screen, Matrix *blk, int top, int dw) {    // dw = 3 
  Matrix *line, *zero, *temp;
  int cy, y;
  int nDeleted, nScanned; 
  int ws_dy = screen->get_dy() - dw;          // 13-3 = 10
  int ws_dx = screen->get_dx() - 2*dw;        // 16-6 = 10 
  zero = new Matrix(1, ws_dx);                 //0으로 가득찬 1 x 10  매트릭스를 만들어서> delete line 후에 맨 위에 붙여넣으려고 함 .

  //nScanned = 반복할 반복문의 수  : 스캔할 행의 수 
  if (top + blk->get_dy() > ws_dy)  {         //currblk의 탑좌표 + y크기 (즉, currblk의 공백을 포함한 형태)가 밑바닥 벽을 뚫었냐?
    nScanned = ws_dy - top;                  //뚫은 경우에는  nscanned를 10-top으로 지정 .
  } 
  else{                                       //currblk의 탑좌표  + y크기(즉, currblk의 공백을  포함한 형태)가 바닥을 뚫지 않은 경우     cout<<"top + Blk->get dy :"<<top + blk->get_dy()<<endl;
    nScanned = blk->get_dy();                 //뚫지 않은 경우에는  currblk의 y좌표 크기로 초기화. 
  }
  nDeleted=0;
  for (y = nScanned - 1; y >= 0; y--) {    //y를 1씩 감소시키며 반복문의 루프 수를 지정, ndeleted는 삭제가 된 경우에만 증가해서 
    cy = top + y + nDeleted;                                //클립을 딸 행의 열  
    line = screen->clip(cy, dw, cy+1, dw + ws_dx);               // 한 행씩 클립을 땀. 
    if (line->sum() == ws_dx) {                      //한 행의 합이 ==10인 경우
      temp = screen->clip(0, dw, cy, dw + ws_dx);          // 방금 딴 행 위까지의 영역을 클립
      screen->paste(temp, 1, dw);                          //한라인 내려야 하므로 paste top좌표 1
      screen->paste(zero, 0, dw);                       //제일 윗라인 0으로 꽉 채움 
      nDeleted++;
      delete temp;
    }
    delete line; 
  }
  delete zero;
  return screen;                 
}

int main(int argc, char *argv[]) {
  char key;
  int top, left, type, degree;
  int max_blk_size = 0;
  bool touchDown = false;

  // register one-second timer
  //registerAlarm();

  // init the random number generator
  srand((unsigned int)time(NULL));

  // init 28 block objects
  Matrix *setOfBlockObjects[MAX_BLK_TYPES][MAX_BLK_DEGREES];
  for (int t = 0; t < MAX_BLK_TYPES; t++) {
    int *array = setOfBlockArrays[t*MAX_BLK_DEGREES];
    int idx, size;
    for (idx = 0; array[idx] != -1 ; idx++); // find the element of -1 in array[]
    for (size = 0; size*size < idx; size++); // comupte the square root of idx
    max_blk_size = (size > max_blk_size ? size : max_blk_size);
    for (int d = 0; d < MAX_BLK_DEGREES; d++) {
      setOfBlockObjects[t][d] = new Matrix(setOfBlockArrays[t*MAX_BLK_DEGREES+d], size, size);
      //cout << *setOfBlockObjects[t][d] << endl;
    }
  }

  // select a new block
  type = rand() % MAX_BLK_TYPES;
  degree = 0;
  top = 0; 
  left = SCREEN_DW + SCREEN_DX/2 - max_blk_size/2;

  // init variables for screen refresh with the new block
  Matrix *iScreen = new Matrix((int *) arrayScreen, ARRAY_DY, ARRAY_DX);
  Matrix *oScreen = new Matrix(iScreen);
  Matrix *currBlk = setOfBlockObjects[type][degree];
  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
  Matrix *tempBlk2 = tempBlk->add(currBlk);
  delete tempBlk;

  oScreen->paste(iScreen, 0, 0);
  oScreen->paste(tempBlk2, top, left);
  drawScreen(oScreen, SCREEN_DW);

  if (tempBlk2->anyGreaterThan(1)) { // exit the game
    delete tempBlk2;
    goto done;
  }
  delete tempBlk2;

  while ((key = getch()) != 'q') {
    switch (key) {
      case 'a': left--; break;
      case 'd': left++; break;
      case 'w': 
        degree = (degree + 1) % MAX_BLK_DEGREES; 
        currBlk = setOfBlockObjects[type][degree]; 
        break;
      case 's': top++; break;
      case ' ': 
        while (true) {
          top++;
          tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
          tempBlk2 = tempBlk->add(currBlk);
          delete tempBlk;
          if (tempBlk2->anyGreaterThan(1)) {
            delete tempBlk2;
            break;
          }
          delete tempBlk2;
        }
        break;
      default: cout << "wrong key input" << endl;
    }

    tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
    tempBlk2 = tempBlk->add(currBlk);
    delete tempBlk;
    
    if (tempBlk2->anyGreaterThan(1)) {
      switch (key) {
        case 'a': left++; break;
        case 'd': left--; break;
        case 'w': 
          degree = (degree - 1) % MAX_BLK_DEGREES; 
          currBlk = setOfBlockObjects[type][degree]; 
          break;
        case 's': top--; touchDown = true; break;
        case ' ': top--; touchDown = true; break;
      }
      delete tempBlk2;
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);    
      delete tempBlk;
    }
    
    // update oScreen
    oScreen->paste(iScreen, 0, 0);
    oScreen->paste(tempBlk2, top, left);
    delete tempBlk2;
    drawScreen(oScreen, SCREEN_DW);

    // handle touchDown
    if (touchDown) { 
      oScreen = deleteFullLines(oScreen, currBlk, top, SCREEN_DW);
      touchDown = false;
      // select a new block
      type = rand() % MAX_BLK_TYPES;
      degree = 0;
      top = 0; 
      left = SCREEN_DW + SCREEN_DX/2 - max_blk_size/2;

      // init variables for screen refresh with the new block
      iScreen->paste(oScreen, 0, 0);
      currBlk = setOfBlockObjects[type][degree];
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;

      // update oScreen before conflict test
      oScreen->paste(iScreen, 0, 0);
      oScreen->paste(tempBlk2, top, left);
      drawScreen(oScreen, SCREEN_DW);
      if (tempBlk2->anyGreaterThan(1)) { // exit the game
        delete tempBlk2;
        goto done;
      }
      delete tempBlk2;
    }
  }

done:
  delete iScreen;
  delete oScreen;

  // free the 28 block objects
  for (int t = 0; t < MAX_BLK_TYPES; t++)
    for (int d = 0; d < MAX_BLK_DEGREES; d++)
      delete setOfBlockObjects[t][d];

  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
  cout << "Program terminated!" << endl;

  return 0;
}

