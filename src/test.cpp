#include "mat.hpp"

using namespace std;
using namespace utils;

int main()
{
    Matrix<double, 2> mat{
        {9, 3, 3, 3},
        {6, 7, 8, 9},
        {2, 1, 5, 7}};
    for (int i = 0; i < mat.size(); ++i)
    {
        std::cout << "OK, element " << i << " is " << *(mat.data() + i) << endl;
    }

    cout << "The row is " << mat.rows() << endl;
    cout << "The row is " << mat.rows() << endl;
    cout << "The cols is " << mat.columns() << endl;

    Matrix<double, 3> mat2 = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 17}, {16, 17, 18}},
        {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}};

    for (int i = 0; i < mat2.size(); ++i)
    {
        std::cout << "OK, element " << i << " is " << *(mat2.data() + i) << endl;
    }
    cout << "The (1, 2, 1) element of mat2 with dimension 3 is " << mat2[1][2][1] << endl;
    mat2[1][2][1] = 2333;
    cout << "The (1, 2, 1) element of mat2 with dimension 3 is " << mat2[1](2, 1) << endl;

    // std::cout << "OK, element 1 is " << mat2[0] << endl;
    cout << "The row is " << mat2.rows() << endl;
    cout << "The cols is " << mat2.columns() << endl;

    Matrix<double, 0> mat3{13};

    cout << "The 1st element of mat3 is " << mat3.row() << endl;

    Matrix<int, 1> mat4{{1, 2, 3}};
    cout << "The 2nd element of mat4 with dimension 1 is " << mat4.row(1) << endl;

    Matrix<int, 2> mat5(2, 2);
    cout << "The row is " << mat5.rows() << endl;
    cout << "The cols is " << mat5.columns() << endl;
    mat5[1][1] = 5;
    cout << "Brace-initialized mat5 with element [1][1] = " << mat5[1][1] << endl;

    Matrix<int, 3> mat6(3, 7, 3);
    mat6[2][6][2] = 2333;
    cout << "Brace-initialized mat6 with element [3][7][3] = " << mat6[2][6][2] << endl;

    Matrix<int, 1> mat7(2);
    mat7[1] = 4;
    cout << "Brace-initialized mat7 with element [1] = " << mat7[1] << endl;

    Matrix<int, 0> mat8;
    mat8 = 6444;
    cout << mat8() << endl;
};