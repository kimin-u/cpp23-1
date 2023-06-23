#include "Matrix.h"

int Matrix::nAlloc = 0;
int Matrix::nFree = 0;

int Matrix::get_nAlloc() { return nAlloc; }

int Matrix::get_nFree() { return nFree; }

int Matrix::get_dy() { return dy; }

int Matrix::get_dx() { return dx; }

int **Matrix::get_array() { return array; }

void Matrix::alloc(int cy, int cx) {
  if ((cy < 0) || (cx < 0)) return;
  dy = cy;
  dx = cx;
  array = new int*[dy];              //dy만큼의 포인터 변수를 만들어서 배열의 형태로 저장해라.
  for (int y = 0; y < dy; y++)
    array[y] = new int[dx];          //y번째 포인터에 1차원 배열을 넣는다. 
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = 0;
  
  nAlloc++;
}

void Matrix::dealloc() { 
  if (array != NULL) {
    for (int y = 0; y < dy; y++)
      delete[] array[y];
    delete[] array;
    array = NULL;
  }

  nFree++;
}



Matrix::Matrix() { alloc(0, 0); }

Matrix::~Matrix() { 
  for (int y = 0; y < dy; y++)
    delete array[y];
  delete array;

  nFree++;
}

Matrix::Matrix(int cy, int cx) {
  alloc(cy, cx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = 0;
}

Matrix::Matrix(const Matrix *obj) {
  alloc(obj->dy, obj->dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj->array[y][x];
}

Matrix::Matrix(const Matrix &obj) {
  alloc(obj.dy, obj.dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj.array[y][x];
}

Matrix::Matrix(int *arr, int col, int row) {
  alloc(col, row);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = arr[y * dx + x];
}

Matrix *Matrix::clip(int top, int left, int bottom, int right) {
  int cy = bottom - top;                                         // y좌표 차이 i.e. 세로길이 
  int cx = right - left;                                         // x좌표 차이 i.e. 가로길이 
  Matrix *temp = new Matrix(cy, cx);                             // temp = 0으로 가득찬 세로길이 x 가로길이 행렬.
  for (int y = 0; y < cy; y++) {
    for (int x = 0; x < cx; x++) {
      if ((top + y >= 0) && (left + x >= 0) && (top + y < dy) && (left + x < dx))                  //boundary check
	      temp->array[y][x] = array[top + y][left + x];                  // temp의 행렬요소 값 초기화, self객체의 값으로 
      else {
	      cerr << "invalid matrix range";   
	      return NULL;
      }
    }
  }
  return temp;
}

void Matrix::paste(const Matrix *obj, int top, int left) {
  for (int y = 0; y < obj->dy; y++)
    for (int x = 0; x < obj->dx; x++) {
      if ((top + y >= 0) && (left + x >= 0) && (top + y < dy) && (left + x < dx))
	      array[y + top][x + left] = obj->array[y][x];
      else {
	      cerr << "invalid matrix range";
	      return NULL;
      }
    }
}

Matrix *Matrix::add(const Matrix *obj) {
  if ((dx != obj->dx) || (dy != obj->dy)) return NULL;
  Matrix *temp = new Matrix(dy, dx);
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      temp->array[y][x] = array[y][x] + obj->array[y][x];
  return temp;
}

int Matrix::sum() {
  int total = 0;
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      total += array[y][x];
  return total;
}

void Matrix::mulc(int coef) {
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = coef * array[y][x];
}

Matrix *Matrix::int2bool() {
  Matrix *temp = new Matrix(dy, dx);
  int **t_array = temp->get_array();
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      t_array[y][x] = (array[y][x] != 0 ? 1 : 0);
  
  return temp;
}

bool Matrix::anyGreaterThan(int val) {
  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < dx; x++) {
      if (array[y][x] > val)
	      return true;
    }
  }
  return false;
}

void Matrix::print() {
  cout << "Matrix(" << dy << "," << dx << ")" << endl;
  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < dx; x++)
      cout << array[y][x] << " ";
    cout << endl;
  }
}


ostream& operator<<(ostream& out, const Matrix& obj){
  out << "Matrix(" << obj.dy << "," << obj.dx << ")" << endl;
  for(int y = 0; y < obj.dy; y++){
    for(int x = 0; x < obj.dx; x++)
      out << obj.array[y][x] << " ";
    out << endl;
  }
  out << endl;
  return out;
}


/*
const Matrix Matrix::operator+(const Matrix &m2) const{
  if ((dx!=m2.dx)||(dy!=m2.dy)) return Matrix(); //행렬의 크기 비교
  Matrix temp(dy,dx);    //생성자 호출 
  for (int y=0; y<dy; y++){
    for (int x=0; x<dx; x++){
      temp.array[y][x] = array[y][x] + m2.array[y][x];
    }
  }
  return temp;
}
*/

//전역함수의 형태; m1의 멤버변수의 접근 하기 위해서는 헤더파일에 friend선언이 있어야함.
const Matrix operator+(const Matrix &m1, const Matrix &m2){
  if ((m1.dx!=m2.dx)||(m1.dy!=m2.dy)) return Matrix();
  Matrix temp(m1.dy,m1.dx);
  for (int y=0; y<m1.dy; y++){
    for (int x=0; x<m1.dx; x++){
      temp.array[y][x] = m1.array[y][x] + m2.array[y][x];
    }
  }
  return temp;
}


Matrix& Matrix::operator=(const Matrix& obj)
{
  if (this == &obj) return *this;
  if ((dx != obj.dx) || (dy != obj.dy)){
    if (array!= NULL) dealloc();
    alloc(obj.dy, obj.dx);
  }
  for (int y = 0; y < dy; y++)
    for (int x = 0; x < dx; x++)
      array[y][x] = obj.array[y][x];
  return *this;
}
