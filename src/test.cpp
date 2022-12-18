#include "mat.hpp"

using namespace std;
using namespace utils;

void test_template_constructors();
void test_arithmetic_operations();

std::vector<void (*)()> funcs{
    test_template_constructors, test_arithmetic_operations};

int main()
{
    for (auto i : funcs)
        (*i)();
};

void test_template_constructors()
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

    Matrix<int, 3> mat2 = {
        {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
        {{10, 11, 12}, {13, 14, 17}, {16, 17, 18}},
        {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}};

    for (int i = 0; i < mat2.size(); ++i)
    {
        std::cout << "OK, element " << i << " is " << *(mat2.data() + i) << endl;
    }
    assert((mat2[1][2][1] == 17));
    mat2[1][2][1] = 2333;
    assert((mat2[1](2, 1) == 2333));

    // std::cout << "OK, element 1 is " << mat2[0] << endl;
    assert(mat2.rows() == 3);
    assert(mat2.columns() == 3);

    Matrix<int, 0> mat3{13};

    assert(mat3() == 13);

    Matrix<int, 1> mat4{{1, 2, 3}};
    assert(mat4.row(1) == 2);

    Matrix<int, 2> mat5(2, 2);
    cout << "The row is " << mat5.rows() << endl;
    cout << "The cols is " << mat5.columns() << endl;
    mat5[1][1] = 5;
    assert(mat5[1][1] == 5);

    Matrix<int, 3> mat6(3, 7, 3);
    mat6[2][6][2] = 2333;
    assert(mat6[2][6][2] == 2333);

    Matrix<int, 1> mat7(2);
    mat7[1] = 4;
    assert(mat7[1] == 4);

    Matrix<int, 0> mat8;
    mat8 = 6444;
    cout << mat8() << endl;

    // iterator test
    cout << "Test normal iterator\n";
    for (auto &i : mat2)
        cout << "Iterator test: " << i << "\n";
    cout << "========>OK.\n";

    cout << "Test reference iterator\n";
    for (auto &i : mat2[2])
        cout << "Iterator test: " << i << "\n";
    cout << "========>OK.\n";

    cout << "Test construction from Matrix_ref\n";
    Matrix<int, 2> mat9(mat2[1]);
    for (auto &i : mat9)
        cout << "Iterator test: " << i++ << "\n";
    cout << mat9.rows() << ", " << mat9.columns() << endl;
    cout << "========>OK.\n";

    cout << "Test iterator change from Matrix\n";
    for (auto &i : mat9)
        cout << "Iterator test: " << i << "\n";
    cout << mat9.rows() << ", " << mat9.columns() << endl;
    cout << "========>OK.\n";

    cout << "Test per element apply from Matrix\n";
    mat9.apply([&](int &a)
               { a += 10; });
    for (auto &i : mat9)
        cout
            << "Iterator test: " << i << "\n";
    cout << mat9.rows() << ", " << mat9.columns() << endl;
    cout << "========>OK.\n";

    cout << "Test 2 per element apply from Matrix\n";
    mat9 += 1;
    for (auto &i : mat9)
        cout
            << "Iterator test: " << i << "\n";
    cout << mat9.rows() << ", " << mat9.columns() << endl;
    cout << "========>OK.\n";
    // assert(mat9 == mat9);
    // assert(mat9 == mat2[1]);
}

void test_arithmetic_operations()
{
    // Matrix<double, 0> mat1{233}, mat2{644};
    // cout << mat1() << endl;
    // mat1 += mat2;
    // cout << mat1() << endl;
}