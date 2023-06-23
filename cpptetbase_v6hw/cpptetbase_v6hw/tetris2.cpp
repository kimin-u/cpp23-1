#include "Tetris.h"

using namespace std;

Matrix *** Tetris::setOfBlockObjects = NULL;
int Tetris::numTypes=0;
int Tetris::numDegrees=0;
int Tetris::wallDepth=0;

//init 
void Tetris::init(int** setofblockarrays, int nTypes, int nDegrees){
    if (setOfBlockObjects!=NULL)
        deinit();
    
    numTypes = nTypes;
    numDegrees = nDegrees;

    setOfBlockObjects = new Matrix**[numTypes];
    for (int t=0; t<numTypes; t++)
        setOfBlockObjects[t] = new Matrix*[numDegrees];
    
    for(int t=0; t<numTypes; t++){
        int size, idx;
        int * array = setofblockarrays[t*numDegrees];
        for(idx=0; array[idx]!=-1; idx++);
        for(size=0; size*size<idx; size++);
        wallDepth = (wallDepth<size? size: wallDepth);
        for (int d=0; d<numDegrees; d++){
            int* array2= new int[size*size+1];
            int k;
            for (k=0; k<size*size; k++)
                array2[k] = (setofblockarrays[t*numDegrees+ d][k]== 0? 0: 1);
            array2[k]=-1;
            setOfBlockObjects[t][d] = new Matrix(array2,size,size);
            delete [] array2;
        }
    }
}
//deinit()
void Tetris::deinit(){
    for(int t=0; t<numTypes; t++)
        for(int d=0; d<numDegrees; d++)
            delete[] setOfBlockObjects[t][d];
    for(int t=0; t<numTypes; t++)
        delete[] setOfBlockObjects[t];
    delete[] setOfBlockObjects;

    setOfBlockObjects= NULL;
}

//deallocArrayScreen;
void deallocArrayScreen(int* array){delete[] array;}

//allocarrayscreen;
int* allocArrayScreen(int dy, int dx, int dw){
    int ** array2d= new int*[dy+dw];
    for (int y=0; y<dy+dw; y++)
        array2d[y] = new int[dx+2*dx];
    
    for(int y=0; y<dy+dw; y++){
        for(int x=0; x<dw; x++){
            array2d[y][x] = 1;
            array2d[y][x+dw+dx] = 1;
        }
    }

    for(int y=0; y<dy+dw; y++){
        if (y<dy){
            for(int x=0; x<dx; x++)
                array2d[y][x+dw] = 0;
        }
        else{
            for(int x=0; x<dx; x++)
                array2d[y][x+dw] = 1;
        }
    }

    int* array1d = new int[(dy+dw)*(dx+2*dw)];
    for(int y=0; y<dy+dw; y++)
        for(int x=0; x<dx+2*dw; x++)
            array1d[y*(dx+2*dw) + x] = array2d[y][x];
    
    for(int y=0; y<dy+dw; y++)
        delete[] array2d[y];
    delete[] array2d;

    return array1d;
}
//생성자
Tetris::Tetris(int cy, int cx){
    top=0; 
    rows = cy+wallDepth;
    cols = cx+2*wallDepth;
    left= cols/2 - wallDepth/2;
    type = -1;
    degree = 0;

    int * arrayscreen= allocArrayScreen(cy,cx,wallDepth);
    iScreen = new Matrix(arrayscreen,rows, cols);
    oScreen = new Matrix(iScreen);
    currBlk = NULL;
    state = TetrisState::NewBlock;

    deallocArrayScreen(arrayscreen);
}

//deleteFullLines()


//accept
TetrisState Tetirs::accept(char key){
    
}