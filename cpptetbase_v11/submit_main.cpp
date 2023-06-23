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
#include "Tetris.h"

using namespace std;


/********************************************************/
/**************** Linux System Functions *********************/
/********************************************************/

char saved_key = 0;
int tty_raw(int fd);   /* put terminal into a raw mode */
int tty_reset(int fd);   /* restore terminal's mode */
  
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

  for (int x = 0; x < dx - 2*dw; x++) {
    string digit = "0" + to_string(x) + " ";
    cout << digit;
  }
  cout << endl;

  for (int y = dw; y < dy - dw; y++) {
    for (int x = dw; x < dx - dw; x++) {
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
    string digit = "0" + to_string(y-dw) + " ";
    cout << digit << endl;
   // cout<<endl;
  }
}
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

class MyOnLeft : public ActionHandler {
public:
    void run(Tetris *t, char key) {
        if(t->left >0)
          t->left--;
        return;
    }
};

class MyOnRight : public ActionHandler {
public:
    void run(Tetris *t, char key) {
        if (t->left < t->oScreen->get_dx()- t->wallDepth)
          t->left++;
        return;
    }
};


class MyOnDown : public ActionHandler {
public :
  void run(Tetris *t, char key) {
    if (t->top < t->oScreen->get_dy() - t->wallDepth)
      t->top++;
    return;
  }
};

class MyOnUp: public ActionHandler{
public:
  void run(Tetris *t, char key){
    if (t->top >0)
      t->top --;
    return;
  }
};


class MyOnstop : public ActionHandler{
public:
  void run(Tetris *t, char key){
    return ;
  }
};

Matrix* myDeleteFullLines(Matrix *screen, Matrix *blk, int top, int left, int dw) {
  Matrix *line, *bline, *zero, *zero2;
  int cy, y, cx, x;
  int nScanned, nScannedx;
  int ws_dy = screen->get_dy() - 2*dw;
  int ws_dx = screen->get_dx() - 2*dw;
  int x_sum10[ws_dx];
  int y_sum10[ws_dy];
  int idx;
  for (idx =0; idx<ws_dx; idx++){
    x_sum10[idx]=-1;
  }
  
  for (idx = 0; idx < ws_dy; idx++){
    y_sum10[idx]=-1;
  }
  
  //search range 
  if (top + blk->get_dy() > ws_dy + dw)
    nScanned = ws_dy + dw - top;
  else
    nScanned = blk->get_dy();

  if (left + blk->get_dx() > ws_dx + dw)
    nScannedx = ws_dx + dw - left;
  else
    nScannedx = blk->get_dx();

  //for paste
  zero = new Matrix(1, ws_dx);
  zero2 = new Matrix(ws_dy, 1);

  for (y = nScanned - 1, idx=0; y >= 0; y--) {
    cy = top + y ;
    line = screen->clip(cy, dw, cy+1, dw + ws_dx);       
    bline = line->int2bool(); // binary version of line
    delete line;
    if (bline->sum() == ws_dx) {
      y_sum10[idx]=cy;
    }
    idx++;
    delete bline; 
  }
  
  for (x = nScannedx - 1,idx=0 ; x >= 0; x--) {
    cx = left + x ;
    line = screen->clip(dw, cx, dw + ws_dy, cx+1);           
    bline = line->int2bool(); // binary version of line
    delete line;
    if (bline->sum() == ws_dy) {
      x_sum10[idx]=cx;
    }
    idx++;
    delete bline; 
  }  

  
  for (idx =0; idx < ws_dx; idx++){
    if (x_sum10[idx]!=-1){
      screen->paste(zero2, dw, x_sum10[idx]);
    }
  }
  
  for (idx =0; idx<ws_dy; idx++){
    if (y_sum10[idx]!=-1){
      screen->paste(zero, y_sum10[idx], dw);
    }
  }
  delete zero;
  delete zero2;
  return screen;
}

class MyOnNewBlock : public ActionHandler {
public:
    void run(Tetris *t, char key) {
       // cout<<"newblk called"<<endl;
        if (t->currBlk != NULL) // why test currBlk != NULL?
            //t->oScreen = myDeleteFullLines3(t->oScreen, t->currBlk, t->top, t->wallDepth);
            t->oScreen = myDeleteFullLines(t->oScreen, t->currBlk, t->top, t->left,t->wallDepth);
        t->iScreen->paste(t->oScreen, 0, 0);
        // select a new block
        t->type = key - '0';
        t->degree = 0;
        t->top = t->wallDepth; 
        t->left = t->cols/2 - t->wallDepth/2;
        t->currBlk = t->setOfBlockObjects[t->type][t->degree];
        return;
    }    
};




int main(int argc, char *argv[]) {
  char key;
  //registerAlarm(); // register one-second timer
  srand((unsigned int)time(NULL)); // init the random number generator
  
  TetrisState state;
  Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
  
  /////////////////////////////////////////////////////////////////////////
  /// Plug-in architecture for generalized Tetris class
  /////////////////////////////////////////////////////////////////////////
  Tetris::setOperation('a', TetrisState::Running, new MyOnLeft(),    TetrisState::Running, new MyOnstop(), TetrisState::Running);
  Tetris::setOperation('d', TetrisState::Running, new MyOnRight(), TetrisState::Running, new MyOnstop(),    TetrisState::Running);
  Tetris::setOperation('s', TetrisState::Running, new MyOnDown(), TetrisState::Running, new MyOnstop(),     TetrisState::Running);
  Tetris::setOperation('w', TetrisState::Running,  new OnClockWise(),    TetrisState::Running, new MyOnstop(),  TetrisState::Running);
  Tetris::setOperation('e', TetrisState::Running,  new MyOnUp(),    TetrisState::Running, new MyOnstop(),  TetrisState::Running);
  Tetris::setOperation(' ', TetrisState::Running, new MyOnstop(),   TetrisState::NewBlock, new MyOnstop(),     TetrisState::Running);
  Tetris::setOperation('0', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  Tetris::setOperation('1', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  Tetris::setOperation('2', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  Tetris::setOperation('3', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  Tetris::setOperation('4', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  Tetris::setOperation('5', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  Tetris::setOperation('6', TetrisState::NewBlock, new MyOnNewBlock(), TetrisState::Running, new OnFinished(), TetrisState::Finished);
  /////////////////////////////////////////////////////////////////////////

  Tetris *board = new Tetris(10, 10);

  key = (char) ('0' + rand() % board->get_numTypes());
  board->accept(key);
  
  drawScreen(board->get_oScreen(), board->get_wallDepth()); cout << endl;

  while ((key = getch()) != 'q') {
    state = board->accept(key);
    drawScreen(board->get_oScreen(), board->get_wallDepth()); cout << endl;
    if (state == TetrisState::NewBlock) {
      key = (char) ('0' + rand() % board->get_numTypes());

      state = board->accept(key);

      drawScreen(board->get_oScreen(), board->get_wallDepth()); cout << endl;
      if (state == TetrisState::Finished) 
        break;
    }
  }

  delete board;

  Tetris::deinit();
  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
  cout << "Program terminated!" << endl;
  return 0;
}