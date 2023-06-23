#include <iostream>
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
 
  for (int y = 0; y < dy-dw+1; y++) {
    for (int x = dw-1; x < dx-dw+1; x++) {
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
	      cout << "X ";
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

#define ARRAY_DY (SCREEN_DY + SCREEN_DW)    // 13
#define ARRAY_DX (SCREEN_DX + 2*SCREEN_DW)   // 16

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
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }  
};


int arrayBlk[3][3] = {
  { 0, 1, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 },
};

//deleteFullLines함수 정의하기
void deleteFullLines(Matrix *iScreen){
  bool isfullline = false;
  Matrix * isfullrow = NULL;
  Matrix * pull = NULL;

  for(int y=0; y<SCREEN_DY; y++){
    if (isfullrow != NULL)
      delete isfullrow;

    isfullrow = iScreen->clip(y,SCREEN_DW,y+1,SCREEN_DX+SCREEN_DW);
    if (isfullrow->sum()==10)   
      isfullline= true;
    
    if (isfullline){
      if (pull!=NULL)
        delete pull;

      pull= iScreen->clip(0,SCREEN_DW,y,SCREEN_DX+SCREEN_DW);
      iScreen->paste(pull,1,SCREEN_DW);  
      isfullline=false;
    }
  }
  if (isfullrow != NULL)
      delete isfullrow;

  if (pull!=NULL)
        delete pull;
      
}

int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL)); 

  char key;
  int top = 0, left = 6;

  Matrix* setOfBlockObjects[7][4];
  int i,j,size_clip;

  for(i=0; i<7; i++) {
    for(j=0; j<4; j++){
      if (i==0){
        size_clip=2;
      }
      else if (i==6){
        size_clip =4;
      }
      else {
        size_clip=3;
      }
      setOfBlockObjects[i][j]= new Matrix(setOfBlockArrays[i*4+j], size_clip, size_clip);
    }
  }

  Matrix *iScreen = new Matrix((int *) arrayScreen, ARRAY_DY, ARRAY_DX);
 
  int blkType = rand() % MAX_BLK_TYPES;
  int idxBlockDegree=0;
  Matrix *currBlk = setOfBlockObjects[blkType][idxBlockDegree];
  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
  Matrix *tempBlk2 = tempBlk->add(currBlk);
  delete tempBlk;

  Matrix *oScreen = new Matrix(iScreen); //Matrix.cpp 47line~
  oScreen->paste(tempBlk2, top, left);
  delete tempBlk2;
  drawScreen(oScreen, SCREEN_DW);
  delete oScreen;
  cout<<endl;

//bool type; check for new block needed
//cannot move to down or after ' ' input; -> true;
  bool newblkbool=false;

  while ((key = getch()) != 'q') {
    switch (key) {
      case 'a': left--; break;  //move to left
      case 'd': left++; break;  //move to right
      case 's': top++; break;   //move to down
      case 'w': {               //rotate
        idxBlockDegree= (idxBlockDegree+1) % MAX_BLK_DEGREES;   //0,1,2,3
        currBlk = setOfBlockObjects[blkType][idxBlockDegree];
        break;
      }
      case ' ':{                 //drop
        while (true){
          top++;
          tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
          tempBlk2 = tempBlk->add(currBlk);
          delete tempBlk;

          if (tempBlk2->anyGreaterThan(1)) {  // cannot move to down; break loop;
            delete tempBlk2;
            break;
          }
          delete tempBlk2;
        }       
        break;
      }
      default: cout << "wrong key input" << endl;
    }
  
    tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
    tempBlk2 = tempBlk->add(currBlk);
    delete tempBlk;

//after crump;
    if (tempBlk2->anyGreaterThan(1)){   
      delete tempBlk2;
      switch(key){
        case 'a': left++; break;
        case 'd': left--; break;
        case 's': {
          top--;
          newblkbool=true;                       //cannot move to down ; newblock needed
          break;
        }
        case 'w': {
          idxBlockDegree= (idxBlockDegree-1);   //0,1,2,3
          if (idxBlockDegree==-1) 
            idxBlockDegree=3; 
          currBlk = setOfBlockObjects[blkType][idxBlockDegree];
          break;
        }
        case ' ': {
          top--;
          newblkbool=true;                     //after drop; newblock needed
          break; 
        }
        default: cout << "wrong key input" << endl;
      }

      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
    }

//drawmatrix;
    oScreen = new Matrix(iScreen);
    oScreen->paste(tempBlk2, top, left);
    delete tempBlk2;
    drawScreen(oScreen, SCREEN_DW);
    cout<<endl;

//newblockneeded -> modify iScreen; save block falled;
    if (newblkbool){
      delete iScreen;
      iScreen= new Matrix(oScreen);
    }

    delete oScreen;

//newblockneeded ->  execute deleteFullline function;
    if (newblkbool){
      deleteFullLines(iScreen);
    }
  //generate newblk;
    if (newblkbool){
      newblkbool=false; 
      top=0; left=6;
      blkType = rand() % MAX_BLK_TYPES; idxBlockDegree=0;
      currBlk = setOfBlockObjects[blkType][idxBlockDegree];
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
  //gameover;  cannot generate  newblock
      if (tempBlk2->anyGreaterThan(1)) {
        delete tempBlk2;
        cout<<"====Game Over===="<<endl;
        break;
      }      
  //draw oscreen;
      oScreen = new Matrix(iScreen);
      oScreen->paste(tempBlk2, top, left);
      delete tempBlk2;
      drawScreen(oScreen, SCREEN_DW);
      cout<<endl;
      delete oScreen;
    }

  }   //while loop end line

// prevent memory leak 
  delete iScreen;
//setofblockobj memory return;
  for(i=0;i<7;i++){
    for(j=0;j<4;j++){
      delete setOfBlockObjects[i][j];
    }
  }

  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
  cout << "Program terminated!" << endl;

  return 0;
}

