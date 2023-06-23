#pragma once
#include <iostream>
#include <cstdlib>

using namespace std;

class Matrix {
private:
  static int nAlloc;         //클래스변수  : 클래스 대표 객체에 저장됨 > 객체 생성수
  static int nFree;          //클래스변수 > 객체 소멸 수 
  int dy;                    //멤버변수  > 매트릭스의 크기
  int dx;
  int **array;               //2차원 배열 
  void alloc(int cy, int cx);    //2차원 배열 초기화 하는 함수 
  void dealloc();
public:
  static int get_nAlloc();
  static int get_nFree();
  int get_dy();
  int get_dx();
  int** get_array();
  Matrix();
  Matrix(int cy, int cx);
  Matrix(const Matrix *obj);
  Matrix(const Matrix &obj);
  Matrix(int *arr, int col, int row);
  ~Matrix();
///////////////////////////////////////////////////////////////////////////////
  Matrix *clip(int top, int left, int bottom, int right);
  void paste(const Matrix *obj, int top, int left);
  Matrix *add(const Matrix *obj);
////////////////////////////////////////////////////////////////////////////////
  const Matrix operator+(const Matrix &m2) const;
  friend const Matrix operator+(const Matrix &m1, const Matrix &m2);
  int sum();
  void mulc(int coef);
  Matrix *int2bool();
  bool anyGreaterThan(int val);
  void print();
  friend ostream& operator<<(ostream& out, const Matrix& obj);
  Matrix& operator=(const Matrix& obj);
  const Matrix operator-(const Matrix& m2) const;
};
