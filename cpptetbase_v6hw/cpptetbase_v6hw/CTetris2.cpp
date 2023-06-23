#include "CTetris.h"

using namespace std;

///**************************************************************
/// static member variables and functions
///**************************************************************

Matrix *** CTetris::setOfColorBlockObjects = NULL;

//init
void CTetris::init(int** setofcolorblockarrays, int nTypes, int nDegrees){
    if (setOfColorBlockObjects != NULL)
        deinit();   

    Tetris::init(setofcolorblockarrays, nTypes, nDegrees);

    setOfColorBlockObjects = new Matrix**[numTypes];

    for(int t=0; t<numTypes; t++)
        setOfColorBlockObjects[t] = new Matrix*[numDegrees];
    
    for(int t=0; t<numTypes; t++){
        int size = setOfBlockObjects[t][0]->get_dy();
        for(int d=0; d<numDegrees; d++){
            setOfColorBlockObjects[t][d] = new Matrix(setofcolorblockarrays[t*numDegrees+d], size, size);
        }
    }
}

//deinit
void CTetris::deinit(){
    Tetris::deinit();

    for(int t=0; t<numTypes; t++)
        for(int d=0; d<numDegrees; d++)
            delete[] setOfColorBlockObjects[t][d];
    for(int t=0; t<numTypes; t++)
        delete[] setOfColorBlockObjects[t];
    delete[] setOfColorBlockObjects;

    setOfColorBlockObjects= NULL;
}
//생성자
CTetris::CTetris(int cy, int cx) : Tetris(cy,cx){
    iCScreen = new Matrix(iScreen);
    oCScreen = new Matrix(oScreen);
    currCBlk = NULL;
}
//소멸자
CTetris::~CTetris(){
    delete iCScreen;
    delete oCScreen;
}

//accept
TetrisState CTetris::accept(char key){
    TetrisState _state = Tetris::accept(key);

    currCBlk = setOfColorBlockObjects[type][degree];
    Matrix * tmpblk = iCScreen->clip(top,left,top+currCBlk->get_dy(), left + currCBlk->get_dx());
    Matrix * tmpblk2 = tmpblk->add(currCBlk);
    delete tmpblk;

    oCScreen->paste(iCScreen,0,0);
    oCScreen->paste(tmpblk2, top, left);
    delete tmpblk2;

    if (_state == TetrisState::NewBlock){
        oCScreen = deleteFullLines(oCScreen, currCBlk, top ,wallDepth);
        iCScreen->paste(oCScreen,0,0);
    }
    return _state;
}