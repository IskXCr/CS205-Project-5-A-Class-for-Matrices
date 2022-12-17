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
    cout << "The cols is " << mat.columns() << endl;

    Matrix<double, 3> mat2 = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 17}, {16, 17, 18}},
        {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}};
    
    for (int i = 0; i < mat2.size(); ++i)
    {
        std::cout << "OK, element " << i << " is " << *(mat2.data() + i) << endl;
    }

    // std::cout << "OK, element 1 is " << mat2[0] << endl;
    cout << "The row is " << mat2.rows() << endl;
    cout << "The cols is " << mat2.columns() << endl;

    Matrix<double, 0> mat3{1};

    Matrix<double, 1> mat4{{1, 2, 3}};
};