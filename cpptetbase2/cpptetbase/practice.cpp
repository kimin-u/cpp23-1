#include "Matrix.h"

int Matrix::get_dx() const{
    return dx;
}
//alloc함수
void Matrix::alloc(int cy, int cx){
    if((cy<=0)||(cx<=0)){
        dy=0; dx=0;
        array=NULL;
        nAlloc++;
        return;
    }
    dx=cx; dy=cy;
    array = new int*[dy];
    for (int y=0; y<dy; y++)
        array[y] = new int [dx];
    for (int y=0;y<dy;y++){
        for ( int x=0; x<dx; x++){
            array[y][x]=0;
        }
    }
    nAlloc++;
}
//dealloc함수
void Matrix::dealloc(){
    if(array!=NULL){
        for (int y=0; y<dy; y++)
            delete [] array[y];
        delete [] array;
    }
    nFree++;
}
//nalloc 개수 반환 함수
//nfree 개수 반환 함수
//디폴트 생성자
//소멸자
//생성자 오버로딩 (크기)
//생성자 오버로딩 (1차원배열 전달)
//복사 생성자(포인터, 참조)
Matrix::Matrix(const Matrix* obj){
    alloc(obj->dy,obj->dx);
    for (int y=0;y<dy;y++){
        for ( int x=0; x<dx; x++){
            array[y][x] = obj->array[y][x];
        }
    }
}
//클립
Matrix * Matrix::clip(int top, int left, int bottom ,int right){
    int cy= bottom- top;
    int cx= right - left;
    Matrix* tmp = new Matrix(cy,cx);
    for (int y=0;y<dy;y++){
        for ( int x=0; x<dx; x++){
            if ((top+y>=0)||(left+x>=0)||(top+y<dy)||(left+x<dx)){
                tmp->array[y][x] = array[top+y][left+x];
            }
            else{
                cerr<<"error";
                delete tmp;
                return NULL;
            }
        }
    }
    return tmp;
}
//paste
//add
//sum
//mulc
//anygreaterthan
//print
//+,-연산자 오버로딩
const Matrix Matrix::operator+(const Matrix& m2) const{

}

const Matrix operator-(const Matrix&m1, const Matrix& m2) {

}
// <<연산자 오버로딩
//대입연산자 오버로딩;
Matrix& Matrix::operator=(const Matrix& obj){
    if (this ==& obj) {return *this;}
    if ((dy!= obj.dy)||(dx!=obj.dx)){
        if (array!=NULL){
            dealloc();
        }
        alloc(obj.dy,obj.dx);
    }
    for (int y=0;y<dy;y++){
        for ( int x=0; x<dx; x++){
            array[y][x] = obj.array[y][x];
        }
    }   
    return *this;
}