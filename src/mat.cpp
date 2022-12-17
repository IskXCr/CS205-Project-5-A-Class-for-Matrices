/**
 * @file mat.cpp
 * @author IskXCr (IskXCr@outlook.com)
 * @brief This file contains the basic implementation of a Matrix class,
 *        with its major reference from @ref "The C++ Programming Language, Fourth Edition" by Bjarne Stroustrup.
 * @version 0.1
 * @date 2022-12-16
 *
 * @copyright Copyright (c) 2022 IskXCr
 *
 */
#include "mat.hpp"

#include <cassert>

using namespace utils;

// ------------------------------
// Other constructors, from submatrix or reference to Matrix.
// template <typename T, size_t N>
// Enable_if<(N > 1), Matrix_ref<T, N - 1>> Matrix<T, N>::row(size_t n)
// {
//     assert(n < rows());
//     Matrix_slice<N - 1> row;
//     // Matrix_impl::slice_dim<0>(n, desc, row);
// }

// template <typename T, size_t N>
// Enable_if<(N == 1), T> &Matrix<T, N>::row(size_t i) { return &elems[i]; }